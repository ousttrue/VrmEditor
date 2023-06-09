#pragma once
#include "printfbuffer.h"
#include <array>
#include <gltfjson.h>
#include <grapho/imgui/widgets.h>

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

template<typename T>
bool
ParentKey(const char* label,
          const gltfjson::tree::NodePtr& parentNode,
          const std::u8string_view key,
          const T& showGui)
{
  if (!parentNode) {
    return false;
  }

  auto node = parentNode->Get(key);

  auto updated = showGui(label, node);
  if (updated) {
    if (node) {
      node->Set(*updated);
    } else {
      parentNode->Add(key, *updated);
    }
  }

  return updated ? true : false;
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

inline bool
ShowGuiSliderFloat(const char* label,
                   const gltfjson::tree::NodePtr& parentNode,
                   std::u8string_view key,
                   float min,
                   float max,
                   float defalutValue)
{
  return ParentKey(
    label, parentNode, key, [min, max, defalutValue](auto label, auto node) {
      return ShowGuiSliderFloat(label, node, min, max, defalutValue);
    });
}

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
    node = parentNode->Add(key, 0.0f);
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

inline bool
ShowGuiStringEnum(const char* label,
                  const gltfjson::tree::NodePtr& parentNode,
                  std::u8string_view key,
                  std::span<const char*> items)
{
  return ParentKey(label, parentNode, key, [items](auto label, auto node) {
    return ShowGuiStringEnum(label, node, items);
  });
}

// emissive
std::optional<std::array<float, 3>>
ShowGuiColor3(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 3>& defaultColor);

inline bool
ShowGuiColor3(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key,
              const std::array<float, 3>& defaultColor)
{
  return ParentKey(
    label, parentNode, key, [defaultColor](auto label, auto node) {
      return ShowGuiColor3(label, node, defaultColor);
    });
}

// baseColor
std::optional<std::array<float, 4>>
ShowGuiColor4(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 4>& defaultValue);

inline bool
ShowGuiColor4(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key,
              const std::array<float, 4>& defaultColor)
{
  return ParentKey(
    label, parentNode, key, [defaultColor](auto label, auto node) {
      return ShowGuiColor4(label, node, defaultColor);
    });
}

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

inline bool
SelectId(const char* label,
         const gltfjson::tree::NodePtr& idParent,
         std::u8string_view key,
         const gltfjson::tree::NodePtr& arrayNode)
{
  return ParentKey(label, idParent, key, [arrayNode](auto label, auto node) {
    return SelectId(label, node, arrayNode).transform([](auto x) {
      return (float)x;
    });
  });
}

template<typename T>
bool
ListId(const char* label, std::vector<uint32_t>& list, const T& values)
{
  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  if (ImGui::CollapsingHeader(label)) {
    ImGui::Indent();
    std::stringstream ss;
    uint32_t i = 0;
    PrintfBuffer buf;
    for (auto it = list.begin(); it != list.end(); ++i) {
      ImGui::TextUnformatted(buf.Printf("%s[%d] %s",
                                        values.Name.c_str(),
                                        i,
                                        (const char*)values[*it].Name.c_str()));
      ImGui::SameLine();

      if (ImGui::Button(buf.Printf("x##%d", i))) {
        it = list.erase(it);
      } else {
        ++it;
      }
    }

    std::optional<uint32_t> new_value;
    // SelectId("+", &new_value, values);
    if (new_value) {
      list.push_back(*new_value);
    }
    ImGui::Unindent();
  }
  return false;
}

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
