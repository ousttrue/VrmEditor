#pragma once
#include "showgui.h"
#include <gltfjson.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <grapho/imgui/widgets.h>
#include <string>
#include <vrm/runtime_node.h>

struct NodeItem
{
  std::string m_name;
  std::string m_mesh_skin;
  std::string m_extensions;
  std::string m_humanbone;
  NodeItem(const gltfjson::Root& root, int i)
  {
    auto node = root.Nodes[i];
    m_name = (const char*)node.Name().c_str();
    if (node.Mesh()) {
      m_mesh_skin += "󰕣 ";
    }
    if (node.Skin()) {
      m_mesh_skin += "󰭓 ";
    }

    if (auto extensions = node.Extensions()) {
      m_extensions += " ";
    }

    auto bone = gltfjson::vrm1::GetHumanBoneName(root, i);
    if (bone.size()) {
      m_humanbone = gltfjson::from_u8(bone);
    }
    bone = gltfjson::vrm0::GetHumanBoneName(root, i);
    if (bone.size()) {
      m_humanbone = gltfjson::from_u8(bone);
    }
  }
  void ShowGui()
  {
    // name
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(m_name.c_str());

    // mesh
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(m_mesh_skin.c_str());

    // extension
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(m_extensions.c_str());

    // humanoid
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(m_humanbone.c_str());
  }
};

inline ShowGuiFunc
JsonGuiNodes(std::u8string_view jsonpath)
{
  return [items = std::vector<NodeItem>()](
           const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           const gltfjson::tree::NodePtr& node) mutable {
    if (items.empty()) {
      auto& nodes = root.Nodes;
      for (size_t i = 0; i < nodes.size(); ++i) {
        items.push_back(NodeItem(root, i));
      }
    }
    std::array<const char*, 5> cols = {
      "index", "name", "mesh/skin", "extensions", "human bone",
    };
    if (grapho::imgui::BeginTableColumns("##accessors", cols)) {
      for (int i = 0; i < items.size(); ++i) {
        auto& item = items[i];
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d", i);

        item.ShowGui();
      }
      ImGui::EndTable();
    }
    return false;
  };
}
