#pragma once
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

using AssetEnter =
    std::function<bool(const std::filesystem::path &path, uint64_t id)>;
using AssetLeave = std::function<void()>;

class AssetDir {
  std::string name_;
  std::filesystem::path root_;

  std::unordered_map<std::filesystem::path, uint64_t> idMap_;
  uint64_t nextId_ = 1;

public:
  AssetDir(std::string_view name, std::string_view path);
  const std::string &name() const { return name_; }
  void traverse(const AssetEnter &enter, const AssetLeave &leave,
                const std::filesystem::path &path = {});
};
