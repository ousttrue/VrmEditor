#include "filewatcher.h"
#include <FileWatcher/FileWatcher.h>
#include <iostream>

class UpdateListener : public FW::FileWatchListener
{
public:
  UpdateListener() {}
  void handleFileAction(FW::WatchID watchid,
                        const std::string& dir,
                        const std::string& filename,
                        FW::Action action)
  {
    switch (action) {
      case FW::Actions::Add:
        std::cout << "File (" << dir + "\\" + filename << ") Added! "
                  << std::endl;
        break;
      case FW::Actions::Delete:
        std::cout << "File (" << dir + "\\" + filename << ") Deleted! "
                  << std::endl;
        break;
      case FW::Actions::Modified:
        std::cout << "File (" << dir + "\\" + filename << ") Modified! "
                  << std::endl;
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
FileWatcher::Update()
{
  m_impl->Update();
}

void
FileWatcher::Watch(const std::filesystem::path& path)
{
  m_impl->Watch(path);
}
