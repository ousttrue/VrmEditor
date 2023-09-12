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
  if (auto object =
        std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(item)) {
    return ToStr(object->Value);
  } else if (auto array =
               std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(item)) {
    return ToStr(array->Value);
  } else {
    std::stringstream ss;
    ss << item;
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
  bool operator()(bool& value) {}
  bool operator()(float& value) {}
  bool operator()(std::u8string& value) {}
  bool operator()(gltfjson::tree::ArrayValue& value) {}
  bool operator()(gltfjson::tree::ObjectValue& value) {}
};

static bool
visit(const gltfjson::tree::NodePtr& json)
{
  if (std::dynamic_pointer_cast<gltfjson::tree::NullNode>(json)) {
  } else if (auto b =
               std::dynamic_pointer_cast<gltfjson::tree::BoolNode>(json)) {
    ImGui::SetNextItemWidth(-1);
    return ImGui::Checkbox("##_bool", &b->Value);
  } else if (auto n =
               std::dynamic_pointer_cast<gltfjson::tree::NumberNode>(json)) {
    ImGui::SetNextItemWidth(-1);
    return ImGui::InputFloat("##_float", &n->Value);
  } else if (auto s =
               std::dynamic_pointer_cast<gltfjson::tree::StringNode>(json)) {
    ImGui::SetNextItemWidth(-1);
    return InputU8Text("##_string", &s->Value);
  } else if (auto a =
               std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(json)) {
    ImGui::TextUnformatted((const char*)ToStr(a->Value).c_str());
    return false;
  } else if (auto o =
               std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(json)) {
    ImGui::TextUnformatted((const char*)ToStr(o->Value).c_str());
    return false;
  }

  assert(false);
  return false;
}

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
    // return std::visit(NodeTypeEditVisitor{ root, bin }, json->Var);
    return visit(json);
  };
}
