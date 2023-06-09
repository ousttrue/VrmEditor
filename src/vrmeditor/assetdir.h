#pragma once
#include "docks/dockspace.h"
#include <filesystem>
#include <functional>
#include <imgui.h>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

using AssetEnter =
  std::function<bool(const std::filesystem::path& path, uint64_t id)>;
using AssetLeave = std::function<void()>;

using LoadFunc = std::function<void(const std::filesystem::path& path)>;

struct Asset
{
  std::filesystem::path Path;
  std::u8string Type;
  std::u8string Label;
  ImVec4 Color;

  static std::optional<Asset> FromPath(const std::filesystem::path& path);
  bool Show() const;

  bool operator<(const Asset& b) const noexcept { return Path < b.Path; }
};

struct AssetDir
{
  std::string Name;
  std::filesystem::path Dir;
  std::vector<Asset> Assets;

  AssetDir(std::string_view name, const std::filesystem::path& path)
    : Name(name)
    , Dir(path)
  {
  }

  void ShowGui(const LoadFunc& callback);
  void Update();
  grapho::imgui::Dock CreateDock(const LoadFunc& callback);
};
