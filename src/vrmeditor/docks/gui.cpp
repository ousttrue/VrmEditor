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

const auto DOCK_SPACE = "VRDocksPACE";

Gui::Gui(const void* window, const char* glsl_version)
  : m_window(window)
  , m_glsl_version(glsl_version)
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
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                                      // Platform Windows
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

#ifndef NDEBUG
  Docks.push_back({
    .Name = "[debug] demo",
    .Begin =
      [](auto, auto popen, auto) {
        ImGui::ShowDemoWindow(popen);
        return false;
      },
    .End = {},
  });
  Docks.back().IsOpen = false;

  Docks.push_back({
    .Name = "[debug] metrics",
    .Begin =
      [](const char*, bool* p_open, auto) {
        ImGui::ShowMetricsWindow(p_open);
        return false;
      },
    .End = {},
  });
  Docks.back().IsOpen = false;
#endif

  PostTask([this]() { LoadFont(); });

  Docks.push_back(grapho::imgui::Dock("[setting] font", [this]() {
    if (ImGui::SliderInt("fontSize", &FontSize, 10, 50)) {
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
  Docks.back().IsOpen = false;

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

void
Gui::DarkMode()
{
  ImGui::StyleColorsDark();
}

void
Gui::LoadState(std::string_view ini)
{
  m_resetLayout = false;
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
    auto iconFontSize = static_cast<float>(FontSize * 2.0f / 3.0f);
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

bool
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

  auto& io = ImGui::GetIO();
  return io.WantSaveIniSettings;
}

void
Gui::DockSpace()
{
  {
    grapho::imgui::BeginDockSpace(DOCK_SPACE);

    if (m_resetLayout) {
      for (auto& dock : Docks) {
        if (dock.Name == "Json" || dock.Name == "Json-Inspector" ||
            dock.Name == "3D-View") {
          dock.IsOpen = true;
        } else {
          dock.IsOpen = false;
        }
      }

      grapho::imgui::DockSpaceLayout(DOCK_SPACE, []() {
        auto root = ImGui::GetID(DOCK_SPACE);
        // auto json = ImGui::GetID("Json");
        // auto json_i = ImGui::GetID("Json-Inspector");
        // auto view = ImGui::GetID("3D-View");
        ImGuiID left_id, right_id;
        ImGui::DockBuilderSplitNode(
          root, ImGuiDir_Left, 0.3f, &left_id, &right_id);
        // ImGui::DockBuilderDockWindow("Json", left_id);
        ImGui::DockBuilderDockWindow("3D-View", right_id);

        ImGuiID top_id, bottom_id;
        ImGui::DockBuilderSplitNode(
          left_id, ImGuiDir_Down, 0.4f, &top_id, &bottom_id);
        ImGui::DockBuilderDockWindow("Json", top_id);
        ImGui::DockBuilderDockWindow("Json-Inspector", bottom_id);
      });

      m_resetLayout = false;
    }

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        static auto filters = ".*,.vrm,.glb,.gltf,.fbx,.bvh,.vrma,.hdr";
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

      if (ImGui::BeginMenu("Docks")) {
        if (ImGui::MenuItem("Reset", "")) {
          m_resetLayout = true;
        }
        ImGui::Separator();
        // Disabling fullscreen would allow the window to be moved to the front
        // of other windows, which we can't undo at the moment without finer
        // window depth/z control.
        for (auto& dock : Docks) {
          ImGui::MenuItem(dock.Name.c_str(), nullptr, &dock.IsOpen);
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
    ImGui::End();
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
        App::Instance().LoadPath(path);
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

  // docks
  for (auto& dock : Docks) {
    dock.Show();
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
