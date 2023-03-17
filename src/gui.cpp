#include "gui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Gui::Gui(const void *window, const char *glsl_version) {
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
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype
  // for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at
  // runtime from the "fonts/" folder. See Makefile.emscripten for details.
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);
}

Gui::~Gui() {}

void Gui::render() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // // 1. Show the big demo window (Most of the sample code is in
  // // ImGui::ShowDemoWindow()! You can browse its code to learn more about
  // Dear
  // // ImGui!).
  // if (show_demo_window)
  ImGui::ShowDemoWindow(&show_demo_window);
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
