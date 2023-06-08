#include "filewatcher.h"
#include <FileWatcher/FileWatcher.h>
#include <iostream>
#include <list>

class UpdateListener : public FW::FileWatchListener
{

public:
  std::list<OnFileUpdated> Callbacks;

  void handleFileAction(FW::WatchID watchid,
                        const std::string& dir,
                        const std::string& filename,
                        FW::Action action)
  {
    switch (action) {
      case FW::Actions::Add:
        for (auto& callback : Callbacks) {
          callback(std::filesystem::path(dir) / filename);
        }
        break;
      case FW::Actions::Delete:
        // std::cout << "File (" << dir + "\\" + filename << ") Deleted! "
        //           << std::endl;
        break;
      case FW::Actions::Modified:
        for (auto& callback : Callbacks) {
          callback(std::filesystem::path(dir) / filename);
        }
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

  void Update() { m_watcher.update(); }

  void Watch(const std::filesystem::path& path)
  {
    m_watcher.addWatch(path.string(), &m_listener);
  }
};

FileWatcher::FileWatcher()
  : m_impl(new FileWatcherImpl)
{
}

FileWatcher::~FileWatcher()
{
  delete m_impl;
}

void
FileWatcher::AddCallback(const OnFileUpdated& callback)
{
  m_impl->m_listener.Callbacks.push_back(callback);
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
