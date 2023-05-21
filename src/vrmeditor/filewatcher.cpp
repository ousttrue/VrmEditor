#include "filewatcher.h"
#include <FileWatcher/FileWatcher.h>
#include <iostream>

class UpdateListener : public FW::FileWatchListener
{
  OnFileUpdated m_callback;

public:
  UpdateListener(const OnFileUpdated& callback)
    : m_callback(callback)
  {
  }

  void handleFileAction(FW::WatchID watchid,
                        const std::string& dir,
                        const std::string& filename,
                        FW::Action action)
  {
    switch (action) {
      case FW::Actions::Add:
        // std::cout << "File (" << dir + "\\" + filename << ") Added! "
        //           << std::endl;
        m_callback(std::filesystem::path(dir) / filename);
        break;
      case FW::Actions::Delete:
        // std::cout << "File (" << dir + "\\" + filename << ") Deleted! "
        //           << std::endl;
        break;
      case FW::Actions::Modified:
        // std::cout << "File (" << dir + "\\" + filename << ") Modified! "
        //           << std::endl;
        m_callback(std::filesystem::path(dir) / filename);
        break;
      default:
        std::cout << "Should never happen!" << std::endl;
    }
  }
};

struct FileWatcherImpl
{
  FW::FileWatcher m_watcher;
  UpdateListener m_listener;

  FileWatcherImpl(const OnFileUpdated& callback)
    : m_listener(callback)
  {
  }

  void Update() { m_watcher.update(); }

  void Watch(const std::filesystem::path& path)
  {
    m_watcher.addWatch(path.string(), &m_listener);
  }
};

FileWatcher::FileWatcher(const OnFileUpdated& callback)
  : m_impl(new FileWatcherImpl(callback))
{
}

FileWatcher::~FileWatcher()
{
  delete m_impl;
}

void
FileWatcher::Update()
{
  m_impl->Update();
}

void
FileWatcher::Watch(const std::filesystem::path& path)
{
  m_impl->Watch(path);
}
