#pragma once
#include "docks/gui.h"
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

using AssetEnter =
  std::function<bool(const std::filesystem::path& path, uint64_t id)>;
using AssetLeave = std::function<void()>;

using LoadFunc = std::function<void(const std::filesystem::path& path)>;
class AssetDir
{
  std::string name_;
  std::filesystem::path root_;

  std::unordered_map<std::u8string, uint64_t> idMap_;
  uint64_t nextId_ = 1;

public:
  AssetDir(std::string_view name, const std::filesystem::path& path);
  const std::string& Name() const { return name_; }
  void Traverse(const AssetEnter& enter,
                const AssetLeave& leave,
                const std::filesystem::path& path = {});

  Dock CreateDock(const LoadFunc& callback);
};
