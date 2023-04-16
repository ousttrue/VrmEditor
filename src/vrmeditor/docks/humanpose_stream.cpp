#include "humanpose_stream.h"
#include "cuber.h"
#include "udp_receiver.h"
#include <algorithm>
#include <array>
#include <imnodes.h>
#include <vector>
#include <vrm/scene.h>

struct BvhNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_motion;
  std::shared_ptr<Cuber> m_cuber;

  // constructor
  using GraphNodeBase::GraphNodeBase;
};

struct UdpNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_motion;
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<UdpReceiver> m_udp;

  // constructor
  using GraphNodeBase::GraphNodeBase;
};

void
GraphNodeBase::Draw()
{
  // std::cout << "node: " << id << std::endl;
  const float node_width = 200.f;
  ImNodes::BeginNode(Id);
  ImNodes::BeginNodeTitleBar();
  if (Prefix.size()) {
    ImGui::TextUnformatted(Prefix.c_str());
  }
  ImGui::TextUnformatted(Name.c_str());
  ImNodes::EndNodeTitleBar();

  for (auto& input : Inputs) {
    // std::cout << "  input: " << pin_id << std::endl;
    ImNodes::BeginInputAttribute(input.Pin.Id);
    {
      auto label = input.Name.c_str();
      // const float label_width = ImGui::CalcTextSize(label).x;
      ImGui::TextUnformatted(label);
      // ImGui::Indent(node_width - label_width);
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
      ImGui::Indent(node_width - label_width);
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

struct HumanPoseStreamImpl
{

  void Draw() {}
};

HumanPoseStream::HumanPoseStream()
{
  ImNodes::CreateContext();

  // sink node
  CreateNode<GraphNodeBase>(
    "HumanPose",
    "SinkNode",
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } },
    {});

  // source nodes
  CreateNode<UdpNode>(
    "Udp",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });

  CreateNode<GraphNodeBase>(
    "TPose",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });
}

HumanPoseStream::~HumanPoseStream() {}

void
HumanPoseStream::LoadIni(std::string_view ini)
{
  // Load the internal imnodes state
  ImNodes::LoadCurrentEditorStateFromIniString(ini.data(), ini.size());

  // // Load our editor state into memory
  //
  // std::fstream fin("save_load.bytes",
  //                  std::ios_base::in | std::ios_base::binary);
  //
  // if (!fin.is_open()) {
  //   return;
  // }
  //
  // // copy nodes into memory
  // size_t num_nodes;
  // fin.read(reinterpret_cast<char*>(&num_nodes),
  //          static_cast<std::streamsize>(sizeof(size_t)));
  // nodes_.resize(num_nodes);
  // fin.read(reinterpret_cast<char*>(nodes_.data()),
  //          static_cast<std::streamsize>(sizeof(Node) * num_nodes));
  //
  // // copy links into memory
  // size_t num_links;
  // fin.read(reinterpret_cast<char*>(&num_links),
  //          static_cast<std::streamsize>(sizeof(size_t)));
  // links_.resize(num_links);
  // fin.read(reinterpret_cast<char*>(links_.data()),
  //          static_cast<std::streamsize>(sizeof(Link) * num_links));
  //
  // // copy current_id into memory
  // fin.read(reinterpret_cast<char*>(&current_id_),
  //          static_cast<std::streamsize>(sizeof(int)));
}

std::string
HumanPoseStream::Save()
{
  // Save the internal imnodes state
  size_t size;
  auto p = ImNodes::SaveCurrentEditorStateToIniString(&size);
  return { p, p + size };

  // // Dump our editor state as bytes into a file
  //
  // std::fstream fout("save_load.bytes",
  //                   std::ios_base::out | std::ios_base::binary |
  //                     std::ios_base::trunc);
  //
  // // copy the node vector to file
  // const size_t num_nodes = nodes_.size();
  // fout.write(reinterpret_cast<const char*>(&num_nodes),
  //            static_cast<std::streamsize>(sizeof(size_t)));
  // fout.write(reinterpret_cast<const char*>(nodes_.data()),
  //            static_cast<std::streamsize>(sizeof(Node) * num_nodes));
  //
  // // copy the link vector to file
  // const size_t num_links = links_.size();
  // fout.write(reinterpret_cast<const char*>(&num_links),
  //            static_cast<std::streamsize>(sizeof(size_t)));
  // fout.write(reinterpret_cast<const char*>(links_.data()),
  //            static_cast<std::streamsize>(sizeof(Link) * num_links));
  //
  // // copy the current_id to file
  // fout.write(reinterpret_cast<const char*>(&current_id_),
  //            static_cast<std::streamsize>(sizeof(int)));
}

