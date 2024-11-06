#include <imgui.h>
#include <imgui_internal.h>

#ifdef __cplusplus
extern "C" {
#endif

void Custom_ButtonBehaviorMiddleRight() {
  auto &last = ImGui::GetCurrentContext()->LastItemData;
  ImGui::ButtonBehavior(last.Rect, last.ID, nullptr, nullptr,
                        ImGuiButtonFlags_MouseButtonMiddle |
                            ImGuiButtonFlags_MouseButtonRight);
}

#ifdef __cplusplus
}
#endif
