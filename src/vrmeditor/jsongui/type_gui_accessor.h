#pragma once

inline bool
ShowGuiTable(const char* title, std::span<const char*> cols)
{
  static ImGuiTableFlags flags =
    ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody |
    ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
    ImGuiTableFlags_SizingFixedFit;

  if (ImGui::BeginTable(title, cols.size(), flags)) {
    for (auto col : cols) {
      ImGui::TableSetupColumn(col);
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();
    return true;
  }

  return false;
}

template<typename T>
inline void
ShowGuiAccessorScalar(std::span<const T> items)
{
  ImGui::Text("[%zu]", items.size());
  std::array<const char*, 2> cols = {
    "index",
    "value",
  };
  if (ShowGuiTable("##accessor_values", cols)) {
    ImGuiListClipper clipper;
    clipper.Begin(items.size());
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        auto& value = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%u", value);
      }
    }
    ImGui::EndTable();
  }
}

template<typename T>
inline void
ShowGuiAccessorInt4(std::span<const T> items)
{
  ImGui::Text("[%zu]", items.size());
  std::array<const char*, 5> cols = {
    "index", "x", "y", "z", "w",
  };
  if (ShowGuiTable("##accessor_values", cols)) {
    ImGuiListClipper clipper;
    clipper.Begin(items.size());
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        auto& value = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%u", value.X);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%u", value.Y);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%u", value.Z);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%u", value.W);
      }
    }
    ImGui::EndTable();
  }
}

template<typename T>
inline void
ShowGuiAccessorVec2(std::span<const T> items)
{
  ImGui::Text("[%zu]", items.size());
  std::array<const char*, 3> cols = {
    "index",
    "x",
    "y",
  };
  if (ShowGuiTable("##accessor_values", cols)) {
    ImGuiListClipper clipper;
    clipper.Begin(items.size());
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        auto& value = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%f", value.x);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%f", value.y);
      }
    }
    ImGui::EndTable();
  }
}

template<typename T>
inline void
ShowGuiAccessorVec3(std::span<const T> items)
{
  ImGui::Text("[%zu]", items.size());
  std::array<const char*, 4> cols = {
    "index",
    "x",
    "y",
    "z",
  };
  if (ShowGuiTable("##accessor_values", cols)) {
    ImGuiListClipper clipper;
    clipper.Begin(items.size());
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        auto& value = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%f", value.x);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%f", value.y);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%f", value.z);
      }
    }
    ImGui::EndTable();
  }
}

template<typename T>
inline void
ShowGuiAccessorVec4(std::span<const T> items)
{
  ImGui::Text("[%zu]", items.size());
  std::array<const char*, 5> cols = {
    "index", "x", "y", "z", "w",
  };
  if (ShowGuiTable("##accessor_values", cols)) {
    ImGuiListClipper clipper;
    clipper.Begin(items.size());
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        auto &value = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%f", value.x);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%f", value.y);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%f", value.z);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%f", value.w);
      }
    }
    ImGui::EndTable();
  }
}

inline void
ShowGuiAccessorMat4(std::span<const DirectX::XMFLOAT4X4> items)
{
  ImGui::Text("mat4[%zu]", items.size());
  std::array<const char*, 1 + 16> cols = {
    "index", "_11", "_12", "_13", "_14", "_21", "_22", "_23", "_24",
    "_31",   "_32", "_33", "_34", "_41", "_42", "_43", "_44",
  };
  if (ShowGuiTable("##accessor_values", cols)) {
    ImGuiListClipper clipper;
    clipper.Begin(items.size());
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        auto& value = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%f", value._11);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%f", value._12);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%f", value._13);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%f", value._14);
        ImGui::TableSetColumnIndex(5);
        ImGui::Text("%f", value._21);
        ImGui::TableSetColumnIndex(6);
        ImGui::Text("%f", value._22);
        ImGui::TableSetColumnIndex(7);
        ImGui::Text("%f", value._23);
        ImGui::TableSetColumnIndex(8);
        ImGui::Text("%f", value._24);
        ImGui::TableSetColumnIndex(9);
        ImGui::Text("%f", value._31);
        ImGui::TableSetColumnIndex(10);
        ImGui::Text("%f", value._32);
        ImGui::TableSetColumnIndex(11);
        ImGui::Text("%f", value._33);
        ImGui::TableSetColumnIndex(12);
        ImGui::Text("%f", value._34);
        ImGui::TableSetColumnIndex(13);
        ImGui::Text("%f", value._41);
        ImGui::TableSetColumnIndex(14);
        ImGui::Text("%f", value._42);
        ImGui::TableSetColumnIndex(15);
        ImGui::Text("%f", value._43);
        ImGui::TableSetColumnIndex(16);
        ImGui::Text("%f", value._44);
      }
    }
    ImGui::EndTable();
  }
}
