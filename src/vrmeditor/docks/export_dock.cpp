#include "export_dock.h"
#include "node_label.h"
#include <imgui.h>
#include <vrm/exporter.h>
#include <vrm/glb.h>
#include <vrm/scene.h>

void
ExportDock::Create(const AddDockFunc& addDock,
                   std::string_view title,
                   const std::shared_ptr<libvrm::gltf::Scene>& scene)
{
  auto debug_scene = std::make_shared<libvrm::gltf::Scene>();

  auto enter = [scene](const std::shared_ptr<libvrm::gltf::Node>& node) {
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    if (node->Children.empty()) {
      node_flags |=
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    }
    // if (context->selected.lock() == node) {
    //   node_flags |= ImGuiTreeNodeFlags_Selected;
    // }

    bool hasRotation = node->InitialTransform.HasRotation();
    if (hasRotation) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
    }

    bool node_open =
      ImGui::TreeNodeEx(&*node, node_flags, "%s", Label(*scene, node).c_str());

    if (hasRotation) {
      ImGui::PopStyleColor();
    }

    // if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    //   context->new_selected = node;
    // }

    return node->Children.size() && node_open;
  };
  auto leave = []() { ImGui::TreePop(); };

  addDock(Dock(title, [scene, debug_scene, enter, leave]() {
    if (ImGui::Button("export scene")) {
      libvrm::gltf::Exporter exporter;
      exporter.Export(*scene);
      // libvrm::gltf::Glb
      // {
      //   .Json = exporter.JsonChunk, .Bin = exporter.BinChunk,
      // }
      // reimport
      debug_scene->Load(exporter.JsonChunk, exporter.BinChunk);
    }

    // scene tree
    debug_scene->Traverse(enter, leave);
  }));
}
