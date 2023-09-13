#include "gui.h"
#include "dockspace.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <misc/freetype/imgui_freetype.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <IconsMaterialDesign.h>
#include <ImGuiFileDialog.h>
#include <imnodes.h>
#include <plog/Log.h>

void
FontSetting::AddFont(int i, int size)
{
  ImGuiIO& io = ImGui::GetIO();
  ImFontConfig config;
  config.SizePixels = static_cast<float>(size);
  config.PixelSnapH = true;
  config.OversampleH = 1;
  config.OversampleV = 1;
  config.FontBuilderFlags |= Flags;
  if (i) {
    PLOG_INFO << "merge_font: " << (const char*)Path.u8string().c_str();
    config.MergeMode = true;
  } else {
    PLOG_INFO << "add_font: " << (const char*)Path.u8string().c_str();
  }
  if (IsIcon) {
    config.GlyphMinAdvanceX = config.SizePixels;
    // config.GlyphMaxAdvanceX = config.SizePixels;
  }
  io.Fonts->AddFontFromFileTTF(
    Path.string().c_str(), config.SizePixels, &config, Ranges.data());
}

FontSetting
FontSetting::JapaneseFont(const std::filesystem::path& _path)
{
  auto path = _path;
  if (path.empty()) {
    path = "C:/Windows/Fonts/msgothic.ttc";
  }
  FontSetting setting{
    path,
  };
  auto range = ImGui::GetIO().Fonts->GetGlyphRangesJapanese();
  for (auto p = range; *p; ++p) {
    setting.Ranges.push_back(*p);
  }
  setting.Ranges.push_back(0);
  return setting;
}

FontSetting
FontSetting::NerdFont(const std::filesystem::path& path)
{
  return {
    path,
    { ICON_MIN_FA, ICON_MAX_FA, ICON_MIN_MD, ICON_MAX_MD, 0 },
    true,
  };
}

FontSetting
FontSetting::EmojiFont(const std::filesystem::path& _path)
{
  auto path = _path;
  if (path.empty()) {
    path = "c:\\Windows\\Fonts\\seguiemj.ttf";
  }
  return {
    path,
    {
      0x1,
      0x1ffff,
      0,
    },
    true,
    ImGuiFreeTypeBuilderFlags_LoadColor,
  };
}

struct GuiImpl
{
  GuiImpl(const void* window, const char* glsl_version)
  {
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window, true);
  }

  ~GuiImpl()
  {
    auto& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
  }
};

Gui::Gui()
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImNodes::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  // stop ImGui auto save
  io.IniFilename = nullptr;

  io.ConfigFlags |=
    ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
    ImGuiConfigFlags_NavEnableGamepad;              // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
#ifdef _WIN32
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                      // / Platform Windows
#endif
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  ImGui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

#if _WIN32
  std::filesystem::path user_home = std::getenv("USERPROFILE");
#else
  std::filesystem::path user_home = std::getenv("HOME");
#endif

  ImGuiFileDialog::Instance()->prBookmarks.push_back({
    .name = "Home",
    .path = (const char*)user_home.u8string().c_str(),
  });
  ImGuiFileDialog::Instance()->prBookmarks.push_back({
    .name = "Desktop",
    .path = (const char*)(user_home / "Desktop").u8string().c_str(),
  });
}

Gui::~Gui()
{
  Shutdown();
}

void
Gui::SetWindow(const void* window, const char* glsl_version)
{
  Shutdown();
  m_window = window;
  m_glsl_version = glsl_version;
}

void
Gui::Shutdown()
{
  if (m_impl) {
    delete m_impl;
    m_impl = nullptr;
  }
}

void
Gui::DarkMode()
{
  ImGui::StyleColorsDark();
}

void
Gui::LoadState(std::string_view ini)
{
  DockSpaceManager::Instance().m_resetLayout = false;
  ImGui::LoadIniSettingsFromMemory(ini.data(), ini.size());
}

std::string
Gui::SaveState()
{
  size_t size;
  auto p = ImGui::SaveIniSettingsToMemory(&size);
  return { p, p + size };
}

void
Gui::Initialize()
{
  Shutdown();
  assert(m_impl == nullptr);
  m_impl = new GuiImpl(m_window, m_glsl_version.c_str());

  if (m_fonts.empty()) {
    m_fonts.push_back(FontSetting::JapaneseFont());
    m_fonts.push_back(FontSetting::EmojiFont());
  }

  for (int i = 0; i < m_fonts.size(); ++i) {
    m_fonts[i].AddFont(i, FontSize);
  }

  ImGui::GetIO().Fonts->Build();
}

bool
Gui::NewFrame()
{
  if (!m_impl) {
    Initialize();
  }

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  auto& io = ImGui::GetIO();
  return io.WantSaveIniSettings;
}

void
Gui::Render()
{
  // Rendering
  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  auto& io = ImGui::GetIO();

  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we
  // save/restore it to make it easier to paste this code elsewhere.
  //  For this specific demo app we could also call
  //  glfwMakeContextCurrent(window) directly)
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}
