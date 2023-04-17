#include "export_dock.h"
#include "json_dock_impl.h"
#include "node_label.h"
#include <imgui.h>
#include <vrm/exporter.h>
#include <vrm/glb.h>
#include <vrm/scene.h>

void
ExportDock::Create(const AddDockFunc& addDock,
                   std::string_view title,
                   const std::shared_ptr<libvrm::gltf::Scene>& scene,
                   float indent)
{
  auto debug_scene = std::make_shared<libvrm::gltf::Scene>();
  auto impl = std::make_shared<JsonDockImpl>(scene);

  addDock(Dock(title, [scene, debug_scene, impl, indent]() {
    if (ImGui::Button("export scene")) {
      libvrm::gltf::Exporter exporter;
      exporter.Export(*scene);
      debug_scene->Load(exporter.JsonChunk, exporter.BinChunk);
    }

    // json tree
    impl->Show(debug_scene, indent);
  }));
}