void
HumanPoseStream::CreateDock(const AddDockFunc& addDock)
{
  addDock(Dock("input-stream", [this]() {
    //
    // draw nodes
    ImNodes::BeginNodeEditor();
    for (auto& node : m_nodes) {
      node->Draw();
    }
    for (auto& edge : Links) {
      edge.Draw();
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
  }));
}

std::tuple<std::shared_ptr<GraphNodeBase>, PinDataTypes>
HumanPoseStream::FindNodeFromOutput(int start) const
{
  for (auto& node : m_nodes) {
    for (auto& output : node->Outputs) {
      if (output.Pin.Id == start) {
        return { node, output.Pin.DataType };
      }
    }
  }
  return {};
}

std::tuple<std::shared_ptr<GraphNodeBase>, PinDataTypes>
HumanPoseStream::FindNodeFromInput(int start) const
{
  for (auto& node : m_nodes) {
    for (auto& input : node->Inputs) {
      if (input.Pin.Id == start) {
        return { node, input.Pin.DataType };
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
      if (it->Start == start || it->End == end) {
        // remove exists
        it = Links.erase(it);
      } else {
        // skip
        ++it;
      }
    }

    Links.push_back({ m_nextLinkId++, start, end });
  }
}

void
HumanPoseStream::TryRemoveLink(int link_id)
{
  auto iter = std::find_if(
    Links.begin(), Links.end(), [link_id](const auto& link) -> bool {
      return link.Id == link_id;
    });
  assert(iter != Links.end());
  Links.erase(iter);
}

bool
HumanPoseStream::LoadMotion(const std::filesystem::path& path)
{
  auto node = CreateNode<BvhNode>(
    "Bvh",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });

  // auto bytes = libvrm::fileutil::ReadAllBytes<uint8_t>(path);
  // if (bytes.empty()) {
  //   Log(LogLevel::Error) << "fail to read: " + path.string();
  //   return false;
  // }
  //
  // // load bvh
  // auto bvh = std::make_shared<libvrm::bvh::Bvh>();
  // if (auto parsed = bvh->Parse({ (const char*)bytes.data(), bytes.size() }))
  // { } else {
  //   Log(LogLevel::Error) << "LoadMotion: " << path;
  //   Log(LogLevel::Error) << "LoadMotion: " << parsed.error();
  //   return false;
  // }
  // auto scaling = bvh->GuessScaling();
  // Log(LogLevel::Info) << "LoadMotion: " << scaling << ", " << path;
  //
  // libvrm::bvh::InitializeSceneFromBvh(m_motion, bvh);
  // m_motion->m_roots[0]->UpdateShapeInstanceRecursive(
  //   DirectX::XMMatrixIdentity(), m_cuber->Instances);
  //
  // // bind time to motion
  // auto track = m_timeline->AddTrack("bvh", bvh->Duration());
  // track->Callbacks.push_back([bvh, scene = m_motion](auto time, bool repeat)
  // {
  //   if (scene->m_roots.size()) {
  //     libvrm::bvh::UpdateSceneFromBvhFrame(scene, bvh, time);
  //     scene->m_roots[0]->CalcWorldMatrix(true);
  //     scene->RaiseSceneUpdated();
  //   }
  // });
  //
  // if (auto map = FindHumanBoneMap(*bvh)) {
  //   // assign human bone
  //   for (auto& node : m_motion->m_nodes) {
  //     auto found = map->NameBoneMap.find(node->Name);
  //     if (found != map->NameBoneMap.end()) {
  //       node->Humanoid = libvrm::gltf::NodeHumanoidInfo{
  //         .HumanBone = found->second,
  //       };
  //     }
  //   }
  //
  // } else {
  //   Log(LogLevel::Wran) << "humanoid map not found";
  // }
  return true;
}

// {
//   HumanoidDock::Create(addDock, "motion-body", "motion-finger", m_motion);
//   auto selection =
//     SceneDock::CreateTree(addDock, "motion-hierarchy", m_motion, indent);
//
//   auto callback = [scene = m_motion,
//                    cuber = m_cuber](std::span<const uint8_t> data) {
//     // udp update m_motion scene
//     libvrm::srht::UpdateScene(scene, cuber->Instances, data);
//
//     if (scene->m_roots.size()) {
//       scene->m_roots[0]->CalcWorldMatrix(true);
//       scene->RaiseSceneUpdated();
//     }
//   };
//
//   MotionDock::Create(
//     addDock,
//     "motion",
//     m_cuber,
//     selection,
//     [this, callback]() {
//       ClearMotion();
//       m_udp->Start(54345, callback);
//     },
//     [udp = m_udp]() { udp->Stop(54345); });
//
// }

// m_udp->Update();

void
HumanPoseStream::Update(libvrm::Time time)
{
  auto a = 0;
}
