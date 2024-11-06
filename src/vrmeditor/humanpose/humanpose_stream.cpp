#include "humanpose_stream.h"
#include "app.h"
#include "bvhnode.h"
#include "posenode.h"
#include <gltfjson/gltf_typing_vrm1.h>
#include <imnodes.h>
#include <plog/Log.h>
#include <vrm/fileutil.h>
#include <vrm/gltfroot.h>
#include <vrm/humanoid/humanbones.h>
#include <vrm/importer.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

namespace humanpose {
struct HumanPoseSink : public GraphNodeBase
{
  // constructor
  using GraphNodeBase::GraphNodeBase;

  void PullData(InputNodes inputs) override
  {
    // pull upstream data
    assert(inputs.size() == 1);
    Pull(inputs);
  }
};

void
GraphNodeBase::Draw()
{
  // std::cout << "node: " << id << std::endl;
  ImNodes::BeginNode(Id);
  ImNodes::BeginNodeTitleBar();
  if (Prefix.size()) {
    ImGui::TextUnformatted(Prefix.c_str());
  }
  ImGui::TextUnformatted(Name.c_str());
  ImNodes::EndNodeTitleBar();

  DrawContent();

  for (auto& input : Inputs) {
    // std::cout << "  input: " << pin_id << std::endl;
    ImNodes::BeginInputAttribute(input.Pin.Id);
    {
      auto label = input.Name.c_str();
      // const float label_width = ImGui::CalcTextSize(label).x;
      ImGui::TextUnformatted(label);
      // ImGui::Indent(NodeWidth - label_width);
      // ImGui::TextUnformatted(label);
    }
    ImNodes::EndInputAttribute();
  }

  for (auto& output : Outputs) {
    // std::cout << "  output: " << pin_id << std::endl;
    ImNodes::BeginOutputAttribute(output.Pin.Id);
    {
      auto label = output.Name.c_str();
      const float label_width = ImGui::CalcTextSize(label).x;
      ImGui::Indent(NodeWidth - label_width);
      ImGui::TextUnformatted(label);
    }
    ImNodes::EndOutputAttribute();
  }

  ImNodes::EndNode();
}

void
Link::Draw()
{
  ImNodes::Link(Id, Start, End);
}

HumanPoseStream::HumanPoseStream()
{
  ImNodes::CreateContext();
  ImNodes::PushAttributeFlag(
    ImNodesAttributeFlags_EnableLinkDetachWithDragClick);

  // sink node
  auto sink = CreateNode<HumanPoseSink>(
    "🎬RuntimeView",
    "Pose",
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } },
    {});

  sink->Pull = [this](GraphNodeBase::InputNodes inputs) {
    if (std::holds_alternative<libvrm::HumanPose>(inputs[0])) {
      auto value = std::get<libvrm::HumanPose>(inputs[0]);
      for (auto it = HumanPoseChanged.begin(); it != HumanPoseChanged.end();) {
        if ((*it)(value)) {
          ++it;
        } else {
          it = HumanPoseChanged.erase(it);
        }
      }
    }
  };

  // source nodes
  // CreateNode<UdpNode>(
  //   "Udp",
  //   "SrcNode",
  //   {},
  //   std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });

  CreateNode<PoseNode>(
    "InitialPose",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });
}

HumanPoseStream::~HumanPoseStream()
{
  ImNodes::PopAttributeFlag();
  ImNodes::DestroyContext();
}

void
HumanPoseStream::LoadIni(std::string_view ini)
{
  // Load the internal imnodes state
  ImNodes::LoadCurrentEditorStateFromIniString(ini.data(), ini.size());
}

std::string
HumanPoseStream::Save()
{
  // Save the internal imnodes state
  size_t size;
  auto p = ImNodes::SaveCurrentEditorStateToIniString(&size);
  return { p, p + size };
}

void
HumanPoseStream::ShowGui()
{
  // draw nodes
  ImNodes::BeginNodeEditor();
  for (auto& node : m_nodes) {
    node->Draw();
  }
  for (auto& link : Links) {
    link->Draw();
  }
  ImNodes::EndNodeEditor();

  // update link
  int start, end;
  if (ImNodes::IsLinkCreated(&start, &end)) {
    TryCreateLink(start, end);
  }
  int link_id;
  if (ImNodes::IsLinkDestroyed(&link_id)) {
    TryRemoveLink(link_id);
  }
}

std::tuple<std::shared_ptr<GraphNodeBase>, size_t>
HumanPoseStream::FindNodeFromOutput(int start) const
{
  for (auto& node : m_nodes) {
    for (size_t i = 0; i < node->Outputs.size(); ++i) {
      if (node->Outputs[i].Pin.Id == start) {
        return { node, i };
      }
    }
  }
  return {};
}

