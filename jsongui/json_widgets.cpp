#include "json_widgets.h"
#include <array>
#include <glr/gl3renderer.h>

struct InputTextCallback_UserData
{
  std::u8string* Str;
  ImGuiInputTextCallback ChainCallback;
  void* ChainCallbackUserData;
};

static int
InputTextCallback(ImGuiInputTextCallbackData* data)
{
  InputTextCallback_UserData* user_data =
    (InputTextCallback_UserData*)data->UserData;
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    // Resize string callback
    // If for some reason we refuse the new length (BufTextLen) and/or capacity
    // (BufSize) we need to set them back to what we want.
    std::u8string* str = user_data->Str;
    IM_ASSERT(data->Buf == (const char*)str->c_str());
    str->resize(data->BufTextLen);
    data->Buf = (char*)str->c_str();
  } else if (user_data->ChainCallback) {
    // Forward to user callback, if any
    data->UserData = user_data->ChainCallbackUserData;
    return user_data->ChainCallback(data);
  }
  return 0;
}

bool
InputU8Text(const char* label,
            std::u8string* str,
            ImGuiInputTextFlags flags,
            ImGuiInputTextCallback callback,
            void* user_data)
{
  IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
  flags |= ImGuiInputTextFlags_CallbackResize;

  InputTextCallback_UserData cb_user_data;
  cb_user_data.Str = str;
  cb_user_data.ChainCallback = callback;
  cb_user_data.ChainCallbackUserData = user_data;
  return ImGui::InputText(label,
                          (char*)str->c_str(),
                          str->capacity() + 1,
                          flags,
                          InputTextCallback,
                          &cb_user_data);
}

bool
ShowGuiString(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key)
{
  if (!parentNode) {
    return false;
  }

  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, u8"");
  }
  auto p = node->Ptr<std::u8string>();
  if (!p) {
    node->Var = u8"";
    p = node->Ptr<std::u8string>();
  }
  return InputU8Text(label, p);
}

bool
ShowGuiBool(const char* label,
            const gltfjson::tree::NodePtr& parentNode,
            std::u8string_view key)
{
  if (!parentNode) {
    return false;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, false);
  }

  auto p = node->Ptr<bool>();
  if (!p) {
    node->Var = false;
    p = node->Ptr<bool>();
  }

  return ImGui::Checkbox(label, p);
}

bool
ShowGuiUInt32(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key)
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

  auto value = (uint32_t)*p;
  if (ImGui::InputScalar(label, ImGuiDataType_U32, &value)) {
    *p = (float)value;
    return true;
  } else {
    return false;
  }
}

std::optional<float>
ShowGuiSliderFloat(const char* label,
                   const gltfjson::tree::NodePtr& node,
                   float min,
                   float max,
                   float defaultValue)
{
  assert(node);
  float value = defaultValue;
  if (auto p = node->Ptr<float>()) {
    value = *p;
  }

  if (ImGui::SliderFloat(label, &value, min, max)) {
    if (node) {
      node->Var = value;
    }
    return value;
  } else {
    return std::nullopt;
  }
}

std::optional<std::u8string>
ShowGuiStringEnum(const char* label,
                  const gltfjson::tree::NodePtr& node,
                  // const gltfjson::tree::NodePtr& parentNode,
                  // std::u8string_view key,
                  std::span<const char*> items)
{
  // if (!parentNode) {
  //   return false;
  // }
  // auto node = parentNode->Get(key);
  // if (!node) {
  //   node = parentNode->Add(key, u8"");
  // }

  // int value = 0;
  // if (node) {
  //   if (auto p = node->Ptr<std::u8string>()) {
  //     for (int i = 0; i < items.size(); ++i) {
  //       if (*p == (const char8_t*)items[i]) {
  //         value = i;
  //         break;
  //       }
  //     }
  //   }
  // }
  assert(node);
  auto p = node->Ptr<std::u8string>();
  if (!p) {
    node->Var = u8"";
    p = node->Ptr<std::u8string>();
  }

  int i = 0;
  for (; i < items.size(); ++i) {
    auto str = items[i];
    if (*p == gltfjson::to_u8(str)) {
      break;
    }
  }

  std::vector<std::tuple<int, std::string>> combo;
  int j = 0;
  for (auto l : items) {
    combo.push_back({ j++, l });
  }

  if (grapho::imgui::GenericCombo<int>(label, &i, combo)) {
    auto value = std::get<1>(combo[i]);
    *p = gltfjson::to_u8(value);
    return std::u8string((const char8_t*)value.data(), value.size());
  } else {
    return {};
  }
}

