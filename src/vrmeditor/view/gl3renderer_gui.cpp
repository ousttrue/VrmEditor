#include <GL/glew.h>

#include "gl3renderer_gui.h"
#include <TextEditor.h>
#include <glr/gl3renderer.h>
#include <glr/material.h>
#include <grapho/gl3/glsl_type_name.h>
#include <grapho/imgui/csscolor.h>
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <vector>

namespace glr {

struct ShaderMacroVisitor
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
    if (ImGui::Checkbox((const char*)Def.Name.c_str(), &var.LastValue)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(IntVar& var)
  {
    if (ImGui::InputInt((const char*)Def.Name.c_str(), &var.LastValue)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(FloatVar& var)
  {
    if (ImGui::InputFloat((const char*)Def.Name.c_str(), &var.LastValue)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(StringVar& var)
  {
    if (ImGui::InputText((const char*)Def.Name.c_str(), &var.LastValue)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
};

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
        if (std::visit(ShaderMacroVisitor{ m }, m.Value)) {
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
Gl3RendererGui::ShowShaderSource(Material& material)
{
  ImGui::TextUnformatted(material.Name.c_str());

  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
    if (ImGui::BeginTabItem("VS")) {
      if (ShowShader(material, material.VS, m_vsEditor)) {
        material.Compiled = {}; //std::unexpected{ "" };
        m_vsEditor.SetText("");
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("FS")) {
      if (ShowShader(material, material.FS, m_fsEditor)) {
        material.Compiled = {}; //std::unexpected{ "" };
        m_fsEditor.SetText("");
      }
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
}

// std::variant<IntVar, FloatVar, Vec3Var, Vec4Var, Mat3Var, Mat4Var>;
struct UniformVisitor
{
  // ShaderMacro& Def;
  const char* Name;

  bool operator()(IntVar& var)
  {
    if (ImGui::InputInt(Name, &var.LastValue)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(FloatVar& var)
  {
    if (ImGui::InputFloat(Name, &var.LastValue)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(Vec3Var& var)
  {
    if (ImGui::InputFloat3(Name, &var.LastValue.x)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(Vec4Var& var)
  {
    if (ImGui::InputFloat4(Name, &var.LastValue.x)) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(RgbVar& var)
  {
    if (ImGui::ColorEdit3(Name, &var.LastValue[0])) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(RgbaVar& var)
  {
    if (ImGui::ColorEdit4(Name, &var.LastValue[0])) {
      var.Override(var.LastValue);
      return true;
    }
    return false;
  }
  bool operator()(Mat3Var& var)
  {
    ImGui::TextUnformatted("Mat3");
    return false;
  }
  bool operator()(Mat4Var& var)
  {
    ImGui::TextUnformatted("Mat4");
    return false;
  }
};

void
Gl3RendererGui::ShowShaderVariables(Material& factory)
{
  bool updated = false;
  if (auto compiled = factory.Compiled) {
    auto shader = compiled;

    ImGui::TextUnformatted("uniform variables");
    std::array<const char*, 5> cols = {
      "index", "location", "type", "name", "binding"
    };
    if (grapho::imgui::BeginTableColumns("uniforms", cols)) {
      grapho::imgui::PrintfBuffer buf;
      ImGui::PushItemWidth(-1);
      for (int i = 0; i < shader->Uniforms.size(); ++i) {
        ImGui::TableNextRow();
        auto& u = shader->Uniforms[i];
        if (u.Location == -1) {
          ImGui::BeginDisabled(true);
          ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::gray);
        }
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", u.Location);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(grapho::gl3::ShaderTypeName(u.Type));
        ImGui::TableSetColumnIndex(3);
        ImGui::TextUnformatted(u.Name.c_str());
        if (auto& u = factory.UniformVars[i]) {
          ImGui::TableSetColumnIndex(4);
          if (std::visit(UniformVisitor{ buf.Printf("##uniform_%d", i) }, *u)) {
            updated = true;
          }
        }
        if (u.Location == -1) {
          ImGui::PopStyleColor();
          ImGui::EndDisabled();
        }
      }
      ImGui::PopItemWidth();
      ImGui::EndTable();
    }
  } else {
    // auto error = factory.Compiled.error();
    // if (error.size()) {
    //   ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    //   if (ImGui::CollapsingHeader("Error")) {
    //     ImGui::TextWrapped("error: %s", error.c_str());
    //   }
    // }
  }
}

void
Gl3RendererGui::Select(uint32_t i)
{
  if (i == m_selected) {
    return;
  }
  m_selected = i;
  if (MaterialMap()[m_selected]) {
    m_vsEditor.SetText("");
    m_vsEditor.SetReadOnly(true);
    m_fsEditor.SetText("");
    m_fsEditor.SetReadOnly(true);
  }
}

static bool
ShowSelectImpl(const std::vector<MaterialFactory>& list, uint32_t* value)
{
  bool updated = false;
  ImGui::Indent();
  for (uint32_t i = 0; i < list.size(); ++i) {
    auto& f = list[i];
    if (ImGui::Selectable(f.Name.c_str(), i == *value)) {
      *value = i;
      updated = true;
    }
  }
  ImGui::Unindent();
  return updated;
}

void
Gl3RendererGui::ShowSelectImpl()
{
  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  // if (ImGui::CollapsingHeader("PBR")) {
  //   if (ShowSelectImpl(m_pbrFactories, &m_pbrFactoriesCurrent)) {
  //     m_materialMap.clear();
  //   }
  // }
  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  // if (ImGui::CollapsingHeader("UNLIT")) {
  //   if (ShowSelectImpl(m_unlitFactories, &m_unlitFactoriesCurrent)) {
  //     m_materialMap.clear();
  //   }
  // }
  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  // if (ImGui::CollapsingHeader("MToon0")) {
  //   if (ShowSelectImpl(m_mtoon0Factories, &m_mtoon0FactoriesCurrent)) {
  //     m_materialMap.clear();
  //   }
  // }
  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  // if (ImGui::CollapsingHeader("MToon1")) {
  //   if (ShowSelectImpl(m_mtoon1Factories, &m_mtoon1FactoriesCurrent)) {
  //     m_materialMap.clear();
  //   }
  // }
}

void
Gl3RendererGui::ShowSelector()
{
  for (uint32_t i = 0; i < MaterialMap().size(); ++i) {
    grapho::imgui::PrintfBuffer buf;
    if (ImGui::Selectable(buf.Printf("%d", i), i == m_selected)) {
      Select(i);
    }
  }
}

void
Gl3RendererGui::ShowSelectedShaderSource()
{
  ImGui::Text("%d", m_selected);
  if (m_selected >= MaterialMap().size()) {
    return;
  }

  if (auto factory = MaterialMap()[m_selected]) {
    ShowShaderSource(*factory);
  } else {
    ImGui::TextUnformatted("nullopt");
  }
}

void
Gl3RendererGui::ShowSelectedShaderVariables()
{
  ImGui::Text("%d", m_selected);
  if (m_selected >= MaterialMap().size()) {
    return;
  }

  if (auto factory = MaterialMap()[m_selected]) {
    ShowShaderVariables(*factory);
  }
}

} // namespace
