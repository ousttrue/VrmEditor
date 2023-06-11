#include "asset_view.h"
#include "app.h"
#include "fs_util.h"
#include "gui.h"
#include <algorithm>
#include <array>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <plog/Log.h>

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
};

struct Asset
{
  std::filesystem::path Path;
  std::string Type;
  std::u8string Label;
  ImVec4 Color;
  std::list<std::shared_ptr<Asset>> Children;

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
        Label = std::u8string(u8"🟩") + path.stem().u8string();
      }
    }
  }

  void Add(const std::shared_ptr<Asset>& asset)
  {
    if (asset->Path.parent_path() == Path) {
      Children.push_back(asset);
      return;
    }

    for (auto& child : Children) {
      if (std::filesystem::is_directory(child->Path) &&
          IsAncestorOf(child->Path, asset->Path)) {
        child->Add(asset);
        return;
      }
    }

    for (auto current = asset->Path.parent_path();;
         current = current.parent_path()) {
      if (current.parent_path() == Path) {
        // found
        auto folder = std::make_shared<Asset>(current);
        Children.push_back(folder);
        Children.back()->Add(asset);
        return;
      }

      if (current == current.parent_path()) {
        break;
      }
    }
    PLOG_ERROR << asset->Path.string() << " not found";
  }

  void ShowGui()
  {
    ImGui::TableNextRow();

    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;
    if (Children.empty()) {
      node_flags |=
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    }

    // 0
    ImGui::TableNextColumn();
    auto node_open = ImGui::TreeNodeEx(
      (void*)this, node_flags, "%s", (const char*)Label.c_str());
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        app::TaskLoadPath(Path);
      }
    }

    // 1
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(Type.c_str());

    if (Children.size() && node_open) {
      for (auto& child : Children) {
        child->ShowGui();
      }

      ImGui::TreePop();
    }
  }

  bool operator<(const Asset& b) const noexcept { return Path < b.Path; }
};

struct AssetViewImpl
{
  std::string Name;
  std::shared_ptr<Asset> Root;

  AssetViewImpl(std::string_view name, const std::filesystem::path& path)
    : Name(name)
    , Root(new Asset(path))
  {
  }

  void Reload()
  {
    Root->Children.clear();
    if (!std::filesystem::is_directory(Root->Path)) {
      return;
    }

    for (auto e : std::filesystem::recursive_directory_iterator(Root->Path)) {
      if (auto asset = std::make_shared<Asset>(e.path())) {
        Root->Add(asset);
      }
    }

    // std::sort(Assets.begin(), Assets.end());
  }

  void ShowGui()
  {
    if (ImGui::Button("📁Open")) {
      PLOG_INFO << "open: " << Root->Path.string().c_str();
      shell_open(Root->Path);
    }
    ImGui::SameLine();
    if (ImGui::Button("🔄Reload")) {
      Reload();
    }
    ImGui::Separator();

    std::array<const char*, 2> cols = {
      "Name",
      "Ext",
    };

    if (grapho::imgui::BeginTableColumns("##assetdir", cols)) {
      // tree
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,
                          Gui::Instance().Indent());

      for (auto& child : Root->Children) {
        child->ShowGui();
      }

      ImGui::PopStyleVar();

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
