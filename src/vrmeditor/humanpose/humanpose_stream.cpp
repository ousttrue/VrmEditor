#include "humanpose_stream.h"
#include "app.h"
#include "bvhnode.h"
#include "udpnode.h"
#include <imnodes.h>
#include <vrm/fileutil.h>

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

struct InitialPose : public GraphNodeBase
{
  // constructor
  using GraphNodeBase::GraphNodeBase;

  void TimeUpdate(libvrm::Time time) override
  {
    Outputs[0].Value = libvrm::vrm::HumanPose::Initial();
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

  // sink node
  auto sink = CreateNode<HumanPoseSink>(
    "HumanPose",
    "SinkNode",
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } },
    {});

  sink->Pull = [this](GraphNodeBase::InputNodes inputs) {
    if (std::holds_alternative<libvrm::vrm::HumanPose>(inputs[0])) {
      auto value = std::get<libvrm::vrm::HumanPose>(inputs[0]);
      for (auto& callback : HumanPoseChanged) {
        callback(value);
      }
    }
  };

  // source nodes
  CreateNode<UdpNode>(
    "Udp",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });

  CreateNode<InitialPose>(
    "InitialPose",
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
HumanPoseStream::CreateDock(const AddDockFunc& addDock)
{
  addDock(Dock("input-stream", [this]() {
    //
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
  }));
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
  auto bytes = libvrm::fileutil::ReadAllBytes<uint8_t>(path);
  if (bytes.empty()) {
    App::Instance().Log(LogLevel::Error) << "fail to read: " + path.string();
    return false;
  }

  // load bvh
  auto bvh = std::make_shared<libvrm::bvh::Bvh>();
  if (auto parsed = bvh->Parse({ (const char*)bytes.data(), bytes.size() })) {
  } else {
    App::Instance().Log(LogLevel::Error) << "LoadMotion: " << path;
    App::Instance().Log(LogLevel::Error) << "LoadMotion: " << parsed.error();
    return false;
  }
  auto scaling = bvh->GuessScaling();
  App::Instance().Log(LogLevel::Info)
    << "LoadMotion: " << scaling << ", " << path;

  auto node = CreateNode<BvhNode>(
    "Bvh",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });

  node->SetBvh(bvh, FindHumanBoneMap(*bvh));

  return true;
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
