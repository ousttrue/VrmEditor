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
#include <gltfjson.h>
#include <gltfjson/glb.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <memory>
#include <plog/Log.h>
#include <vrm/fileutil.h>

static std::string s_supportedTypes[]{
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
  std::vector<std::u8string> Tags;
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
    ImGui::PushID(this);

    ImGui::SetNextItemWidth(-1);
    if (ImGui::Button((const char*)Label.c_str())) {
      app::TaskLoadPath(Path);
    }

    ImGui::SmallButton(Type.c_str());
    for (int i = 0; i < Tags.size(); ++i) {
      ImGui::SameLine();
      ImGui::SmallButton((const char*)Tags[i].c_str());
    }

    ImGui::PopID();
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

    // std::array<const char*, 2> cols = {
    //   "Name",
    //   "Ext",
    // };
    //
    // if (grapho::imgui::BeginTableColumns("##assetdir", cols))
    {
      // tree
      // ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,
      //                     Gui::Instance().Indent());

      bool first = true;
      for (auto& item : Assets) {
        if (first) {
          first = false;
        } else {
          ImGui::Separator();
        }
        item->ShowGui();
      }

      // ImGui::PopStyleVar();

      // ImGui::EndTable();
    }
  }

  // asio::awaitable<void> TraverseAsync(const std::filesystem::path& dir)
  // {
  // }

  bool Load(const std::filesystem::path& path)
  {
    for (auto& ext : s_supportedTypes) {
      if (path.extension().string() != ext) {
        continue;
      }
      auto asset = std::make_shared<Asset>(path);
      if (ext == ".gltf") {
        auto bytes = libvrm::ReadAllBytes(path);
        gltfjson::tree::Parser parser(bytes);
        if (auto result = parser.Parse()) {
          auto gltf = gltfjson::Root(result);
          if (auto used = gltf.ExtensionsUsed()) {
            if (auto array = used->Array()) {
              for (auto& ex : *array) {
                asset->Tags.push_back(ex->U8String());
              }
            }
          }
        }
      } else if (ext == ".glb" || ext == ".vrm") {
        // load thumbnail
        auto bytes = libvrm::ReadAllBytes(path);
        if (auto glb = gltfjson::Glb::Parse(bytes)) {
          gltfjson::tree::Parser parser(glb->JsonChunk);
          if (auto result = parser.Parse()) {
            auto gltf = gltfjson::Root(result);
            if (auto used = gltf.ExtensionsUsed()) {
              if (auto array = used->Array()) {
                for (auto& ex : *array) {
                  asset->Tags.push_back(ex->U8String());
                }
              }
            }
          }
        }
      }
      Assets.push_back(asset);
      return true;
    }

    return false;
  }

  asio::awaitable<void> ReloadAsync()
  {
    Assets.clear();
    m_loading = true;

    auto executor = co_await asio::this_coro::executor;

    auto SUSPEND = [](auto executor) {
      return asio::steady_timer(executor, asio::chrono::seconds(0))
        .async_wait(asio::use_awaitable);
    };

    // co_await TraverseAsync(Path);
    for (auto e : std::filesystem::recursive_directory_iterator(Path)) {
      if (Load(e.path())) {
        co_await SUSPEND(executor);
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
