#pragma once
#include "json_gui.h"
#include "json_widgets.h"
#include <array>
#include <gltfjson.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <gltfjson/jsonpath.h>
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <span>
#include <vector>

bool
InputU8Text(const char* label,
            std::u8string* str,
            ImGuiInputTextFlags flags = 0,
            ImGuiInputTextCallback callback = nullptr,
            void* user_data = nullptr);

template<size_t N>
static std::array<float, N>
FillArray(const gltfjson::tree::NodePtr& node,
          const std::array<float, N>& defautColor)
{
  std::array<float, N> values;
  auto array = node->Array();
  if (!array) {
    node->Var = gltfjson::tree::ArrayValue{};
    array = node->Array();
  }
  for (int i = 0; i < N; ++i) {
    if (i < array->size()) {
      if (auto child = (*array)[i]) {
        if (auto n = child->Ptr<float>()) {
          values[i] = *n;
        } else {
          child->Var = defautColor[i];
          values[i] = defautColor[i];
        }
      } else {
        assert(false);
      }
    } else {
      auto new_child = std::make_shared<gltfjson::tree::Node>();
      new_child->Var = defautColor[i];
      values[i] = defautColor[i];
      array->push_back(new_child);
    }
  }
  return values;
}

bool
ShowGuiString(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key);

bool
ShowGuiBool(const char* label,
            const gltfjson::tree::NodePtr& parentNode,
            std::u8string_view key);

bool
ShowGuiUInt32(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key);

std::optional<float>
ShowGuiSliderFloat(const char* label,
                   const gltfjson::tree::NodePtr& node,
                   float min,
                   float max,
                   float defalutValue);

template<typename T>
inline bool
ShowGuiEnum(const char* label,
            const gltfjson::tree::NodePtr& parentNode,
            std::u8string_view key,
            std::span<const std::tuple<T, const char*>> combo)
{
  if (!parentNode) {
    return false;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->SetProperty(key, 0.0f);
  }

  auto p = node->Ptr<float>();
  if (!p) {
    node->Var = 0.0f;
    p = node->Ptr<float>();
  }

  auto value = (T)*p;
  if (grapho::imgui::EnumCombo(label, &value, combo)) {
    *p = (float)value;
    return true;
  } else {
    return false;
  }
}

std::optional<std::u8string>
ShowGuiStringEnum(const char* label,
                  const gltfjson::tree::NodePtr& node,
                  // const gltfjson::tree::NodePtr& parentNode,
                  // std::u8string_view key,
                  std::span<const char*> items);

// emissive
std::optional<std::array<float, 3>>
ShowGuiColor3(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 3>& defaultColor);

// baseColor
std::optional<std::array<float, 4>>
ShowGuiColor4(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 4>& defaultValue);

bool
ShowGuiFloat3(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 3>& defaultValue);

bool
ShowGuiFloat4(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 4>& defaultValue);

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
void
HelpMarker(const char* desc);

std::optional<uint32_t>
SelectId(const char* label,
         const gltfjson::tree::NodePtr& node,
         const gltfjson::tree::NodePtr& arrayNode);

// template<typename T>
// bool
// ListId(const char* label, std::vector<uint32_t>& list, const T& values)
// {
//   ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
//   if (ImGui::CollapsingHeader(label)) {
//     ImGui::Indent();
//     std::stringstream ss;
//     uint32_t i = 0;
//     PrintfBuffer buf;
//     for (auto it = list.begin(); it != list.end(); ++i) {
//       ImGui::TextUnformatted(buf.Printf("%s[%d] %s",
//                                         values.Name.c_str(),
//                                         i,
//                                         (const
//                                         char*)values[*it].Name.c_str()));
//       ImGui::SameLine();
//
//       if (ImGui::Button(buf.Printf("x##%d", i))) {
//         it = list.erase(it);
//       } else {
//         ++it;
//       }
//     }
//
//     std::optional<uint32_t> new_value;
//     // SelectId("+", &new_value, values);
//     if (new_value) {
//       list.push_back(*new_value);
//     }
//     ImGui::Unindent();
//   }
//   return false;
// }

bool
ShowGuiVectorFloat(const char* label,
                   const gltfjson::tree::NodePtr& parentNode,
                   std::u8string_view key,
                   const std::function<bool(size_t i, float* p)>& showGui);

bool
ShowGuiOptional(
  const gltfjson::tree::NodePtr& parentNode,
  const char8_t* key,
  const std::function<bool(const gltfjson::tree::NodePtr&)>& showGui);

void
ShowGuiTexturePreview(const gltfjson::Root& root,
                      const gltfjson::Bin& bin,
                      const gltfjson::tree::NodePtr& parentNode,
                      const char8_t* key);

inline ShowGuiFunc
SelectSampler(std::u8string_view jsonpath)
{
  return [](const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& node) {
    if (SelectId("Sampler", node, root.Samplers.m_json)) {
      return true;
    } else {
      return false;
    }
  };
}

inline bool
SelectMesh(const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           const gltfjson::tree::NodePtr& node)
{
  if (SelectId("##_SelectMesh", node, root.Meshes.m_json)) {
    return true;
  } else {
    return false;
  }
}

inline ShowGuiFunc
SelectTexture(std::u8string_view jsonpath)
{
  return [](const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& node) {
    if (SelectId("Source", node, root.Textures.m_json)) {
      return true;
    } else {
      return false;
    }
  };
}

struct FloatSlider
{
  float Min = 0;
  float Max = 1;
  float Default = 0.5f;

  bool operator()(const gltfjson::Root& root,
                  const gltfjson::Bin& bin,
                  const gltfjson::tree::NodePtr& node)
  {
    return ShowGuiSliderFloat("##_FloatSLider", node, Min, Max, Default)
             ? true
             : false;
  }
};

struct RgbPicker
{
  std::array<float, 3> Default = { 1, 1, 1 };

  bool operator()(const gltfjson::Root& root,
                  const gltfjson::Bin& bin,
                  const gltfjson::tree::NodePtr& node)
  {
    return ShowGuiColor3("##_RgbPicker", node, Default) ? true : false;
  }
};

struct RgbaPicker
{
  std::array<float, 4> Default = { 1, 1, 1, 1 };

  bool operator()(const gltfjson::Root& root,
                  const gltfjson::Bin& bin,
                  const gltfjson::tree::NodePtr& node)
  {
    return ShowGuiColor4("##_RgbaPicker", node, Default) ? true : false;
  }
};

struct StringEnum
{
  std::vector<const char*> Values;

  bool operator()(const gltfjson::Root& root,
                  const gltfjson::Bin& bin,
                  const gltfjson::tree::NodePtr& node)
  {
    return ShowGuiStringEnum("##_StringEnum", node, Values) ? true : false;
  }
};

bool
ShowGuiVrm0Meta(const gltfjson::vrm0::Meta& meta);
bool
ShowGuiVrm1Meta(const gltfjson::vrm1::Meta& meta);
