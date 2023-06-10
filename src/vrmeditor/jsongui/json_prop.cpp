#include "json_prop.h"
#include "json_widgets.h"
// #include "type_gui.h"

// std::u8string
// JsonProp::Value(const gltfjson::tree::NodePtr& item) const
// {
//   if (item) {
//     std::u8string name;
//     if (auto obj = item->Object()) {
//       if (auto p = item->Get(u8"name")) {
//         name = p->U8String();
//       }
//     }
//     std::stringstream ss;
//     if (name.size()) {
//       ss << "{" << gltfjson::from_u8(name) << "}";
//     } else {
//       ss << *item;
//     }
//     auto str = ss.str();
//     return std::u8string{ (const char8_t*)str.data(), str.size() };
//   } else {
//     return u8"";
//   }
// }

struct NodeTypeEditVisitor
{
  const gltfjson::Root& root;
  const gltfjson::Bin& bin;

  bool operator()(std::monostate) { return false; }
  bool operator()(bool& value)
  {
    //
    return ImGui::Checkbox("##_bool", &value);
  }
  bool operator()(float& value)
  {
    //
    return ImGui::InputFloat("##_float", &value);
  }
  bool operator()(std::u8string& value)
  {
    //
    return InputU8Text("##_string", &value);
  }
  bool operator()(gltfjson::tree::ArrayValue& value) { return false; }
  bool operator()(gltfjson::tree::ObjectValue& value) { return false; }
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
