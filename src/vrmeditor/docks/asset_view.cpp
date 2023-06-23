#include "asset_view.h"
#include "app.h"
#include "fs_util.h"
#include "gui.h"
#include <algorithm>
#include <array>
#include <asio.hpp>
#include <asio/use_awaitable.hpp>
#include <asio_task.h>
#include <functional>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <memory>
#include <plog/Log.h>

static const char* s_supportedTypes[]{
  ".glb", ".gltf", ".vrm", ".bvh", ".fbx", ".obj", ".hdr", ".vrma",
};

static bool
IsAncestorOf(const std::filesystem::path& lhs, const std::filesystem::path& rhs)
{
  if (!std::filesystem::is_directory(lhs)) {
    return false;
  }
  for (auto current = rhs.parent_path();; current = current.parent_path()) {
    if (current == lhs) {
      return true;
    }
    if (current == current.parent_path()) {
      break;
    }
  }
  return false;
}

std::unordered_map<std::string, std::u8string> g_iconMap = {
  // image
  { ".png", u8"🖼" },
  { ".jpg", u8"🖼" },
  { ".gif", u8"🖼" },
  // model
  { ".gltf", u8"📦" },
  { ".glb", u8"📦" },
  { ".vrm", u8"📦" },
  { ".fbx", u8"📦" },
  // animation
  { ".bvh", u8"🏃" },
  { ".vrma", u8"🏃" },
  // text
  { ".txt", u8"📄" },
  { ".md", u8"📄" },
  // bin
  { ".bin", u8"🫙" },
};

struct Asset
{
  std::filesystem::path Path;
  std::string Type;
  std::u8string Label;
  ImVec4 Color;
  // std::list<std::shared_ptr<Asset>> Children;

  Asset(const std::filesystem::path& path)
    : Path(path)
  {
    Type = path.extension().string();
    std::transform(Type.cbegin(), Type.cend(), Type.begin(), tolower);
    if (std::filesystem::is_directory(path)) {
      Label = std::u8string(u8"📁") + path.stem().u8string();
    } else {
      auto found = g_iconMap.find(Type);
      if (found != g_iconMap.end()) {
        Label = found->second + path.stem().u8string();
      } else {
        Label = std::u8string(u8"⬜") + path.stem().u8string();
      }
    }
  }

  void ShowGui()
  {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Selectable((const char*)Label.c_str());
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        app::TaskLoadPath(Path);
      }
    }

    // 1
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(Type.c_str());
  }

  bool operator<(const Asset& b) const noexcept { return Path < b.Path; }
};

struct AssetViewImpl
{
  std::string Name;
  std::filesystem::path Path;
  std::list<std::shared_ptr<Asset>> Assets;
  bool m_loading = false;

  AssetViewImpl(std::string_view name, const std::filesystem::path& path)
    : Name(name)
    , Path(path)
  {
  }

  void ShowGui()
  {
    if (ImGui::Button("📁Explorer")) {
      PLOG_INFO << "open: " << Path.string().c_str();
      shell_open(Path);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_loading);
    if (ImGui::Button("🔄ReloadAsync")) {
      ReloadAsync();
    }
    ImGui::EndDisabled();
    ImGui::Separator();

    std::array<const char*, 2> cols = {
      "Name",
      "Ext",
    };

    if (grapho::imgui::BeginTableColumns("##assetdir", cols)) {
      // tree
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,
                          Gui::Instance().Indent());

      // for (auto& child : Root->Children) {
      //   child->ShowGui();
      // }
      for (auto& item : Assets) {
        item->ShowGui();
      }

      ImGui::PopStyleVar();

      ImGui::EndTable();
    }
  }

  // asio::awaitable<void> TraverseAsync(const std::filesystem::path& dir)
  // {
  // }

  asio::awaitable<void> ReloadAsync()
  {
    Assets.clear();
    m_loading = true;

    // co_await TraverseAsync(Path);
    for (auto e : std::filesystem::recursive_directory_iterator(Path)) {
      for (auto& ext : s_supportedTypes) {
        if (e.path().extension().string() == ext) {
          auto asset = std::make_shared<Asset>(e.path());
          if (ext == ".vrm") {
            // load thumbnail
          }
          Assets.push_back(asset);
          co_await asio::this_coro::executor;
          break;
        }
      }
    }

    m_loading = false;
    co_return;
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
AssetView::ShowGui()
{
  m_impl->ShowGui();
}

// void
// AssetView::Reload()
// {
//   m_impl->Reload();
// }

void
AssetView::ReloadAsync()
{
  asio::co_spawn(AsioTask::Instance().Executor(),
                 std::bind(&AssetViewImpl::ReloadAsync, m_impl),
                 asio::detached);
}
