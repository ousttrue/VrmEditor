#include "animation.h"
#include "../../printfbuffer.h"
#include <gltfjson.h>
#include <grapho/imgui/widgets.h>

struct AnimationImpl
{
  uint32_t m_selected = 0;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  bool m_isPlay = false;

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_runtime = runtime;
  }

  void ShowGui()
  {
    ImGui::Text("selected: %d", m_selected);

    // seek
    if (ImGui::Button("⏮")) {
      // rewind
    }
    ImGui::SameLine();
    if (m_isPlay) {
      if (ImGui::Button("⏹")) {
        m_isPlay = false;
      }
    } else {
      if (ImGui::Button("▶")) {
        m_isPlay = true;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("⏭")) {
      // end
    }

    std::array<const char*, 3> cols = {
      "Index",
      "Name",
      "Duration",
    };
    if (grapho::imgui::BeginTableColumns("##_Animations", cols)) {
      if (m_runtime) {
        PrintfBuffer buf;
        auto root = m_runtime->m_table;
        for (int i = 0; i < root->m_gltf->Animations.size(); ++i) {
          auto a = root->m_gltf->Animations[i];
          ImGui::TableNextRow();

          ImGui::TableNextColumn();
          ImGui::Text("%d", i);

          ImGui::TableNextColumn();
          auto name = a.Name();
          // ImGui::Text("%s", (const char*)name.c_str());
          if (ImGui::Selectable(
                buf.Printf("%s##_animation_%d", (const char*)name.c_str(), i),
                i == m_selected)) {
            m_selected = i;
          }

          ImGui::TableNextColumn();
          float duration = 0;
          for (auto s : a.Samplers) {
            if (auto input = s.Input()) {
              if (auto times = root->m_bin.GetAccessorBytes<float>(
                    *root->m_gltf, (int)*input)) {
                if (times->size()) {
                  auto last = times->back();
                  if (last > duration) {
                    duration = last;
                  }
                }
              }
            }
          }
          ImGui::Text("%f", duration);
        }
      }
      ImGui::EndTable();
    }
  }
};

Animation::Animation()
  : m_impl(new AnimationImpl)
{
}

Animation::~Animation()
{
  delete m_impl;
}

void
Animation::SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
{
  m_impl->SetRuntime(runtime);
}

void
Animation::ShowGui()
{
  m_impl->ShowGui();
}