std::tuple<std::shared_ptr<GraphNodeBase>, size_t>
HumanPoseStream::FindNodeFromInput(int end) const
{
  for (auto& node : m_nodes) {
    for (size_t i = 0; i < node->Inputs.size(); ++i) {
      if (node->Inputs[i].Pin.Id == end) {
        return { node, i };
      }
    }
  }
  return {};
}

void
HumanPoseStream::TryCreateLink(int start, int end)
{
  auto [src, srcType] = FindNodeFromOutput(start);
  auto [sink, sinkType] = FindNodeFromInput(end);
  if (src && sink && srcType == sinkType) {

    for (auto it = Links.begin(); it != Links.end();) {
      if ((*it)->Start == start || (*it)->End == end) {
        // remove exists
        it = Links.erase(it);
      } else {
        // skip
        ++it;
      }
    }

    Links.push_back(std::make_shared<Link>(m_nextLinkId++, start, end));
  }
}

void
HumanPoseStream::TryRemoveLink(int link_id)
{
  auto iter = std::find_if(
    Links.begin(), Links.end(), [link_id](const auto& link) -> bool {
      return link->Id == link_id;
    });
  assert(iter != Links.end());
  Links.erase(iter);
}

bool
HumanPoseStream::LoadMotion(const std::filesystem::path& path)
{
  auto bytes = libvrm::ReadAllBytes(path);
  if (bytes.empty()) {
    PLOG_ERROR << "fail to read: " + path.string();
    return false;
  }

  // load bvh
  auto bvh = std::make_shared<libvrm::bvh::Bvh>();
  if (auto parsed = bvh->Parse({ (const char*)bytes.data(), bytes.size() })) {
  } else {
    PLOG_ERROR << "LoadMotion: " << path.string();
    // PLOG_ERROR << "LoadMotion: " << parsed.error();
    return false;
  }
  auto scaling = bvh->GuessScaling();
  PLOG_INFO << "LoadMotion: " << scaling << ", " << path.string();

  auto node = CreateNode<BvhNode>(
    "Bvh",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });

  node->SetBvh(bvh, FindHumanBoneMap(*bvh));

  return true;
}

bool
HumanPoseStream::LoadVrmPose(const std::string& json)
{
  if (auto loaded = libvrm::LoadGltf(json)) {
    // auto scene = SetGltf(*gltf);
    PLOG_INFO << "paste gltf string";

    auto node = CreateNode<PoseNode>(
      "VRMC_vrm_pose",
      "SrcNode",
      {},
      std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });
    auto runtime = libvrm::RuntimeScene::Load(loaded);

    if (auto VRMC_vrm_animation =
          runtime->m_base->m_gltf
            ->GetExtension<gltfjson::vrm1::VRMC_vrm_animation>()) {
      if (auto VRMC_vrm_pose =
            VRMC_vrm_animation->GetExtension<gltfjson::vrm1::VRMC_vrm_pose>()) {

        if (auto humanoid = VRMC_vrm_pose->Humanoid()) {
          for (auto kv : *humanoid) {
            if (kv.first == u8"translation") {
              if (auto node = runtime->GetBoneNode(libvrm::HumanBones::hips)) {
                auto v = libvrm::ToVec3(kv.second);
                node->Transform.Translation = v;
              }
            }
            if (kv.first == u8"rotations") {
              if (auto rotations =
                    std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(
                      kv.second)) {
                for (auto [key, value] : rotations->Value) {
                  if (auto bone = libvrm::HumanBoneFromName(
                        gltfjson::from_u8(key), libvrm::VrmVersion::_1_0)) {
                    if (auto node = runtime->GetBoneNode(*bone)) {
                      DirectX::XMFLOAT4 q = libvrm::ToVec4(value);
                      node->Transform.Rotation = q;
                    }
                  }
                }
              }
            }
          }
          node->Payload.SetPose(runtime->UpdateHumanPose());
        }
      }
    }

    return true;
  } else {
    // PLOG_ERROR << loaded.error();
    return false;
  }
}

void
HumanPoseStream::Update(libvrm::Time time, std::shared_ptr<GraphNodeBase> node)
{
  // 全部 Update
  for (auto& n : m_nodes) {
    n->TimeUpdate(time);
  }

  // sink から遡って再帰的に update する
  if (!node) {
    node = m_nodes.front();
  }

  std::vector<InputData> inputs;
  for (auto& input : node->Inputs) {
    if (auto link = FindLinkFromEnd(input.Pin.Id)) {
      auto [upstream, index] = FindNodeFromOutput(link->Start);
      inputs.push_back(upstream->Outputs[index].Value);
      if (upstream) {
        Update(time, upstream);
      }
    } else {
      inputs.push_back({});
    }
  }

  // process
  node->PullData(inputs);
}
}
