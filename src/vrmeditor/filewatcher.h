#pragma once
#include <filesystem>

class FileWatcher
{
  struct FileWatcherImpl* m_impl;

public:
  FileWatcher();
  ~FileWatcher();
  void Update();
  void Watch(const std::filesystem::path& path);
};
