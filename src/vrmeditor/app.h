#pragma once
#include <filesystem>
#include <span>

namespace app {

inline const auto DOCKNAME_JSON = "🎁Json";
inline const auto DOCKNAME_VIEW = "🎁AssetView";

void
TaskLoadModel(const std::filesystem::path& path);
void
TaskLoadPath(const std::filesystem::path& path);
void
TaskLoadHdr(const std::filesystem::path& hdr);
void
LoadGltfString(const std::string& json);

void
SetShaderDir(const std::filesystem::path& path);

void
Run(std::span<const char*> args);

bool
WriteScene(const std::filesystem::path& path);

bool
AddAssetDir(std::string_view name, const std::filesystem::path& path);

std::string
CopyVrmPoseText();

} // namespace
