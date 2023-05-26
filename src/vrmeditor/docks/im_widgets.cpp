#include <GL/glew.h>

#include "im_widgets.h"
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

static bool
InputU8Text(const char* label,
            std::u8string* str,
            ImGuiInputTextFlags flags = 0,
            ImGuiInputTextCallback callback = nullptr,
            void* user_data = nullptr)
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

void
ShowGuiString(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key)
{
  if (!parentNode) {
    return;
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
  InputU8Text(label, p);
}

void
ShowGuiBool(const char* label,
            const gltfjson::tree::NodePtr& parentNode,
            std::u8string_view key)
{
  if (!parentNode) {
    return;
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

  ImGui::Checkbox(label, p);
}

void
ShowGuiUInt32(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key)
{
  if (!parentNode) {
    return;
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
  }
}

void
ShowGuiSliderFloat(const char* label,
                   const gltfjson::tree::NodePtr& parentNode,
                   std::u8string_view key,
                   float min,
                   float max,
                   float defaultValue)
{
  if (!parentNode) {
    return;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, defaultValue);
  }

  auto p = node->Ptr<float>();
  if (!p) {
    node->Var = defaultValue;
    p = node->Ptr<float>();
  }

  ImGui::SliderFloat(label, p, min, max);
}

void
ShowGuiStringEnum(const char* label,
                  const gltfjson::tree::NodePtr& parentNode,
                  std::u8string_view key,
                  std::span<const std::tuple<int, std::string>> combo)
{
  if (!parentNode) {
    return;
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

  int i = 0;
  for (; i < combo.size(); ++i) {
    auto& str = std::get<1>(combo[i]);
    if (*p == gltfjson::tree::to_u8(str)) {
      break;
    }
  }

  if (grapho::imgui::GenericCombo(label, &i, combo)) {
    *p = gltfjson::tree::to_u8(std::get<1>(combo[i]));
  }
}

void
ShowGuiColor3(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key,
              const std::array<float, 3>& defaultColor)
{
  if (!parentNode) {
    return;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, gltfjson::tree::ArrayValue{});
  }

  auto values = FillArray<3>(node, defaultColor);
  if (ImGui::ColorEdit4(label, &values[0])) {
    node->Set(values);
  }
}

void
ShowGuiColor4(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key,
              const std::array<float, 4>& defaultColor)
{
  if (!parentNode) {
    return;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, gltfjson::tree::ArrayValue{});
  }

  auto values = FillArray<4>(node, defaultColor);
  if (ImGui::ColorEdit4(label, &values[0])) {
    node->Set(values);
  }
}

void
ShowGuiFloat3(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 3>& defaultValue)
{
  if (!node) {
    return;
  }

  auto values = FillArray<3>(node, defaultValue);
  if (ImGui::InputFloat3(label, &values[0])) {
    node->Set(values);
  }
}

void
ShowGuiFloat4(const char* label,
              const gltfjson::tree::NodePtr& node,
              const std::array<float, 4>& defaultValue)
{
  if (!node) {
    return;
  }

  auto values = FillArray<4>(node, defaultValue);
  if (ImGui::InputFloat4(label, &values[0])) {
    node->Set(values);
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

void
SelectId(const char* label,
         const gltfjson::tree::NodePtr& idParent,
         const char8_t* key,
         const gltfjson::tree::NodePtr& arrayNode)
{
  if (!arrayNode) {
    return;
  }
  auto values = arrayNode->Array();
  if (!values) {
    return;
  }
  if (!idParent) {
    return;
  }

  float* id = nullptr;
  auto idNode = idParent->Get(key);
  if (idNode) {
    id = idNode->Ptr<float>();
  }

  auto selected = (uint32_t)(id ? *id : -1);
  using TUPLE = std::tuple<uint32_t, std::string>;
  std::vector<TUPLE> combo;
  PrintfBuffer buf;
  for (int i = 0; i < values->size(); ++i) {
    auto item = (*values)[i];
    combo.push_back(
      { i,
        buf.Printf(
          "%s[%d] %s", label, i, (const char*)item->U8String().c_str()) });
  }
  std::span<const TUPLE> span(combo.data(), combo.size());

  grapho::imgui::GenericCombo<uint32_t>(label, &selected, span);
  if (selected >= 0 && selected < values->size()) {
    if (id) {
      *id = (float)selected;
    } else {
      idParent->Add(key, (float)selected);
    }
  } else {
    idParent->Remove(key);
  }

  if (id) {
    ImGui::SameLine();
    if (ImGui::Button("x")) {
      idParent->Remove(key);
    }
  }
}

void
ShowGuiVectorFloat(const char* label,
                   const gltfjson::tree::NodePtr& parentNode,
                   std::u8string_view key,
                   const std::function<void(size_t i, float* p)>& showGui)
{
  if (!parentNode) {
    return;
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

  if (ImGui::CollapsingHeader(label)) {
    ImGui::Indent();
    ImGui::PushID(node.get());
    PrintfBuffer buf;
    size_t i = 0;
    for (auto it = array->begin(); it != array->end(); ++i) {
      auto p = (*it)->Ptr<float>();
      if (!p) {
        (*it)->Var = 0.0f;
        p = (*it)->Ptr<float>();
      }
      showGui(i, p);
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
}

void
ShowGuiOptional(
  const gltfjson::tree::NodePtr& parentNode,
  const char8_t* key,
  const std::function<void(const gltfjson::tree::NodePtr&)>& showGui)
{
  if (!parentNode) {
    return;
  }

  PrintfBuffer buf;
  if (auto node = parentNode->Get(key)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    bool visible = true;
    if (ImGui::CollapsingHeader((const char*)key, &visible)) {
      ImGui::PushID(node.get());
      ImGui::Indent();
      showGui(node);
      ImGui::Unindent();
      ImGui::PopID();
    }
    if (!visible) {
      parentNode->Remove(key);
    }
  } else {
    if (ImGui::Button(buf.Printf("%s +", (const char*)key))) {
      auto node = parentNode->Add(key, gltfjson::tree::ObjectValue{});
    }
  }
}

void
ShowGuiTexturePreview(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      const gltfjson::tree::NodePtr& parentNode,
                      const char8_t* key)
{
  if (!parentNode) {
    return;
  }

  auto node = parentNode->Get(key);
  if (node) {
    if (auto p = node->Ptr<float>()) {
      if (auto texture = glr::GetOrCreateTexture(
            root, bin, (uint32_t)*p, libvrm::gltf::ColorSpace::Linear)) {
        ImGui::Image((ImTextureID)(int64_t)texture->Handle(), { 150, 150 });
        return;
      }
    }
  }

  auto pos = ImGui::GetCursorPos();
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRect(pos, { pos.x + 150, pos.y + 150 }, ImColor());
}
