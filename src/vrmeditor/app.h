#pragma once
#include <filesystem>

namespace app {

void
SetShaderDir(const std::filesystem::path& path);
void
SetShaderChunkDir(const std::filesystem::path& path);

void
LoadModel(const std::filesystem::path& path);
void
LoadLua(const std::filesystem::path& path);
void
LoadPath(const std::filesystem::path& path);

void
ProjectMode();

void
Run();

bool
WriteScene(const std::filesystem::path& path);

} // namespace