std::optional<std::array<float, 3>>
ShowGuiColor3(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 3>& defaultValue)
{
  auto value = defaultValue;
  if (node) {
    if (auto p = node->Ptr<gltfjson::tree::ArrayValue>()) {
      if (p->size() == 3) {
        if (auto p0 = (*p)[0]->Ptr<float>()) {
          if (auto p1 = (*p)[1]->Ptr<float>()) {
            if (auto p2 = (*p)[2]->Ptr<float>()) {
              value = { *p0, *p1, *p2 };
            }
          }
        }
      }
    }
  }

  if (ImGui::ColorEdit3(label, &value[0])) {
    if (node) {
      node->Set(value);
    }
    return value;
  } else {
    return {};
  }
}

std::optional<std::array<float, 4>>
ShowGuiColor4(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 4>& defaultValue)
{
  auto value = defaultValue;
  if (node) {
    if (auto p = node->Ptr<gltfjson::tree::ArrayValue>()) {
      if (p->size() == 4) {
        if (auto p0 = (*p)[0]->Ptr<float>()) {
          if (auto p1 = (*p)[1]->Ptr<float>()) {
            if (auto p2 = (*p)[2]->Ptr<float>()) {
              if (auto p3 = (*p)[3]->Ptr<float>()) {
                value = { *p0, *p1, *p2, *p3 };
              }
            }
          }
        }
      }
    }
  }

  if (ImGui::ColorEdit4(label, &value[0])) {
    if (node) {
      node->Set(value);
    }
    return value;
  } else {
    return {};
  }
}

bool
ShowGuiFloat3(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 3>& defaultValue)
{
  if (!node) {
    return false;
  }

  auto values = FillArray<3>(node, defaultValue);
  if (ImGui::InputFloat3(label, &values[0])) {
    node->Set(values);
    return true;
  } else {
    return false;
  }
}

