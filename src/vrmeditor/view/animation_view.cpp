#include "animation_view.h"
#include "../../printfbuffer.h"
#include <gltfjson.h>
#include <grapho/imgui/widgets.h>

struct AnimationViewImpl
{
  uint32_t m_selected = -1;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_selected = -1;
    m_runtime = runtime;
  }

  void ShowGui()
  {
    if (!m_runtime) {
      ImGui::TextUnformatted("no scene");
      return;
    }

    // seek
    if (m_selected < m_runtime->m_animations.size()) {
      ImGui::Text("selected: %d", m_selected);
      if (ImGui::Button("⏮")) {
        // rewind
        m_runtime->m_timeline->CurrentTime = {};
      }
      ImGui::SameLine();
      if (m_runtime->m_timeline->IsPlaying) {
        if (ImGui::Button("⏹")) {
          m_runtime->m_timeline->IsPlaying = false;
        }
      } else {
        if (ImGui::Button("▶")) {
          m_runtime->m_timeline->IsPlaying = true;
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("⏭")) {
        // end
        // m_runtime->m_timeline->CurrentTime =
      }

      ImGui::Text("%lf", m_runtime->m_timeline->CurrentTime.count());
    }

    std::array<const char*, 3> cols = {
      "Index",
      "Name",
      "Duration",
    };
    if (grapho::imgui::BeginTableColumns("##_Animations", cols)) {
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
          m_runtime->SetActiveAnimation(i);
        }

        ImGui::TableNextColumn();
        float duration = 0;
        for (auto s : a.Samplers) {
          if (auto input = s.Input()) {
            if (auto times = root->m_bin.GetAccessorBytes<float>(*root->m_gltf,
                                                                 (int)*input)) {
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
      ImGui::EndTable();
    }
  }
};

AnimationView::AnimationView()
  : m_impl(new AnimationViewImpl)
{
}

AnimationView::~AnimationView()
{
  delete m_impl;
}

void
AnimationView::SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
{
  m_impl->SetRuntime(runtime);
}

void
AnimationView::ShowGui()
{
  m_impl->ShowGui();
}
