#include "asset_view.h"
#include "app.h"
#include "fs_util.h"
#include <algorithm>
#include <array>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <plog/Log.h>

// using AssetEnter =
//   std::function<bool(const std::filesystem::path& path, uint64_t id)>;
// using AssetLeave = std::function<void()>;
//
// using LoadFunc = std::function<void(const std::filesystem::path& path)>;
//
//
struct Asset
{
  std::filesystem::path Path;
  std::u8string Type;
  std::u8string Label;
  ImVec4 Color;

  static std::optional<Asset> FromPath(const std::filesystem::path& path)
  {
    auto extension = path.extension().string();
    std::transform(
      extension.begin(), extension.end(), extension.begin(), tolower);

    if (extension == ".gltf") {
      return Asset{
        .Path = path,
        .Type = std::u8string(u8"GLTF"),
        .Label = path.filename().u8string(),
        .Color = (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.6f),
      };
    }
    if (extension == ".glb") {
      return Asset{
        .Path = path,
        .Type = std::u8string(u8"GLB"),
        .Label = path.filename().u8string(),
        .Color = (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.6f),
      };
    }
    if (extension == ".vrm") {
      return Asset{
        .Path = path,
        .Type = std::u8string(u8"VRM"),
        .Label = path.filename().u8string(),
        .Color = (ImVec4)ImColor::HSV(4 / 7.0f, 0.8f, 0.6f),
      };
    }
    if (extension == ".vrma") {
      return Asset{
        .Path = path,
        .Type = std::u8string(u8"VRMA"),
        .Label = path.filename().u8string(),
        .Color = (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.6f),
      };
    }
    if (extension == ".fbx") {
      return Asset{
        .Path = path,
        .Type = std::u8string(u8"FBX"),
        .Label = path.filename().u8string(),
        .Color = (ImVec4)ImColor::HSV(6 / 7.0f, 0.8f, 0.6f),
      };
    }
    if (extension == ".bvh") {
      return Asset{
        .Path = path,
        .Type = std::u8string(u8"BVH"),
        .Label = path.filename().u8string(),
        .Color = (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.6f),
      };
    }

    return {};
  }

  bool Show() const
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted((const char*)Type.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(
      (const char*)Path.parent_path().filename().string().c_str());
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Button, Color);
    auto result = ImGui::Button((const char*)Label.c_str(), { -1, 0 });
    ImGui::PopStyleColor();
    return result;
  }

  bool operator<(const Asset& b) const noexcept { return Path < b.Path; }
};

struct AssetViewImpl
{
  // grapho::imgui::Dock CreateDock(const LoadFunc& callback);
  std::string Name;
  std::filesystem::path Dir;
  std::vector<Asset> Assets;

  AssetViewImpl(std::string_view name, const std::filesystem::path& path)
    : Name(name)
    , Dir(path)
  {
  }

  void Reload()
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

  void ShowGui()
  {
    if (ImGui::Button("📁Open")) {
      PLOG_INFO << "open: " << (const char*)Dir.u8string().c_str();
      shell_open(Dir);
    }
    ImGui::SameLine();
    if (ImGui::Button("🔄Reload")) {
      Reload();
    }
    ImGui::Separator();

    std::array<const char*, 3> cols = {
      "Type",
      "Dir",
      "Name",
    };
    if (grapho::imgui::BeginTableColumns("##assetdir", cols)) {
      for (auto& asset : Assets) {
        if (asset.Show()) {
          app::TaskLoadPath(asset.Path);
        }
      }
      ImGui::EndTable();
    }
  }
};

AssetView::AssetView(std::string_view name, const std::filesystem::path& path)
  : m_impl(new AssetViewImpl(name, path))
{
}

AssetView::~AssetView()
{
  delete m_impl;
}

void
AssetView::Reload()
{
  m_impl->Reload();
}

void
AssetView::ShowGui()
{
  m_impl->ShowGui();
}

// grapho::imgui::Dock
// AssetView::CreateDock(const LoadFunc& callback)
// {
//   return {
//     std::string("🎁") + Name,
//     [this, callback]() { this->ShowGui(callback); },
//   };
// }
