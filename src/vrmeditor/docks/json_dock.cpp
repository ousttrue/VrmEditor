#include "json_dock.h"
#include "json_gui.h"
#include <imgui.h>

void
JsonDock::Create(const AddDockFunc& addDock,
                 std::string_view title,
                 const std::shared_ptr<libvrm::gltf::Scene>& scene,
                 float indent)
{
  auto impl = std::make_shared<JsonGui>(scene);
  addDock(
    Dock(title, [scene, impl, indent](const char* title, bool* p_open) mutable {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      auto is_open = ImGui::Begin(title, p_open);
      ImGui::PopStyleVar();
      if (is_open) {
        impl->Show(scene, indent);
      }
      ImGui::End();
    }));
}
