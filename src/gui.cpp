#include "gui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>

const auto DOCK_SPACE = "VRM_DOCKSPACE";

void Dock::show() {
  if (p_open) {
    // begin
    if (use_window) {
      if (ImGui::Begin(name.c_str(), &p_open)) {
        on_show(&p_open);
      }
    } else {
      on_show(&p_open);
    }
    if (use_window) {
      ImGui::End();
    }
  }
}

Gui::Gui(const void *window, const char *glsl_version)
    : window_(window), glsl_version_(glsl_version) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;              // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                                      // Platform Windows
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  // ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  m_docks.push_back(
      Dock("demo", [](bool *p_open) { ImGui::ShowDemoWindow(p_open); }));

  m_docks.push_back(
      Dock("metrics", [](bool *p_open) { ImGui::ShowMetricsWindow(p_open); }));

  postTask([this]() { loadFont(); });

  m_docks.push_back(Dock("font", [this]() {
    if (ImGui::SliderInt("fontSize", &fontSize_, 10, 50)) {
      postTask([this]() { loadFont(); });
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
}

Gui::~Gui() {
  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
}

void Gui::loadFont() {

  ImGuiIO &io = ImGui::GetIO();

  if (initialized_) {
    io.Fonts->Clear();
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
  }
  // Setup Platform/Renderer backends
  ImGui_ImplOpenGL3_Init(glsl_version_.c_str());
  ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)window_, true);
  initialized_ = true;

  for (auto &font : fonts_) {
    if (font.font.empty()) {
      // default font
      ImFontConfig config;
      config.SizePixels = static_cast<float>(fontSize_);
      io.Fonts->AddFontDefault(&config);
    } else {
    }
  }

  io.Fonts->Build();
}

std::optional<MouseEvent> Gui::backgroundMouseEvent() const {
  MouseEvent event{};

  auto &io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    // mouse event is consumed by ImGui
    if (io.MouseDown[1]) {
      event.rightDrag = Delta{static_cast<int>(io.MouseDelta.x),
                              static_cast<int>(io.MouseDelta.y)};
    }
    if (io.MouseDown[2]) {
      event.middleDrag = Delta{static_cast<int>(io.MouseDelta.x),
                               static_cast<int>(io.MouseDelta.y)};
    }
    if (io.MouseWheel) {
      event.wheel = static_cast<int>(io.MouseWheel);
    }
  }
  return event;
}

void Gui::dockspace() {
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
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
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
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID(DOCK_SPACE);
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  } else {
    assert(false);
  }

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      // Disabling fullscreen would allow the window to be moved to the front of
      // other windows, which we can't undo at the moment without finer window
      // depth/z control.
      ImGui::MenuItem("Fullscreen", nullptr, &opt_fullscreen);
      ImGui::MenuItem("Padding", nullptr, &opt_padding);
      ImGui::Separator();

      if (ImGui::MenuItem("Flag: NoSplit", "",
                          (dockspace_flags & ImGuiDockNodeFlags_NoSplit) !=
                              0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
      }
      if (ImGui::MenuItem("Flag: NoResize", "",
                          (dockspace_flags & ImGuiDockNodeFlags_NoResize) !=
                              0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
      }
      if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "",
                          (dockspace_flags &
                           ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
      }
      if (ImGui::MenuItem(
              "Flag: AutoHideTabBar", "",
              (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
      }
      if (ImGui::MenuItem(
              "Flag: PassthruCentralNode", "",
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
      for (auto &dock : m_docks) {
        ImGui::MenuItem(dock.name.c_str(), nullptr, &dock.p_open);
      }
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  ImGui::End();

  for (auto &dock : m_docks) {
    dock.show();
  }
}

void Gui::newFrame() {

  if (!tasks_.empty()) {
    // dequeue task
    tasks_.front()();
    tasks_.pop();
  }

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Gui::update() {
  dockspace();
  //
  // // 2. Show a simple window that we create ourselves. We use a Begin/End
  // pair
  // // to create a named window.
  // {
  //   static float f = 0.0f;
  //   static int counter = 0;
  //
  //   ImGui::Begin("Hello, world!"); // Create a window called "Hello,
  //   world!"
  //                                  // and append into it.
  //
  //   ImGui::Text("This is some useful text."); // Display some text (you can
  //                                             // use a format strings too)
  //   ImGui::Checkbox(
  //       "Demo Window",
  //       &show_demo_window); // Edit bools storing our window open/close
  //       state
  //   ImGui::Checkbox("Another Window", &show_another_window);
  //
  //   ImGui::SliderFloat("float", &f, 0.0f,
  //                      1.0f); // Edit 1 float using a slider from 0.0f
  //                      to 1.0f
  //   ImGui::ColorEdit3(
  //       "clear color",
  //       (float *)&clear_color); // Edit 3 floats representing a color
  //
  //   if (ImGui::Button("Button")) // Buttons return true when clicked (most
  //                                // widgets return true when
  //                                edited/activated)
  //     counter++;
  //   ImGui::SameLine();
  //   ImGui::Text("counter = %d", counter);
  //
  //   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
  //               1000.0f / io.Framerate, io.Framerate);
  //   ImGui::End();
  // }
  //
  // // 3. Show another simple window.
  // if (show_another_window) {
  //   ImGui::Begin(
  //       "Another Window",
  //       &show_another_window); // Pass a pointer to our bool variable (the
  //                              // window will have a closing button that
  //                              will
  //                              // clear the bool when clicked)
  //   ImGui::Text("Hello from another window!");
  //   if (ImGui::Button("Close Me"))
  //     show_another_window = false;
  //   ImGui::End();
  // }
}

void Gui::render() {
  // Rendering
  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  auto &io = ImGui::GetIO();

  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we
  // save/restore it to make it easier to paste this code elsewhere.
  //  For this specific demo app we could also call
  //  glfwMakeContextCurrent(window) directly)
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}
