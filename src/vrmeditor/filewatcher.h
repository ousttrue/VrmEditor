#pragma once
#include <filesystem>
#include <functional>

using OnFileUpdated = std::function<void(const std::filesystem::path& path)>;

class FileWatcher
{
  struct FileWatcherImpl* m_impl;

public:
  FileWatcher(const OnFileUpdated& callback);
  ~FileWatcher();
  void Watch(const std::filesystem::path& path);
  void Update();
};
