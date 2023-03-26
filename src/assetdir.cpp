#include "assetdir.h"

AssetDir::AssetDir(std::string_view name, std::string_view path) : name_(name) {
  root_ = path;
}

void AssetDir::traverse(const AssetEnter &enter, const AssetLeave &leave,
                        const std::filesystem::path &path) {

  if (path.empty()) {
    // root
    // traverse(enter, leave, root_);
    for (auto e : std::filesystem::directory_iterator(root_)) {
      traverse(enter, leave, e);
    }
    return;
  }

  uint64_t id;
  auto found = idMap_.find(path);
  if (found != idMap_.end()) {
    id = found->second;
  } else {
    id = nextId_++;
    idMap_.insert(std::make_pair(path, id));
  }

  if (enter(path, id)) {
    if (std::filesystem::is_directory(path)) {
      for (auto e : std::filesystem::directory_iterator(path)) {
        traverse(enter, leave, e);
      }
    }
    leave();
  }
}
