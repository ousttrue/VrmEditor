#pragma once
#include <filesystem>
#include <functional>
#include <span>

class Gui;

namespace app {

using Task = std::function<void()>;

void
PostTask(const Task& task);

void
SetShaderDir(const std::filesystem::path& path);
void
SetShaderChunkDir(const std::filesystem::path& path);

void
LoadModel(const std::filesystem::path& path);
void
LoadPath(const std::filesystem::path& path);

void
ProjectMode();

void
Run(std::span<const char*> args);

bool
WriteScene(const std::filesystem::path& path);

void
LoadImGuiIni(std::string_view ini);

bool
AddAssetDir(std::string_view name, const std::filesystem::path& path);

void
ShowDock(std::string_view name, bool visible);

bool
LoadPbr(const std::filesystem::path& hdr);

} // namespace
