#pragma once
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

class Gui
{
  struct GuiImpl* m_impl = nullptr;
  const void* m_window = nullptr;
  std::string m_glsl_version;
  std::filesystem::path m_baseFont;
  std::filesystem::path m_japanseseFont;
  std::filesystem::path m_iconFont;

  Gui();

public:
  void SetWindow(const void* window, const char* glsl_version);
  void Shutdown();
  ~Gui();
  Gui(const Gui&) = delete;
  Gui& operator=(const Gui&) = delete;
  static Gui& Instance()
  {
    static Gui s_instance;
    return s_instance;
  }

  int FontSize = 20;
  void DarkMode();

  bool SetFont(const std::filesystem::path& path);
  bool AddJapaneseFont(const std::filesystem::path& path);
  bool AddIconFont(const std::filesystem::path& path);

  void LoadState(std::string_view ini);
  std::string SaveState();

  // return WantSaveIniSettings
  bool NewFrame();
  void Render();

private:
  void Initialize();
};
