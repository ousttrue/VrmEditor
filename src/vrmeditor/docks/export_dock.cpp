#include "export_dock.h"
#include "../view/scene_preview.h"
#include <fstream>
#include <gltfjson/glb.h>
#include <imgui.h>
#include <vrm/gltfroot.h>
#include <vrm/importer.h>
#include <vrm/runtime_scene.h>

void
ExportDock::Create(const AddDockFunc& addDock,
                   std::string_view title,
                   const std::shared_ptr<libvrm::RuntimeScene>& scene)
{
  auto debug_table = std::make_shared<libvrm::GltfRoot>();
  // auto impl = std::make_shared<JsonGui>();
  // impl->SetScene(debug_table);

  auto debug_scene = std::make_shared<libvrm::RuntimeScene>(debug_table);
  auto preview = std::make_shared<ScenePreview>();
  preview->SetRuntime(debug_scene);

  addDock({ { title.begin(), title.end() }, [scene, debug_scene, preview]() {
             auto pos = ImGui::GetCursorScreenPos();

             static float color[] = {
               0.4f,
               0.2f,
               0.2f,
               1.0f,
             };
             preview->ShowScreenRect("debug", color, pos.x, pos.y, 300, 300);

             if (ImGui::Button("export scene")) {
               //       libvrm::gltf::Exporter exporter;
               //       exporter.Export(*scene->m_table);
               //
               //       std::stringstream ss;
               //       gltfjson::Glb{
               //         .JsonChunk = exporter.JsonChunk,
               //         .BinChunk = exporter.BinChunk,
               //       }
               //         .WriteTo(ss);
               //       auto str = ss.str();
               //
               // #ifndef NDEBUG
               //       std::ofstream j("tmp.json", std::ios::binary);
               //       j.write((const char*)exporter.JsonChunk.data(),
               //               exporter.JsonChunk.size());
               //
               //       std::ofstream w("tmp.glb", std::ios::binary);
               //       w.write(str.data(), str.size());
               // #endif
               //
               //       libvrm::gltf::LoadBytes(debug_scene->m_table,
               //                               { (const uint8_t*)str.data(),
               //                               str.size()
               //                               });
             }

             // json tree
             // impl->Show(indent);
           } });
}
