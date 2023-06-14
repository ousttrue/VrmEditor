#include "dockspace.h"
#include "app.h"
#include "config.h"
#include <ImGuiFileDialog.h>
#include <filesystem>
#include <imgui.h>

const auto OPEN_FILE_DIALOG = "OPEN_FILE_DIALOG";
const auto SAVE_FILE_DIALOG = "SAVE_FILE_DIALOG";
const auto DOCK_SPACE = "VRDocksPACE";

DockSpaceManager::DockSpaceManager()
{
  // Docks.push_back(grapho::imgui::Dock("[setting] font", [this]() {
  //   if (ImGui::SliderInt("fontSize", &FontSize, 10, 50)) {
  //     app::PostTask([=]() { Shutdown(); });
  //   }
  // }));
  // Docks.back().IsOpen = false;

  // ImGuiFileDialog::Instance()->SetFileStyle(
  //   IGFD_FileStyleByTypeDir,
  //   nullptr,
  //   ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
  //   (const char*)u8" "); // for all dirs

#ifndef NDEBUG
  DebugDocks.push_back({
    .Name = "🚧demo",
    .Begin =
      [](auto, auto popen, auto) {
        ImGui::ShowDemoWindow(popen);
        return false;
      },
    .End = {},
  });
  DebugDocks.back().IsOpen = false;

  DebugDocks.push_back({
    .Name = "🚧metrics",
    .Begin =
      [](const char*, bool* p_open, auto) {
        ImGui::ShowMetricsWindow(p_open);
        return false;
      },
    .End = {},
  });
  DebugDocks.back().IsOpen = false;
#endif
}

void
DockSpaceManager::Reset()
{
  Docks.clear();

  for (auto& callback : OnResetCallbacks) {
    callback();
  }
}

void
DockSpaceManager::AddDock(const grapho::imgui::Dock& dock, bool tmporary)
{
  if (tmporary) {
    TmpDocks.push_back(dock);
    return;
  }

  bool visible = true;
  auto found = std::find_if(Docks.begin(), Docks.end(), [&dock](auto& d) {
    return d.Name == dock.Name;
  });
  if (found != Docks.end()) {
    visible = found->IsOpen;
    Docks.erase(found);
  }

  Docks.push_back(dock);
  Docks.back().IsOpen = visible;
}

void
DockSpaceManager::SetDockVisible(std::string_view name, bool visible)
{
  for (auto& dock : Docks) {
    if (dock.Name == name) {
      dock.IsOpen = visible;
      return;
    }
  }

  Docks.push_back({ .Name = { name.begin(), name.end() }, .IsOpen = visible });
}

static void
ShowDocks(std::list<grapho::imgui::Dock>& docks)
{
  for (auto& dock : docks) {
    dock.Show();
  }
}

static void
MenuDocks(std::list<grapho::imgui::Dock>& docks)
{
  for (auto& dock : docks) {
    ImGui::MenuItem(dock.Name.c_str(), nullptr, &dock.IsOpen);
  }
}

void
DockSpaceManager::ShowGui()
{
  {
    grapho::imgui::BeginDockSpace(DOCK_SPACE);

    if (m_resetLayout) {
      Reset();

      grapho::imgui::DockSpaceLayout(DOCK_SPACE, []() {
        auto root = ImGui::GetID(DOCK_SPACE);
        // auto json = ImGui::GetID("Json");
        // auto json_i = ImGui::GetID("Json-Inspector");
        // auto view = ImGui::GetID("3D-View");
        ImGuiID left_id, right_id;
        ImGui::DockBuilderSplitNode(
          root, ImGuiDir_Left, 0.3f, &left_id, &right_id);
        // ImGui::DockBuilderDockWindow("Json", left_id);
        ImGui::DockBuilderDockWindow(app::DOCKNAME_VIEW, right_id);
        ImGui::DockBuilderDockWindow(app::DOCKNAME_JSON, left_id);

        // ImGuiID top_id, bottom_id;
        // ImGui::DockBuilderSplitNode(
        //   left_id, ImGuiDir_Up, 0.4f, &top_id, &bottom_id);
        // ImGui::DockBuilderDockWindow("Json-Inspector", bottom_id);
      });

      m_resetLayout = false;
    }

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        static auto filters = ".vrm,.glb,.gltf,.fbx,.bvh,.vrma,.hdr,.*";
        if (ImGui::MenuItem("Open", "")) {
          ImGuiFileDialog::Instance()->OpenDialog(
            OPEN_FILE_DIALOG,
            "Open",
            filters,
            m_fileDialogCurrent.string().c_str());
        }

        if (ImGui::MenuItem("Save", "")) {
          ImGuiFileDialog::Instance()->OpenDialog(
            SAVE_FILE_DIALOG,
            "Save",
            filters,
            m_fileDialogCurrent.string().c_str(),
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
        // Disabling fullscreen would allow the window to be moved to the
        // front of other windows, which we can't undo at the moment without
        // finer window depth/z control.
        MenuDocks(Docks);
        ImGui::Separator();
        MenuDocks(TmpDocks);
        ImGui::Separator();
        MenuDocks(DebugDocks);

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Help")) {
        ImGui::MenuItem("Version", PACKAGE_VERSION);
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
        m_fileDialogCurrent = path.parent_path();
        app::TaskLoadPath(path);
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
      m_fileDialogCurrent = path.parent_path();
      app::WriteScene(path);
    }

    // close
    ImGuiFileDialog::Instance()->Close();
  }

  // docks
  ShowDocks(Docks);
  ShowDocks(TmpDocks);
  ShowDocks(DebugDocks);
}
