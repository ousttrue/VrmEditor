#include "gui.h"
#include "app.h"
#include "fs_util.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <ImGuiFileDialog.h>
#include <imnodes.h>
#include <iostream>

#include <IconsFontAwesome5.h>
#include <IconsMaterialDesign.h>

const auto OPEN_FILE_DIALOG = "OPEN_FILE_DIALOG";
const auto SAVE_FILE_DIALOG = "SAVE_FILE_DIALOG";

const auto DOCK_SPACE = "VRM_DOCKSPACE";

void
Dock::Show()
{
  if (IsOpen) {
    // begin
    if (UseWindow) {
      ImGui::SetNextWindowPos({ 100, 100 }, ImGuiCond_FirstUseEver);
      ImGui::SetNextWindowSize({ 100, 100 }, ImGuiCond_FirstUseEver);
      if (ImGui::Begin(Name.c_str(), &IsOpen)) {
        OnShow(&IsOpen);
      }
    } else {
      OnShow(&IsOpen);
    }
    if (UseWindow) {
      ImGui::End();
    }
  }
}

Gui::Gui(const void* window, const char* glsl_version)
  : m_window(window)
  , m_glsl_version(glsl_version)
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImNodes::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  // auto dir = get_home() / ".cache/vrmeditor";
  // if (!std::filesystem::exists(dir)) {
  //   // mkdir
  //   std::cout << "create: " << dir << std::endl;
  //   std::filesystem::create_directories(dir);
  // }
  auto file = get_home() / ".vrmeditor.ini";
  m_imgui_ini = file.u8string();

  io.IniFilename = (const char*)m_imgui_ini.c_str();

  io.ConfigFlags |=
    ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
    ImGuiConfigFlags_NavEnableGamepad;              // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
#ifdef _WIN32
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                                      // Platform Windows
#endif
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  // ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  m_docks.push_back(
    Dock("demo", [](bool* p_open) { ImGui::ShowDemoWindow(p_open); }));
  m_docks.back().IsOpen = false;

  m_docks.push_back(
    Dock("metrics", [](bool* p_open) { ImGui::ShowMetricsWindow(p_open); }));
  m_docks.back().IsOpen = false;

  PostTask([this]() { LoadFont(); });

  m_docks.push_back(Dock("font", [this]() {
    if (ImGui::SliderInt("fontSize", &m_fontSize, 10, 50)) {
      PostTask([this]() { LoadFont(); });
    }

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return nullptr. Please
    // handle those errors in your application (e.g. use an assertion, or
    // display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and
    // stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
    // below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use
    // Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at
    // runtime from the "fonts/" folder. See Makefile.emscripten for details.
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);
  }));
  m_docks.back().IsOpen = false;

  ImGuiFileDialog::Instance()->SetFileStyle(
    IGFD_FileStyleByTypeDir,
    nullptr,
    ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
    (const char*)u8" "); // for all dirs

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
  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
}

bool
Gui::SetFont(const std::filesystem::path& path)
{
  if (!std::filesystem::exists(path)) {
    return false;
  }
  m_baseFont = path;
  return true;

  ImNodes::DestroyContext();
  ImGui::DestroyContext();
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
Gui::LoadFont()
{

  ImGuiIO& io = ImGui::GetIO();

  if (m_initialized) {
    io.Fonts->Clear();
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
  }
  // Setup Platform/Renderer backends
  ImGui_ImplOpenGL3_Init(m_glsl_version.c_str());
  ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)m_window, true);
  m_initialized = true;

  ImFontConfig config;
  config.SizePixels = static_cast<float>(m_fontSize);
  if (m_baseFont.string().size()) {
    io.Fonts->AddFontFromFileTTF(m_baseFont.string().c_str(),
                                 static_cast<float>(m_fontSize));
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
    auto iconFontSize = static_cast<float>(m_fontSize * 2.0f / 3.0f);
    config.PixelSnapH = true;
    config.GlyphMinAdvanceX = iconFontSize;
    config.SizePixels = iconFontSize;
    io.Fonts->AddFontFromFileTTF(
      m_iconFont.string().c_str(), config.SizePixels, &config, icons_ranges);
  }

  io.Fonts->Build();
}

std::optional<MouseEvent>
Gui::BackgroundMouseEvent() const
{
  MouseEvent event{};

  auto& io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    // mouse event is consumed by ImGui
    if (io.MouseDown[1]) {
      event.RightDrag = Delta{ static_cast<int>(io.MouseDelta.x),
                               static_cast<int>(io.MouseDelta.y) };
    }
    if (io.MouseDown[2]) {
      event.MiddleDrag = Delta{ static_cast<int>(io.MouseDelta.x),
                                static_cast<int>(io.MouseDelta.y) };
    }
    if (io.MouseWheel) {
      event.Wheel = static_cast<int>(io.MouseWheel);
    }
  }
  return event;
}

