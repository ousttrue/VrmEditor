#pragma once
#include "json_gui.h"
#include <vrm/node.h>

inline ShowGui
JsonGuiNodeList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                std::string_view jsonpath)
{
  return [scene]() {
    auto& nodes = scene->m_gltf.Nodes;
    std::array<const char*, 9> cols = {
      "index", "name", "T", "R", "S", "children", "mesh", "skin", "extensions",
    };
    std::string no_name;
    if (JsonGuiTable("##nodes", cols)) {
      for (int i = 0; i < nodes.Size(); ++i) {
        auto& node = nodes[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", node.Name.c_str());
        if (auto _t = node.Translation) {
          auto t = *_t;
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("%f, %f, %f", t[0], t[1], t[2]);
        }
        // ImGui::TableSetColumnIndex(3);
        // ImGui::Text("%f, %f, %f, %f",
        //             node->Transform.Rotation.x,
        //             node->Transform.Rotation.y,
        //             node->Transform.Rotation.z,
        //             node->Transform.Rotation.w);
        // ImGui::TableSetColumnIndex(4);
        // ImGui::Text(
        //   "%f, %f, %f", node->Scaling.x, node->Scaling.y, node->Scaling.z);
        {
          std::stringstream ss;
          int j = 0;
          for (int child : node.Children) {
            if (j++) {
              ss << ',';
            }
            ss << child;
          }
          ImGui::TableSetColumnIndex(5);
          ImGui::Text("%s", ss.str().c_str());
        }
        if (auto mesh = node.Mesh) {
          ImGui::TableSetColumnIndex(6);
          ImGui::Text("%d", *mesh);
        }
        if (auto skin = node.Skin) {
          ImGui::TableSetColumnIndex(7);
          ImGui::Text("%d", *skin);
        }
        // if (libvrm::gltf::has(node, "extensions")) {
        //   std::stringstream ss;
        //   int j = 0;
        //   // for (auto kv : node.at("extensions").items()) {
        //   //   ss << ',';
        //   //   ss << kv.key();
        //   // }
        //   ImGui::TableSetColumnIndex(8);
        //   ImGui::Text("%s", ss.str().c_str());
        // }
      }
      ImGui::EndTable();
    }
  };
}
