#include "GuiApp.h"
#include <imgui.h>

GuiApp::GuiApp() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
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
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);
}

GuiApp::~GuiApp() { ImGui::DestroyContext(); }

void GuiApp::UpdateGuiDockspace() {

  auto flags =
      (ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
       ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
       ImGuiWindowFlags_NoNavFocus);

  auto viewport = ImGui::GetMainViewport();
  auto pos = viewport->Pos;
  auto size = viewport->Size;
  ImGui::SetNextWindowPos(pos);
  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});

  // DockSpace
  auto name = "dockspace";
  ImGui::Begin(name, nullptr, flags);
  ImGui::PopStyleVar(3);
  auto dockspace_id = ImGui::GetID(name);
  ImGui::DockSpace(dockspace_id, {}, ImGuiDockNodeFlags_PassthruCentralNode);
  ImGui::End();
}

void GuiApp::UpdateGui() {
  ImGuiIO &io = ImGui::GetIO();
  ImGui::NewFrame();

  // camera
  turntable_.SetSize(static_cast<int>(io.DisplaySize.x),
                     static_cast<int>(io.DisplaySize.y));
  if (!io.WantCaptureMouse) {
    if (io.MouseDown[ImGuiMouseButton_Right]) {
      turntable_.YawPitch(static_cast<int>(io.MouseDelta.x),
                          static_cast<int>(io.MouseDelta.y));
    }
    if (io.MouseDown[ImGuiMouseButton_Middle]) {
      turntable_.Shift(static_cast<int>(io.MouseDelta.x),
                       static_cast<int>(io.MouseDelta.y));
    }
    turntable_.Dolly(static_cast<int>(io.MouseWheel));
  }
  turntable_.Update(projection, view);

  // Widgets
  UpdateGuiDockspace();

  // 1. Show the big demo window (Most of the sample code is in
  // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
  // ImGui!).
  if (show_demo_window)
    ImGui::ShowDemoWindow(&show_demo_window);

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair
  // to create a named window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                   // and append into it.

    ImGui::Text("This is some useful text."); // Display some text (you can
                                              // use a format strings too)
    ImGui::Checkbox(
        "Demo Window",
        &show_demo_window); // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f,
                       1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3(
        "clear color",
        (float *)&clear_color_); // Edit 3 floats representing a color

    if (ImGui::Button("Button")) // Buttons return true when clicked (most
                                 // widgets return true when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
  }

  // 3. Show another simple window.
  if (show_another_window) {
    ImGui::Begin(
        "Another Window",
        &show_another_window); // Pass a pointer to our bool variable (the
                               // window will have a closing button that will
                               // clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      show_another_window = false;
    ImGui::End();
  }
}

ImDrawData *GuiApp::RenderGui() {
  ImGui::Render();

  clear_color[0] = clear_color_[0] * clear_color_[3];
  clear_color[1] = clear_color_[1] * clear_color_[3];
  clear_color[2] = clear_color_[2] * clear_color_[3];
  clear_color[3] = clear_color_[3];

  return ImGui::GetDrawData();
}
