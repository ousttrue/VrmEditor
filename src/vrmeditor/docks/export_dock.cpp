#include "export_dock.h"
#include "glr/scene_preview.h"
#include "json_gui.h"
#include "node_label.h"
#include <fstream>
#include <grapho/orbitview.h>
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
  auto impl = std::make_shared<JsonGui>(debug_scene);
  auto view = std::make_shared<grapho::OrbitView>();
  auto context = std::make_shared<libvrm::gltf::SceneContext>();

  auto preview =
    std::make_shared<glr::ScenePreview>(debug_scene, view, context);

  addDock(Dock(title, [scene, debug_scene, impl, indent, preview]() {
    auto pos = ImGui::GetCursorScreenPos();
    preview->Show("debug", DirectX::XMFLOAT4{ pos.x, pos.y, 300, 300 });

    if (ImGui::Button("export scene")) {
      libvrm::gltf::Exporter exporter;
      exporter.Export(*scene);

      std::stringstream ss;
      libvrm::gltf::Glb{
        .JsonChunk = exporter.JsonChunk,
        .BinChunk = exporter.BinChunk,
      }
        .WriteTo(ss);
      auto str = ss.str();

#ifndef NDEBUG
      std::ofstream j("tmp.json", std::ios::binary);
      j.write((const char*)exporter.JsonChunk.data(),
              exporter.JsonChunk.size());

      std::ofstream w("tmp.glb", std::ios::binary);
      w.write(str.data(), str.size());
#endif

      debug_scene->LoadBytes({ (const uint8_t*)str.data(), str.size() });
    }

    // json tree
    impl->Show(indent);
  }));
}
