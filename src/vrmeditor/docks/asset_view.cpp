#include <GL/glew.h>

#include "app.h"
#include "asset_view.h"
#include "fs_util.h"
#include "grapho/gl3/texture.h"
#include "gui.h"
#include <algorithm>
#include <array>
#include <asio.hpp>
#include <asio/use_awaitable.hpp>
#include <asio_task.h>
#include <functional>
#include <glr/gl3renderer.h>
#include <gltfjson.h>
#include <gltfjson/glb.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <memory>
#include <plog/Log.h>
#include <vrm/fileutil.h>
#include <vrm/image.h>

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

static std::optional<std::uint32_t>
GetThumbnailImageid(const gltfjson::Root& root)
{
  if (auto VRMC_vrm = root.GetExtension<gltfjson::vrm1::VRMC_vrm>()) {
    if (auto meta = VRMC_vrm->Meta()) {
      return meta->ThumbnailImageId();
    }
  } else if (auto VRM = root.GetExtension<gltfjson::vrm0::VRM>()) {
    if (auto meta = VRM->Meta()) {
      if (auto texId = meta->TextureId()) {
        auto texture = root.Textures[*texId];
        return texture.SourceId();
      }
    }
  }
  return {};
}

enum class LoadStatus
{
  Yet,
  Loading,
  Done,
};

struct Asset
{
  std::filesystem::path Path;
  std::u8string Label;
  ImVec4 Color;
  std::vector<std::u8string> Tags;
  std::vector<uint8_t> ImageBytes;
  std::shared_ptr<grapho::gl3::Texture> Texture;

  LoadStatus m_loaded = {};

  Asset(const std::filesystem::path& path)
    : Path(path)
  {
    auto type = path.extension().string();
    std::transform(type.cbegin(), type.cend(), type.begin(), tolower);

    std::u8string icon(u8"⬜");
    if (std::filesystem::is_directory(path)) {
      icon = u8"📁";
    } else {
      auto found = g_iconMap.find(type);
      if (found != g_iconMap.end()) {
        icon = found->second;
      }
    }

    Label = icon + path.filename().u8string();
  }

  static std::shared_ptr<Asset> Load(const std::filesystem::path& path)
  {
    auto work = std::make_shared<Asset>("thread");
    auto ext = path.extension().string();
    if (ext == ".gltf") {
      auto bytes = libvrm::ReadAllBytes(path);
      gltfjson::tree::Parser parser(bytes);
      if (auto result = parser.Parse()) {
        auto gltf = gltfjson::Root(result);
        if (auto used = gltf.ExtensionsUsed()) {
          if (auto array =
                std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(used)) {
            for (auto& ex : array->Value) {
              work->Tags.push_back(ex->U8String());
            }
          }
        }
      }

    } else if (ext == ".glb" || ext == ".vrm" || ext == ".vrma" ||
               ext == ".vci") {

      // load thumbnail
      auto bytes = libvrm::ReadAllBytes(path);
      if (auto glb = gltfjson::Glb::Parse(bytes)) {
        gltfjson::tree::Parser parser(glb->JsonChunk);
        if (auto result = parser.Parse()) {
          auto gltf = gltfjson::Root(result);
          if (auto used = gltf.ExtensionsUsed()) {
            if (auto array =
                  std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(used)) {
              for (auto& ex : array->Value) {
                work->Tags.push_back(ex->U8String());
              }
            }
          }
          if (auto imageId = GetThumbnailImageid(gltf)) {
            gltfjson::Bin bin{ std::make_shared<gltfjson::Directory>(
                                 path.parent_path()),
                               glb->BinChunk };
            auto image = gltf.Images[*imageId];
            if (auto viewId = image.BufferViewId()) {
              if (auto span = bin.GetBufferViewBytes(gltf, *viewId)) {
                work->ImageBytes.assign(span->begin(), span->end());
              }
            }
          }
        }
      }
    }
    if (work->ImageBytes.empty()) {

      auto ss = path.parent_path().parent_path() / "screenshot";
      if (std::filesystem::is_directory(ss)) {
        for (auto f : std::filesystem::directory_iterator(ss)) {
          if (f.path().stem().string() == "screenshot") {
            work->ImageBytes = libvrm::ReadAllBytes(f);
            break;
          }
        }
      }
    }

    return work;
  }

