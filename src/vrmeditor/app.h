#pragma once
#include <filesystem>
#include <functional>
#include <span>

namespace app {

using Task = std::function<void()>;

void
PostTask(const Task& task);
void
TaskLoadModel(const std::filesystem::path& path);
void
TaskLoadPath(const std::filesystem::path& path);
void
TaskLoadPbr(const std::filesystem::path& hdr);

void
SetShaderDir(const std::filesystem::path& path);

void
Run(std::span<const char*> args);

bool
WriteScene(const std::filesystem::path& path);

bool
AddAssetDir(std::string_view name, const std::filesystem::path& path);

} // namespace
