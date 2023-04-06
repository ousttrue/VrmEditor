#include <GL/glew.h>

#include "gl3renderer.h"
#include "gui.h"
#include "motion_dock.h"
#include "orbitview.h"
#include "rendertarget.h"
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <imgui.h>
#include <imnodes.h>
#include <iostream>
#include <vrm/bvh.h>
#include <vrm/bvhsolver.h>
#include <vrm/humanbones.h>
#include <vrm/humanpose.h>

struct Edge
{
  int Start;
  int End;
};
std::vector<Edge> s_edges;

static std::vector<GraphNode> s_inputs = {
  { .Prefix = "Gui", .Name = "emotion", .Outputs = { { "emotion" } } },
  { .Prefix = "Gui", .Name = "lipsync", .Outputs = { { "lipsync" } } },
  { .Prefix = "Gui", .Name = "lookat", .Outputs = { { "lookat" } } },
  { .Prefix = "Clip", .Name = "bvh", .Outputs = { { "upper" }, { "lower" } } },
  { .Prefix = "Clip",
    .Name = "vrma",
    .Outputs = { { "upper" },
                 { "lower" },
                 { "hand" },
                 { "emotion" },
                 { "lipsync" },
                 { "lookat" } } },
  { .Prefix = "Capture",
    .Name = "mocopi",
    .Outputs = { { "upper" }, { "lower" } } },
  { .Prefix = "Capture",
    .Name = "azure_kinect",
    .Outputs = { { "upper" }, { "lower" } } },
  { .Prefix = "OpenXR",
    .Name = "EXT_hand_tracking",
    .Outputs = { { "hand" } } },
  { .Prefix = "OpenXR",
    .Name = "FB_body_tracking",
    .Outputs = { { "upper" }, { "hand" } } },
  { .Prefix = "OpenXR",
    .Name = "FB_face_tracking",
    .Outputs = { { "facial" } } },
  { .Prefix = "OpenXR",
    .Name = "FB_eye_tracking_sotial",
    .Outputs = { { "lookat" } } },
  { .Prefix = "OpenXR",
    .Name = "VR_3points",
    .Outputs = { { "three_points" } } }
};

static std::vector<GraphNode> s_conv = {
  {
    .Name = "faicial_conv",
    .Outputs = { { "emotion" }, { "lipsync" }, { "lookat" } },
    .Inputs = { { "facial" } },
  },
  {
    .Name = "threepoints_conv",
    .Outputs = { { "upper" }, { "lower" } },
    .Inputs = { { "three_points" } },
  },
};

static GraphNode s_output = {
    .Name="humanoid",
    .Inputs = {
      {"upper"},
      {"lower"},
      {"hand"},
      {"emotion"},
      {"lipsync"},
      {"lookat"},
      {"faicial"},
    },
  };

static void
draw(const GraphNode& node, int id)
{
  // std::cout << "node: " << id << std::endl;
  const float node_width = 200.f;
  ImNodes::BeginNode(id);
  ImNodes::BeginNodeTitleBar();
  if (node.Prefix.size()) {
    ImGui::TextUnformatted(node.Prefix.c_str());
  }
  ImGui::TextUnformatted(node.Name.c_str());
  ImNodes::EndNodeTitleBar();

  int k = 0;
  for (; k < node.Inputs.size(); ++k) {
    int pin_id = id + k + 1;
    // std::cout << "  input: " << pin_id << std::endl;
    ImNodes::BeginInputAttribute(pin_id);
    {
      auto label = node.Inputs[k].Name.c_str();
      // const float label_width = ImGui::CalcTextSize(label).x;
      ImGui::TextUnformatted(label);
      // ImGui::Indent(node_width - label_width);
      // ImGui::TextUnformatted(label);
    }
    ImNodes::EndInputAttribute();
  }

  int j = 0;
  for (; j < node.Outputs.size(); ++j) {
    int pin_id = id + j + k + 1;
    // std::cout << "  output: " << pin_id << std::endl;
    ImNodes::BeginOutputAttribute(pin_id);
    {
      auto label = node.Outputs[j].Name.c_str();
      const float label_width = ImGui::CalcTextSize(label).x;
      ImGui::Indent(node_width - label_width);
      ImGui::TextUnformatted(label);
    }
    ImNodes::EndOutputAttribute();
  }

  ImNodes::EndNode();
}

void
MotionDock::Create(const AddDockFunc& addDock,
                   const std::shared_ptr<MotionSource>& motion_source)
{
  auto rt = std::make_shared<RenderTarget>(std::make_shared<OrbitView>());
  rt->color[0] = 0.4f;
  rt->color[1] = 0.2f;
  rt->color[2] = 0.2f;
  rt->color[3] = 1.0f;

  auto cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
  auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  std::vector<grapho::LineVertex> lines;
  cuber::PushGrid(lines);

  rt->render =
    [cuber, liner, lines, motion_source](const ViewProjection& camera) {
      if (motion_source->MotionSolver) {
        cuber->Render(camera.projection,
                      camera.view,
                      motion_source->MotionSolver->instances_.data(),
                      motion_source->MotionSolver->instances_.size());
      }
      liner->Render(camera.projection, camera.view, lines);
    };

  auto gl3r = std::make_shared<Gl3Renderer>();

  addDock(Dock("motion", [rt](bool* p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    if (ImGui::Begin("motion",
                     p_open,
                     ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoScrollWithMouse)) {
      auto pos = ImGui::GetWindowPos();
      pos.y += ImGui::GetFrameHeight();
      auto size = ImGui::GetContentRegionAvail();
      rt->show_fbo(pos.x, pos.y, size.x, size.y);
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }));

  addDock(Dock("input-stream", []() {
    ImNodes::BeginNodeEditor();

    // ImNodes::BeginNode(hardcoded_node_id);
    // ImGui::Dummy(ImVec2(80.0f, 45.0f));
    // ImNodes::EndNode();

    for (int i = 0; i < s_inputs.size(); ++i) {
      auto& node = s_inputs[i];
      int id = i * 100;
      draw(node, id);
    }

    for (int i = 0; i < s_conv.size(); ++i) {
      auto& node = s_conv[i];
      int id = i * 100 + 10000;
      draw(node, id);
    }

    {
      auto& node = s_output;
      int id = -1000;
      draw(node, id);
    }

    for (int i = 0; i < s_edges.size(); ++i) {
      auto& edge = s_edges[i];
      ImNodes::Link(i, edge.Start, edge.End);
    }

    ImNodes::EndNodeEditor();

    {
      int start_attr, end_attr;
      if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        // const NodeType start_type = graph_.node(start_attr).type;
        // const NodeType end_type = graph_.node(end_attr).type;
        //
        // const bool valid_link = start_type != end_type;
        // if (valid_link)
        {
          // Ensure the edge is always directed from the value to
          // whatever produces the value
          // if (start_type != NodeType::value) {
          //   std::swap(start_attr, end_attr);
          // }
          s_edges.push_back({ start_attr, end_attr });
        }
      }
    }
  }));
}
