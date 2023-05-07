#include "assetdir.h"
#include "app.h"
#include "fs_util.h"
#include <algorithm>
#include <imgui.h>

std::optional<Asset>
Asset::FromPath(const std::filesystem::path& path)
{
  auto extension = path.extension().string();
  std::transform(
    extension.begin(), extension.end(), extension.begin(), tolower);

  if (extension == ".gltf") {
    return Asset{
      .Path = path,
      .Label = std::u8string(u8" ") + path.filename().u8string(),
      .Color = (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.6f),
    };
  }
  if (extension == ".glb") {
    return Asset{
      .Path = path,
      .Label = std::u8string(u8"󰕣 ") + path.filename().u8string(),
      .Color = (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.6f),
    };
  }
  if (extension == ".vrm") {
    return Asset{
      .Path = path,
      .Label = std::u8string(u8"󰋦 ") + path.filename().u8string(),
      .Color = (ImVec4)ImColor::HSV(4 / 7.0f, 0.8f, 0.6f),
    };
  }
  if (extension == ".vrma") {
    return Asset{
      .Path = path,
      .Label = std::u8string(u8"󰋦 󰑮 ") + path.filename().u8string(),
      .Color = (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.6f),
    };
  }
  if (extension == ".fbx") {
    return Asset{
      .Path = path,
      .Label = std::u8string(u8"󰕠 ") + path.filename().u8string(),
      .Color = (ImVec4)ImColor::HSV(6 / 7.0f, 0.8f, 0.6f),
    };
  }
  if (extension == ".bvh") {
    return Asset{
      .Path = path,
      .Label = std::u8string(u8"󰑮 ") + path.filename().u8string(),
      .Color = (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.6f),
    };
  }

  return {};
}

bool
Asset::Show(float width) const
{
  ImGui::PushStyleColor(ImGuiCol_Button, Color);
  auto result = ImGui::Button((const char*)Label.c_str(), { width, 0 });
  ImGui::PopStyleColor();

  return result;
}

void
AssetDir::Update()
{
  Assets.clear();
  if (!std::filesystem::is_directory(Dir)) {
    return;
  }

  for (auto e : std::filesystem::recursive_directory_iterator(Dir)) {
    if (auto asset = Asset::FromPath(e.path())) {
      Assets.push_back(*asset);
    }
  }

  std::sort(Assets.begin(), Assets.end());
}

grapho::imgui::Dock
AssetDir::CreateDock(const LoadFunc& callback)
{
  return {
    std::string("[") + Name + "]",
    [this, callback]() {
      if (ImGui::Button(" Open")) {
        App::Instance().Log(LogLevel::Info)
          << "open: " << (const char*)Dir.u8string().c_str();
        shell_open(Dir);
      }
      ImGui::SameLine();
      if (ImGui::Button("󰑓 Reload")) {
        Update();
      }
      ImGui::Separator();

      // ImGui::PushItemWidth(ImGui::GetWindowWidth());
      ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0, .5f });
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
      for (auto& asset : Assets) {
        if (asset.Show(ImGui::GetContentRegionAvail().x)) {
          callback(asset.Path);
        }
      }
      ImGui::PopStyleColor();
      ImGui::PopStyleVar();
    },
  };
}
