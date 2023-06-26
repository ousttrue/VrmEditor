#pragma once
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct FontSetting
{
  std::filesystem::path Path;
  std::vector<uint32_t> Ranges;
  bool IsIcon = false;
  uint32_t Flags = 0;

  void AddFont(int i, int size);
  static FontSetting JapaneseFont(const std::filesystem::path& path = {});
  static FontSetting NerdFont(const std::filesystem::path& path);
  static FontSetting EmojiFont(const std::filesystem::path& path = {});
};

class Gui
{
  struct GuiImpl* m_impl = nullptr;
  const void* m_window = nullptr;
  std::string m_glsl_version;

  int FontSize = 20;

  Gui();

public:
  std::vector<FontSetting> m_fonts;

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
  void SetFontSize(int size) { FontSize = size; }
  float Indent() const { return FontSize * 0.5f; }
  void DarkMode();

  void LoadState(std::string_view ini);
  std::string SaveState();

  // return WantSaveIniSettings
  bool NewFrame();
  void Render();

private:
  void Initialize();
};