bool
ShowGuiFloat4(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 4>& defaultValue)
{
  if (!node) {
    return false;
  }

  auto values = FillArray<4>(node, defaultValue);
  if (ImGui::InputFloat4(label, &values[0])) {
    node->Set(values);
    return true;
  } else {
    return false;
  }
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
void
HelpMarker(const char* desc)
{
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) &&
      ImGui::BeginTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

std::optional<uint32_t>
SelectId(const char* label,
         const gltfjson::tree::NodePtr& node,
         const gltfjson::tree::NodePtr& arrayNode)
{
  if (!arrayNode) {
    return false;
  }
  auto values = arrayNode->Array();
  if (!values) {
    return false;
  }

  float* id = nullptr;
  if (node) {
    id = node->Ptr<float>();
  }

  auto selected = (uint32_t)(id ? *id : -1);
  using TUPLE = std::tuple<uint32_t, std::string>;
  std::vector<TUPLE> combo;
  grapho::imgui::PrintfBuffer buf;
  for (int i = 0; i < values->size(); ++i) {
    auto item = (*values)[i];
    const gltfjson::tree::NodePtr name = item->Get(u8"name");
    combo.push_back(
      { i,
        buf.Printf(
          "[%d] %s", i, (name ? (const char*)name->U8String().c_str() : "")) });
  }
  std::span<const TUPLE> span(combo.data(), combo.size());

  bool updated = grapho::imgui::GenericCombo<uint32_t>(label, &selected, span);
  if (updated) {
    if (node) {
      node->Var = (float)selected;
    }
    return selected;
  } else {
    return std::nullopt;
  }
}

// bool
// SelectId(const char* label,
//          const gltfjson::tree::NodePtr& idParent,
//          const char8_t* key,
//          const gltfjson::tree::NodePtr& arrayNode)
// {
//   if (!idParent) {
//     return false;
//   }
//
//   auto node = idParent->Get(key);
//   auto selected = SelectId(label, node, arrayNode);
//   if (selected) {
//     if (!node) {
//       idParent->Add(key, (float)*selected);
//     }
//   }
//   return selected ? true : false;
// }

bool
ShowGuiVectorFloat(const char* label,
                   const gltfjson::tree::NodePtr& parentNode,
                   std::u8string_view key,
                   const std::function<bool(size_t i, float* p)>& showGui)
{
  if (!parentNode) {
    return false;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, gltfjson::tree::ArrayValue{});
  }
  auto array = node->Array();
  if (!array) {
    node->Var = gltfjson::tree::ArrayValue{};
    array = node->Array();
  }

  bool updated = false;
  if (ImGui::CollapsingHeader(label)) {
    ImGui::Indent();
    ImGui::PushID(node.get());
    grapho::imgui::PrintfBuffer buf;
    size_t i = 0;
    for (auto it = array->begin(); it != array->end(); ++i) {
      auto p = (*it)->Ptr<float>();
      if (!p) {
        (*it)->Var = 0.0f;
        p = (*it)->Ptr<float>();
      }
      if (showGui(i, p)) {
        updated = true;
      }
      ImGui::SameLine();
      if (ImGui::Button(buf.Printf("x##%s_%d", label, i))) {
        it = array->erase(it);
      } else {
        ++it;
      }
    }

    if (ImGui::Button(buf.Printf("+##%s_%d", label, i))) {
      array->push_back({});
    }
    ImGui::PopID();
    ImGui::Unindent();
  }
  return updated;
}

bool
ShowGuiOptional(
  const gltfjson::tree::NodePtr& parentNode,
  const char8_t* key,
  const std::function<bool(const gltfjson::tree::NodePtr&)>& showGui)
{
  if (!parentNode) {
    return false;
  }

  bool updated = false;
  grapho::imgui::PrintfBuffer buf;
  if (auto node = parentNode->Get(key)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    bool visible = true;
    if (ImGui::CollapsingHeader((const char*)key, &visible)) {
      ImGui::PushID(node.get());
      ImGui::Indent();
      if (showGui(node)) {
        updated = true;
      }
      ImGui::Unindent();
      ImGui::PopID();
    }
    if (!visible) {
      parentNode->Remove(key);
      updated = true;
    }
  } else {
    if (ImGui::Button(buf.Printf("%s +", (const char*)key))) {
      auto node = parentNode->Add(key, gltfjson::tree::ObjectValue{});
      updated = true;
    }
  }
  return updated;
}

void
ShowGuiTexturePreview(const gltfjson::Root& root,
                      const gltfjson::Bin& bin,
                      const gltfjson::tree::NodePtr& parentNode,
                      const char8_t* key)
{
  if (!parentNode) {
    return;
  }

  auto node = parentNode->Get(key);
  if (node) {
    if (auto p = node->Ptr<float>()) {
      if (auto texture = glr::GetOrCreateTextureHandle(
            root, bin, (uint32_t)*p, glr::ColorSpace::Linear)) {
        ImGui::Image((ImTextureID)(uint64_t)*texture, { 150, 150 });
        return;
      }
    }
  }

  auto pos = ImGui::GetCursorPos();
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRect(pos, { pos.x + 150, pos.y + 150 }, ImColor());
}