void
Gui::NewFrame()
{

  if (!m_tasks.empty()) {
    // dequeue task
    m_tasks.front()();
    m_tasks.pop();
  }

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void
Gui::DockSpace()
{
  // If you strip some features of, this demo is pretty much equivalent to
  // calling DockSpaceOverViewport()! In most cases you should be able to just
  // call DockSpaceOverViewport() and ignore all the code below! In this
  // specific demo, we are not using DockSpaceOverViewport() because:
  // - we allow the host window to be floating/moveable instead of filling the
  // viewport (when opt_fullscreen == false)
  // - we allow the host window to have padding (when opt_padding == true)
  // - we have a local menu bar in the host window (vs. you could use
  // BeginMainMenuBar() + DockSpaceOverViewport() in your code!) TL;DR; this
  // demo is more complicated than what you would normally use. If we removed
  // all the options we are showcasing, this demo would become:
  //     void ShowExampleAppDockSpace()
  //     {
  //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  //     }

  static bool opt_fullscreen = true;
  static bool opt_padding = false;
  static ImGuiDockNodeFlags dockspace_flags =
    ImGuiDockNodeFlags_PassthruCentralNode;

  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window
  // not dockable into, because it would be confusing to have two docking
  // targets within each others.
  ImGuiWindowFlags window_flags =
    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  } else {
    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }

  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render
  // our background and handle the pass-thru hole, so we ask Begin() to not
  // render a background.
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  // Important: note that we proceed even if Begin() returns false (aka window
  // is collapsed). This is because we want to keep our DockSpace() active. If a
  // DockSpace() is inactive, all active windows docked into it will lose their
  // parent and become undocked. We cannot preserve the docking relationship
  // between an active window and an inactive docking, otherwise any change of
  // dockspace/settings would lead to windows being stuck in limbo and never
  // being visible.
  if (!opt_padding)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin(DOCK_SPACE, nullptr, window_flags);
  if (!opt_padding)
    ImGui::PopStyleVar();

  if (opt_fullscreen)
    ImGui::PopStyleVar(2);

  // Submit the DockSpace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID(DOCK_SPACE);
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  } else {
    assert(false);
  }

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      static auto filters = ".*,.vrm,.glb,.gltf,.fbx,.bvh,.vrma";
      if (ImGui::MenuItem("Open", "")) {
        ImGuiFileDialog::Instance()->OpenDialog(
          OPEN_FILE_DIALOG, "Open", filters, m_current.string().c_str());
      }

      if (ImGui::MenuItem("Save", "")) {
        ImGuiFileDialog::Instance()->OpenDialog(
          SAVE_FILE_DIALOG,
          "Save",
          filters,
          m_current.string().c_str(),
          "out",
          1,
          nullptr,
          ImGuiFileDialogFlags_ConfirmOverwrite);
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Options")) {
      // Disabling fullscreen would allow the window to be moved to the front of
      // other windows, which we can't undo at the moment without finer window
      // depth/z control.
      ImGui::MenuItem("Fullscreen", nullptr, &opt_fullscreen);
      ImGui::MenuItem("Padding", nullptr, &opt_padding);
      ImGui::Separator();

      if (ImGui::MenuItem("Flag: NoSplit",
                          "",
                          (dockspace_flags & ImGuiDockNodeFlags_NoSplit) !=
                            0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
      }
      if (ImGui::MenuItem("Flag: NoResize",
                          "",
                          (dockspace_flags & ImGuiDockNodeFlags_NoResize) !=
                            0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
      }
      if (ImGui::MenuItem("Flag: NoDockingInCentralNode",
                          "",
                          (dockspace_flags &
                           ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
      }
      if (ImGui::MenuItem(
            "Flag: AutoHideTabBar",
            "",
            (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
      }
      if (ImGui::MenuItem(
            "Flag: PassthruCentralNode",
            "",
            (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0,
            opt_fullscreen)) {
        dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
      }
      ImGui::Separator();

      // if (ImGui::MenuItem("Close", nullptr, false, p_open != nullptr)){
      //   *p_open = false;
      // }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Docks")) {
      // Disabling fullscreen would allow the window to be moved to the front of
      // other windows, which we can't undo at the moment without finer window
      // depth/z control.
      for (auto& dock : m_docks) {
        ImGui::MenuItem(dock.Name.c_str(), nullptr, &dock.IsOpen);
      }
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  ImGui::End();

  for (auto& dock : m_docks) {
    dock.Show();
  }

  // display
  if (ImGuiFileDialog::Instance()->Display(OPEN_FILE_DIALOG)) {
    // action if OK
    if (ImGuiFileDialog::Instance()->IsOk()) {
      auto path =
        std::filesystem::path(ImGuiFileDialog::Instance()->GetCurrentPath()) /
        ImGuiFileDialog::Instance()->GetFilePathName();
      // action
      // std::cout << filePathName << "::" << filePath << std::endl;
      if (std::filesystem::exists(path)) {
        m_current = path.parent_path();
        App::Instance().LoadModel(path);
      }
    }

    // close
    ImGuiFileDialog::Instance()->Close();
  }
  if (ImGuiFileDialog::Instance()->Display(SAVE_FILE_DIALOG)) {
    // action if OK
    if (ImGuiFileDialog::Instance()->IsOk()) {
      auto path =
        std::filesystem::path(ImGuiFileDialog::Instance()->GetCurrentPath()) /
        ImGuiFileDialog::Instance()->GetFilePathName();
      // action
      // std::cout << filePathName << "::" << filePath << std::endl;
      m_current = path.parent_path();
      App::Instance().WriteScene(path);
    }

    // close
    ImGuiFileDialog::Instance()->Close();
  }
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
