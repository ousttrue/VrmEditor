#pragma once
#include <filesystem>
#include <string>

struct AssetView
{
  struct AssetViewImpl* m_impl;

  AssetView(std::string_view name, const std::filesystem::path& path);
  ~AssetView();
  void ShowGui();
  void Reload();
};
