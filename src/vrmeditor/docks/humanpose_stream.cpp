#include "humanpose_stream.h"
#include <algorithm>
#include <array>
#include <imnodes.h>
#include <vector>

void
GraphNode::Draw()
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

  CreateNode(
    "HumanPose",
    "SinkNode",
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } },
    {});

  CreateNode(
    "Bvh",
    "SrcNode",
    {},
    std::vector<PinNameWithType>{ { "HumanPose", PinDataTypes::HumanPose } });

  CreateNode(
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

std::shared_ptr<GraphNode>
HumanPoseStream::CreateNode(std::string_view name,
                            std::string_view prefix,
                            std::span<const PinNameWithType> inputs,
                            std::span<const PinNameWithType> outputs)
{
  auto ptr = std::make_shared<GraphNode>(m_nextNodeId++, name);
  ptr->Prefix = prefix;
  for (auto [pinName, pinDataType] : inputs) {
    ptr->Inputs.push_back({ pinName, { m_nextPinId++, pinDataType } });
  }
  for (auto [pinName, pinDataType] : outputs) {
    ptr->Outputs.push_back({ pinName, { m_nextPinId++, pinDataType } });
  }
  m_nodes.push_back(ptr);
  return ptr;
}

std::tuple<std::shared_ptr<GraphNode>, PinDataTypes>
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

std::tuple<std::shared_ptr<GraphNode>, PinDataTypes>
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
