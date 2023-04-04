#include "assetdir.h"
#include "app.h"
#include "fs_util.h"
#include <imgui.h>

AssetDir::AssetDir(std::string_view name, const std::filesystem::path& path)
  : name_(name)
{
  root_ = path;
}

void
AssetDir::Traverse(const AssetEnter& enter,
                   const AssetLeave& leave,
                   const std::filesystem::path& path)
{

  if (path.empty()) {
    // root
    // traverse(enter, leave, root_);
    for (auto e : std::filesystem::directory_iterator(root_)) {
      Traverse(enter, leave, e);
    }
    return;
  }

  uint64_t id;
  auto key = path.u8string();
  for (auto& c : key) {
    if (c == '\\') {
      c = '/';
    }
  }
  auto found = idMap_.find(key);
  if (found != idMap_.end()) {
    id = found->second;
  } else {
    id = nextId_++;
    idMap_.insert(std::make_pair(key, id));
  }

  if (enter(path, id)) {
    if (std::filesystem::is_directory(path)) {
      for (auto e : std::filesystem::directory_iterator(path)) {
        Traverse(enter, leave, e);
      }
    }
    leave();
  }
}

Dock
AssetDir::CreateDock(const LoadFunc& callback)
{

  auto enter = [callback](const std::filesystem::path& path, uint64_t id) {
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;

#if _WIN32
    auto name = WideToMb(CP_OEMCP, path.filename().c_str());
#else
    auto name = path.filename();
#endif

    if (std::filesystem::is_directory(path)) {
      ImGuiTreeNodeFlags node_flags = base_flags;
      return ImGui::TreeNodeEx(
        (void*)(intptr_t)id, node_flags, "%s", name.c_str());
    } else {
      if (ImGui::Button(name.c_str())) {
        callback(path);
      }
      return false;
    }
  };
  auto leave = []() { ImGui::TreePop(); };
  return Dock{
    std::string("[") + Name() + "]",
    [this, enter, leave]() {
      if (ImGui::Button(" Open")) {
        App::Instance().Log(LogLevel::Info)
          << "open: " << (const char*)root_.u8string().c_str();
        shell_open(root_);
      }
      ImGui::SameLine();
      if (ImGui::Button("󰑓 Reload")) {
      }
      ImGui::Separator();
      Traverse(enter, leave);
    },
  };
}
