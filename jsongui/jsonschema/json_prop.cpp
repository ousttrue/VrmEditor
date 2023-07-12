#include "json_prop.h"
#include "../json_widgets.h"
#include <gltfjson/json_tree_exporter.h>

static std::u8string
ToStr(const gltfjson::tree::ArrayValue& array)
{
  std::stringstream ss;
  ss << "[" << array.size() << "]";
  auto str = ss.str();
  return { (const char8_t*)str.c_str(), str.size() };
}

static std::u8string
ToStr(const gltfjson::tree::ObjectValue& object)
{
  std::stringstream ss;
  std::u8string name;
  auto found = object.find(u8"name");
  if (found != object.end()) {
    name = found->second->U8String();
  }
  if (name.size()) {
    ss << "{" << gltfjson::from_u8(name) << "}";
  } else {
    ss << "{...}";
  }
  auto str = ss.str();
  return { (const char8_t*)str.c_str(), str.size() };
}

static std::u8string
ToStr(const gltfjson::tree::NodePtr& item)
{
  if (auto object = item->Object()) {
    return ToStr(*object);
  } else if (auto array = item->Array()) {
    return ToStr(*array);
  } else {
    std::stringstream ss;
    ss << *item;
    auto str = ss.str();
    return { (const char8_t*)str.c_str(), str.size() };
  }
}

std::u8string
JsonValue::TextOrDeault(const gltfjson::tree::NodePtr& item) const
{
  if (item) {
    return ToStr(item);
  } else {
    return DefaultJson;
  }
}

struct NodeTypeEditVisitor
{
  const gltfjson::Root& root;
  const gltfjson::Bin& bin;

  bool operator()(std::monostate) { return false; }
  bool operator()(bool& value)
  {
    ImGui::SetNextItemWidth(-1);
    return ImGui::Checkbox("##_bool", &value);
  }
  bool operator()(float& value)
  {
    ImGui::SetNextItemWidth(-1);
    return ImGui::InputFloat("##_float", &value);
  }
  bool operator()(std::u8string& value)
  {
    ImGui::SetNextItemWidth(-1);
    return InputU8Text("##_string", &value);
  }
  bool operator()(gltfjson::tree::ArrayValue& value)
  {
    ImGui::TextUnformatted((const char*)ToStr(value).c_str());
    return false;
  }
  bool operator()(gltfjson::tree::ObjectValue& value)
  {
    ImGui::TextUnformatted((const char*)ToStr(value).c_str());
    return false;
  }
};

ShowGuiFunc
JsonValue::EditorOrDefault() const
{
  if (Editor) {
    return Editor;
  }

  return [](const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& json) {
    assert(json);
    return std::visit(NodeTypeEditVisitor{ root, bin }, json->Var);
  };
}