  // asio::awaitable<void> LoadAsync()
  // {
  //   auto work =
  //     co_await AsioTask::ThreadTask<std::shared_ptr<Asset>>::AsyncThredTask(
  //       [path = Path]() { return Load(path); }, asio::use_awaitable);
  //   ImageBytes = work->ImageBytes;
  //   Tags.assign(work->Tags.begin(), work->Tags.end());
  //   m_loaded = LoadStatus::Done;
  // }

 //  void ShowGui(float w)
 //  {
 //    if (m_loaded == LoadStatus::Yet) {
 //      m_loaded = LoadStatus::Loading;
 //      asio::co_spawn(AsioTask::Instance().Executor(),
 //                     std::bind(&Asset::LoadAsync, this),
 //                     asio::detached);
 //    }
// 
 //    ImGui::PushID(this);
 //    if (ImageBytes.size()) {
 //      if (!Texture) {
 //        libvrm::Image image("thumb");
 //        if (image.Load(ImageBytes)) {
 //          Texture = glr::CreateTexture(image);
 //        }
 //      }
 //    }
 //    if (Texture) {
 //      ImGui::Image((ImTextureID)(intptr_t)Texture->Handle(), { 100, 100 });
 //    } else {
 //      ImGui::Image(0, { 100, 100 });
 //    }
 //    ImGui::SameLine();
// 
 //    ImGui::BeginGroup();
 //    ImGui::SetNextItemWidth(-1);
 //    if (ImGui::Button((const char*)Label.c_str(), { w, 0 })) {
 //      app::TaskLoadPath(Path);
 //    }
// 
 //    if (Tags.size()) {
 //      for (int i = 0; i < Tags.size(); ++i) {
 //        if (i) {
 //          ImGui::SameLine();
 //        }
 //        ImGui::SmallButton((const char*)Tags[i].c_str());
 //      }
 //    } else {
 //      ImGui::NewLine();
 //    }
 //    ImGui::EndGroup();
// 
 //    ImGui::PopID();
 //  }

  bool operator<(const Asset& b) const noexcept { return Path < b.Path; }
};

struct AssetViewImpl
{
  std::string Name;
  std::filesystem::path Path;
  std::vector<std::shared_ptr<Asset>> Assets;
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

    auto size = ImGui::GetContentRegionAvail();
    // bool first = true;
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0, 0.5f });
#if false
    for (auto& item : Assets) {
      item->ShowGui(size.x);
    }
#else
    ImGuiListClipper clipper;
    clipper.Begin(Assets.size(), 100);
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        // Assets[i]->ShowGui(size.x);
      }
    }
#endif

    ImGui::PopStyleVar();
  }

  static std::shared_ptr<Asset> Load(const std::filesystem::path& path)
  {
    for (auto& ext : s_supportedTypes) {
      if (path.extension().string() != ext) {
        continue;
      }
      auto asset = std::make_shared<Asset>(path);
      return asset;
    }

    return {};
  }

  asio::awaitable<void> _ReloadAsync()
  {
    Assets.clear();
    m_loading = true;

    auto executor = co_await asio::this_coro::executor;

    for (auto e : std::filesystem::recursive_directory_iterator(Path)) {

      if (auto asset = Load(e.path())) {
        Assets.push_back(asset);
        co_await asio::steady_timer(executor, asio::chrono::seconds(0))
          .async_wait(asio::use_awaitable);
      }
    }

    m_loading = false;
    co_return;
  }

  void ReloadAsync()
  {
    asio::co_spawn(AsioTask::Instance().Executor(),
                   std::bind(&AssetViewImpl::_ReloadAsync, this),
                   asio::detached);
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

void
AssetView::ReloadAsync()
{
  m_impl->ReloadAsync();
}
