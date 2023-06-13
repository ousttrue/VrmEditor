#include "gui.h"
#include "app.h"
#include "dockspace.h"
#include "fs_util.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <misc/freetype/imgui_freetype.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <ImGuiFileDialog.h>
#include <imnodes.h>
#include <iostream>

#include <IconsFontAwesome5.h>
#include <IconsMaterialDesign.h>

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

bool
Gui::SetFont(const std::filesystem::path& path)
{
  if (!std::filesystem::exists(path)) {
    return false;
  }
  m_baseFont = path;
  return true;
}

bool
Gui::AddJapaneseFont(const std::filesystem::path& path)
{
  if (!std::filesystem::exists(path)) {
    return false;
  }
  m_japanseseFont = path;
  return true;
}

bool
Gui::AddIconFont(const std::filesystem::path& path)
{
  if (!std::filesystem::exists(path)) {
    return false;
  }
  m_iconFont = path;
  return true;
}

void
Gui::Initialize()
{
  ImGuiIO& io = ImGui::GetIO();

  Shutdown();

  m_impl = new GuiImpl(m_window, m_glsl_version.c_str());

  ImFontConfig config;
  config.SizePixels = static_cast<float>(FontSize);
  if (m_baseFont.string().size()) {
    io.Fonts->AddFontFromFileTTF(m_baseFont.string().c_str(),
                                 static_cast<float>(FontSize));
  } else {
    // default font
    io.Fonts->AddFontDefault(&config);
  }

  config.MergeMode = true;
  if (m_japanseseFont.string().size()) {
    io.Fonts->AddFontFromFileTTF(m_japanseseFont.string().c_str(),
                                 config.SizePixels,
                                 &config,
                                 io.Fonts->GetGlyphRangesJapanese());
  }

  if (m_iconFont.string().size()) {
    // static const ImWchar icons_ranges[] = {0xf000, 0xf3ff, 0};
    static const ImWchar icons_ranges[] = {
      //
      ICON_MIN_FA,
      ICON_MAX_FA,
      //
      ICON_MIN_MD,
      ICON_MAX_MD,
      //
      0,
    };
    auto iconFontSize = static_cast<float>(FontSize);
    config.PixelSnapH = true;
    config.GlyphMinAdvanceX = iconFontSize;
    config.SizePixels = iconFontSize;
    io.Fonts->AddFontFromFileTTF(
      m_iconFont.string().c_str(), config.SizePixels, &config, icons_ranges);
  }

  // emoji
  {
    auto iconFontSize = static_cast<float>(FontSize);
    config.PixelSnapH = true;
    config.SizePixels = iconFontSize;
    // config.OversampleH = 1;
    // config.OversampleV = 1;
    config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static ImWchar const emoji_ranges[] = {
      0x1,
      0x1ffff,
      0,
    };
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\seguiemj.ttf",
                                 config.SizePixels,
                                 &config,
                                 emoji_ranges);
  }

  io.Fonts->Build();
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
