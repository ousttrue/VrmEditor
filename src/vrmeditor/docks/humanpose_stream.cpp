#include "humanpose_stream.h"
#include <array>
#include <imnodes.h>
#include <vector>

enum PinDataTypes
{
  None,
  HumanPose,
};

struct GraphPin
{
  int Id;
  PinDataTypes DataType;

  GraphPin(int id, PinDataTypes dataType)
    : Id(id)
    , DataType(dataType)
  {
  }
};

struct Input
{
  std::string Name;
  GraphPin Pin;

  Input(std::string_view name, GraphPin pin)
    : Name(name)
    , Pin(pin)
  {
  }
};

struct Output
{
  std::string Name;
  GraphPin Pin;

  Output(std::string_view name, GraphPin pin)
    : Name(name)
    , Pin(pin)
  {
  }
};

struct GraphNode
{
  int Id;
  std::string Prefix;
  std::string Name;
  std::vector<Input> Inputs;
  std::vector<Output> Outputs;

  GraphNode(int id, std::string_view name)
    : Id(id)
    , Name(name)
  {
  }

  void Draw()
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
};

struct Edge
{
  int Id;
  int Start;
  int End;

  void Draw() { ImNodes::Link(Id, Start, End); }
};

struct PinNameWithType
{
  std::string Name;
  PinDataTypes Type;
};

class GraphManager
{
  int m_nextId = 1;
  std::list<std::shared_ptr<GraphNode>> m_nodes;
  std::list<Edge> m_edges;

public:
  GraphManager()
  {
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

  std::shared_ptr<GraphNode> CreateNode(
    std::string_view name,
    std::string_view prefix,
    std::span<const PinNameWithType> inputs,
    std::span<const PinNameWithType> outputs)
  {
    auto ptr = std::make_shared<GraphNode>(m_nextId++, name);
    ptr->Prefix = prefix;
    for (auto [pinName, pinDataType] : inputs) {
      ptr->Inputs.push_back({ pinName, { m_nextId++, pinDataType } });
    }
    for (auto [pinName, pinDataType] : outputs) {
      ptr->Outputs.push_back({ pinName, { m_nextId++, pinDataType } });
    }
    m_nodes.push_back(ptr);
    return ptr;
  }

  std::tuple<std::shared_ptr<GraphNode>, PinDataTypes> FindNodeFromOutput(
    int start) const
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

  std::tuple<std::shared_ptr<GraphNode>, PinDataTypes> FindNodeFromInput(
    int start) const
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

  void TryCreateLink(int start, int end)
  {
    auto [src, srcType] = FindNodeFromOutput(start);
    auto [sink, sinkType] = FindNodeFromInput(end);
    if (src && sink && srcType == sinkType) {

      for (auto it = m_edges.begin(); it != m_edges.end();) {
        if (it->Start == start || it->End == end) {
          // remove exists
          it = m_edges.erase(it);
        } else {
          // skip
          ++it;
        }
      }

      m_edges.push_back({ m_nextId++, start, end });
    }
  }

  void Draw()
  {
    // draw nodes
    ImNodes::BeginNodeEditor();
    for (auto& node : m_nodes) {
      node->Draw();
    }
    for (auto& edge : m_edges) {
      edge.Draw();
    }
    ImNodes::EndNodeEditor();

    // update link
    int start, end;
    if (ImNodes::IsLinkCreated(&start, &end)) {
      TryCreateLink(start, end);
    }
  }
};

struct HumanPoseStreamImpl
{
  GraphManager m_graph;
};

HumanPoseStream::HumanPoseStream()
  : m_impl(new HumanPoseStreamImpl)
{
}

HumanPoseStream::~HumanPoseStream()
{
  delete m_impl;
}

void
HumanPoseStream::CreateDock(const AddDockFunc& addDock)
{
  addDock(Dock("input-stream", [&graph = m_impl->m_graph]() {
    //
    graph.Draw();
  }));
}
