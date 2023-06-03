#include <GL/glew.h>

#include "gl3renderer_gui.h"
#include <TextEditor.h>
#include <grapho/gl3/glsl_type_name.h>
#include <grapho/imgui/csscolor.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace glr {

static bool
ShowShader(Material& f, ShaderFactory& s, TextEditor& editor)
{
  bool updated = false;
  ImGui::TextUnformatted(s.SourceName.c_str());

  std::vector<std::tuple<int, std::string>> combo;
  for (auto& e : s.Enums) {
    combo.clear();
    for (auto& kv : e.Values) {
      combo.push_back(
        { kv.Value,
          std::string{ (const char*)kv.Name.data(), kv.Name.size() } });
    }
    auto& var = std::get<IntVar>(e.Selected.Value);
    auto value = var.LastValue;
    if (grapho::imgui::GenericCombo<int>(
          (const char*)e.Selected.Name.c_str(), &value, combo)) {
      var.Override(value);
      updated = true;
    }
  }
  for (auto& g : s.MacroGroups) {
    if (ImGui::CollapsingHeader(g.first.c_str())) {
      for (auto& m : g.second) {
        struct Visitor
        {
          ShaderMacro& Def;

          bool operator()(OptVar& var)
          {
            bool value = var.LastValue ? true : false;
            if (ImGui::Checkbox((const char*)Def.Name.c_str(), &value)) {
              if (value) {
                var.Override(std::monostate{});
              } else {
                var.Override(std::nullopt);
              }
              return true;
            } else {
              return false;
            }
          }
          bool operator()(BoolVar& var)
          {
            if (ImGui::Checkbox((const char*)Def.Name.c_str(),
                                &var.LastValue)) {
              var.Override(var.LastValue);
              return true;
            }
            return false;
          }
          bool operator()(IntVar& var)
          {
            if (ImGui::InputInt((const char*)Def.Name.c_str(),
                                &var.LastValue)) {
              var.Override(var.LastValue);
              return true;
            }
            return false;
          }
          bool operator()(FloatVar& var)
          {
            if (ImGui::InputFloat((const char*)Def.Name.c_str(),
                                  &var.LastValue)) {
              var.Override(var.LastValue);
              return true;
            }
            return false;
          }
          bool operator()(StringVar& var)
          {
            if (ImGui::InputText((const char*)Def.Name.c_str(),
                                 &var.LastValue)) {
              var.Override(var.LastValue);
              return true;
            }
            return false;
          }
        };

        if (std::visit(Visitor{ m }, m.Value)) {
          updated = true;
        }
      }
    }
  }

  if (editor.GetText() == "\n") {
    editor.SetText((const char*)s.FullSource.c_str());
    editor.SetReadOnly(true);
  }

  if (ImGui::CollapsingHeader(s.SourceName.c_str())) {
    editor.Render(s.SourceName.c_str());
  }

  return updated;
}

void
ShowShaderSource(Material& factory,
                 TextEditor& vsEditor,
                 TextEditor& fsEditor)
{
  switch (factory.Type) {
    case ShaderTypes::Pbr:
      ImGui::TextUnformatted("pbr");
      break;
    case ShaderTypes::Unlit:
      ImGui::TextUnformatted("unlit");
      break;
    case ShaderTypes::MToon0:
      ImGui::TextUnformatted("mtoon0");
      break;
    case ShaderTypes::MToon1:
      ImGui::TextUnformatted("mtoon1");
      break;
  }

  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
    if (ImGui::BeginTabItem("VS")) {
      if (ShowShader(factory, factory.VS, vsEditor)) {
        factory.Compiled = std::unexpected{ "" };
        vsEditor.SetText("");
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("FS")) {
      if (ShowShader(factory, factory.FS, fsEditor)) {
        factory.Compiled = std::unexpected{ "" };
        fsEditor.SetText("");
      }
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
}

void
ShowShaderVariables(Material& factory)
{
  if (auto compiled = factory.Compiled) {
    auto shader = *compiled;

    ImGui::TextUnformatted("uniform variables");
    std::array<const char*, 5> cols = {
      "index", "location", "type", "name", "binding"
    };
    if (grapho::imgui::BeginTableColumns("uniforms", cols)) {
      for (int i = 0; i < shader->Uniforms.size(); ++i) {
        ImGui::TableNextRow();
        auto& u = shader->Uniforms[i];
        if (u.Location == -1) {
          ImGui::BeginDisabled(true);
          ImGui::PushStyleColor(ImGuiCol_Text, grapho::imgui::gray);
        }
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", u.Location);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(grapho::gl3::ShaderTypeName(u.Type));
        ImGui::TableSetColumnIndex(3);
        ImGui::TextUnformatted(u.Name.c_str());
        if (factory.UniformVars[i]) {
          ImGui::TableSetColumnIndex(4);
          ImGui::TextUnformatted("OK");
        }
        if (u.Location == -1) {
          ImGui::PopStyleColor();
          ImGui::EndDisabled();
        }
      }
      ImGui::EndTable();
    }
  } else {
    auto error = factory.Compiled.error();
    if (error.size()) {
      ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
      if (ImGui::CollapsingHeader("Error")) {
        ImGui::TextWrapped("error: %s", error.c_str());
      }
    }
  }
}

}
