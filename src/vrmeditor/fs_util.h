#pragma once
#include <filesystem>
#include <sstream>

std::filesystem::path
get_home();

void
shell_open(const std::filesystem::path& path);
