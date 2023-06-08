#include "app.h"
#include "assetdir.h"
#include "docks/export_dock.h"
#include "docks/gl3renderer_gui.h"
#include "docks/gui.h"
#include "docks/hierarchy_gui.h"
#include "docks/humanoid_dock.h"
#include "docks/imlogger.h"
#include "docks/imtimeline.h"
#include "docks/json_gui.h"
#include "docks/scene_selection.h"
#include "docks/view_dock.h"
#include "docks/vrm_dock.h"
#include "filewatcher.h"
#include "fs_util.h"
#include "humanpose/humanpose_stream.h"
#include "luahost.h"
#include "platform.h"
#include <ImGuizmo.h>
#include <cuber/mesh.h>
#include <fstream>
#include <glr/error_check.h>
#include <glr/gl3renderer.h>
#include <glr/rendering_env.h>
#include <gltfjson.h>
#include <gltfjson/glb.h>
#include <gltfjson/json_tree_exporter.h>
#include <grapho/orbitview.h>
#include <imgui.h>
// #include <remotery.h>
#include <vrm/animation_update.h>
#include <vrm/fileutil.h>
#include <vrm/gizmo.h>
#include <vrm/importer.h>
#include <vrm/timeline.h>

FileWatcher g_watcher;
std::filesystem::path g_shaderDir;

const auto WINDOW_TITLE = "VrmEditor";

static std::optional<std::filesystem::path>
getRelative(const std::filesystem::path& base,
            const std::filesystem::path& target)
{
  std::filesystem::path last;
  for (auto current = target.parent_path(); current != last;
       current = current.parent_path()) {
    if (current == base) {
      return std::filesystem::path(
        target.string().substr(base.string().size() + 1));
      break;
    }
  }
  return std::nullopt;
}

class App
{
  std::filesystem::path m_ini;
  std::shared_ptr<Gui> m_gui;
  std::list<std::shared_ptr<AssetDir>> m_assets;
  std::shared_ptr<HierarchyGui> m_hierarchy;

  std::shared_ptr<libvrm::Timeline> m_timeline;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::shared_ptr<struct SceneNodeSelection> m_selection;
  std::shared_ptr<grapho::OrbitView> m_staticView;
  std::shared_ptr<grapho::OrbitView> m_runtimeView;
  std::shared_ptr<glr::ViewSettings> m_settings;
  std::shared_ptr<glr::RenderingEnv> m_env;

  std::shared_ptr<JsonGui> m_json;
  std::shared_ptr<glr::Gl3RendererGui> m_gl3gui;

public:
  App()
  {
    g_watcher.AddCallback(
      [=](const std::filesystem::path& path) { this->OnFileUpdated(path); });

    auto file = get_home() / ".vrmeditor.ini.lua";
    m_ini = file.u8string();
    m_selection = std::make_shared<SceneNodeSelection>();
    m_hierarchy = std::make_shared<HierarchyGui>();
    m_staticView = std::make_shared<grapho::OrbitView>();
    m_runtimeView = std::make_shared<grapho::OrbitView>();
    m_timeline = std::make_shared<libvrm::Timeline>();
    m_env = std::make_shared<glr::RenderingEnv>();
    m_settings = std::make_shared<glr::ViewSettings>();
    m_settings->ShowCuber = false;

    auto window =
      Platform::Instance().CreateWindow(2000, 1200, false, WINDOW_TITLE);
    if (!window) {
      throw std::runtime_error("createWindow");
    }
    GL_ErrorClear("CreateWindow");

    Platform::Instance().OnDrops.push_back([=](auto& path) { LoadPath(path); });

    glr::Initialize();

    m_gui =
      std::make_shared<Gui>(window, Platform::Instance().glsl_version.c_str());
    m_gl3gui = std::make_shared<glr::Gl3RendererGui>();

    auto track = m_timeline->AddTrack("PoseStream", {});
    track->Callbacks.push_back([](auto time, auto repeat) {
      humanpose::HumanPoseStream::Instance().Update(time);
      return true;
    });

    m_json = std::make_shared<JsonGui>();
  }

  ~App() {}

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  void ProjectMode() { m_gui->DarkMode(); }

  std::shared_ptr<libvrm::RuntimeScene> SetScene(
    const std::shared_ptr<libvrm::GltfRoot>& table)
  {
    glr::Release();
    m_runtime = std::make_shared<libvrm::RuntimeScene>(table);
    m_timeline->Tracks.clear();

    std::weak_ptr<libvrm::RuntimeScene> weak = m_runtime;
    humanpose::HumanPoseStream::Instance().HumanPoseChanged.push_back(
      [weak](const auto& pose) {
        if (auto scene = weak.lock()) {
          scene->SetHumanPose(pose);
          return true;
        } else {
          return false;
        }
      });

    auto addDock = [gui = m_gui](const grapho::imgui::Dock& dock) {
      gui->AddDock(dock);
    };
    auto indent = m_gui->FontSize * 0.5f;

    {
      m_json->SetScene(table);

      HumanoidDock::Create(
        addDock, "humanoid-body", "humanoid-finger", m_runtime->m_table);

      ViewDock::CreateTPose(addDock,
                            "3D-View",
                            m_runtime->m_table,
                            m_env,
                            m_staticView,
                            m_settings,
                            m_selection);

      ViewDock::Create(addDock,
                       "Runtime-View",
                       m_runtime,
                       m_env,
                       m_runtimeView,
                       m_settings,
                       m_selection);

      VrmDock::CreateVrm(addDock, "vrm", m_runtime);

#ifndef NDEBUG
      ExportDock::Create(addDock, "[debug] export", m_runtime, indent);
#endif

      m_hierarchy->SetRuntimeScene(m_runtime, indent);
    }

    return m_runtime;
  }

  void LoadImGuiIni(std::string_view ini) { m_gui->LoadState(ini); }

  void SaveState()
  {
    std::ofstream os((const char*)m_ini.u8string().c_str());

    os << "-- This file is auto generated ini file.\n\n";

    // imnodes
    os << "vrmeditor.imnodes_load_ini [===[\n"
       << humanpose::HumanPoseStream::Instance().Save() << "\n]===]\n\n";

    for (auto& link : humanpose::HumanPoseStream::Instance().Links) {
      os << "vrmeditor.imnodes_add_link(" << link->Start << ", " << link->End
         << ")\n";
    }

    // imgui
    os << "vrmeditor.imgui_load_ini [===[\n"
       << m_gui->SaveState() << "\n]===]\n\n";

    // dock visibility
    for (auto& dock : m_gui->Docks) {
      if (!dock.IsOpen) {
        os << "vrmeditor.show_dock('" << dock.Name << "', false)\n";
      }
    }
    os << "\n";

    auto [width, height] = Platform::Instance().WindowSize();
    auto maximize = Platform::Instance().IsWindowMaximized();
    os << "vrmeditor.set_window_size(" << width << ", " << height << ", "
       << (maximize ? "true" : "false") << ")\n\n";
  }

  int Run()
  {
    // Create the main instance of Remotery.
    // You need only do this once per program.
    // Remotery* rmt;
    // rmt_CreateGlobalInstance(&rmt);

    GL_ErrorClear("CreateWindow");

    // must after App::App
    LuaEngine::Instance().DoFile(m_ini);

    auto addDock = [gui = m_gui](const grapho::imgui::Dock& dock) {
      gui->AddDock(dock);
    };

#ifndef NDEBUG
    ImTimeline::Create(addDock, "[animation] timeline", m_timeline);
    humanpose::HumanPoseStream::Instance().CreateDock(
      addDock, "[animation] input-stream");
#endif

    addDock({
      "logger",
      []() { ImLogger::Instance().Draw(); },
    });

    //   glr::CreateDock(addDock);
    // void
    // CreateDock(const AddDockFunc& addDock)
    {
      addDock(
        grapho::imgui::Dock("GL impl", [=]() { m_gl3gui->ShowSelectImpl(); }));

      addDock(grapho::imgui::Dock("GL selector", [=]() {
        //
        m_gl3gui->ShowSelector();
      }));

      addDock(grapho::imgui::Dock("GL selected shader source", [=]() {
        //
        m_gl3gui->ShowSelectedShaderSource();
      }));

      addDock(grapho::imgui::Dock("GL selected shader variables", [=]() {
        //
        m_gl3gui->ShowSelectedShaderVariables();
      }));
    }

    ViewDock::CreateSetting(
      addDock, "view-settings", m_env, m_runtimeView, m_settings);

    auto indent = m_gui->FontSize * 0.5f;
    addDock({
      "Json",
      [json = m_json, indent]() mutable { json->ShowSelector(indent); },
    });
    addDock({
      "Json-Inspector",
      [json = m_json]() mutable { json->ShowSelected(); },
    });
    addDock({ "3D-View" });

    addDock({
      "hierarchy",
      [hierarchy = m_hierarchy]() { hierarchy->ShowGui(); },
    });

    std::optional<libvrm::Time> lastTime;
    while (auto info = Platform::Instance().NewFrame()) {
      {
        // rmt_ScopedCPUSample(update, 0);

        ERROR_CHECK;

        g_watcher.Update();

        auto time = info->Time;

        if (lastTime) {
          auto delta = time - *lastTime;
          m_timeline->SetDeltaTime(delta);
        } else {
          m_timeline->SetDeltaTime({}, true);
        }
        humanpose::HumanPoseStream::Instance().Update(time);
        lastTime = time;

        // newFrame
        if (m_gui->NewFrame()) {
          SaveState();
        }
        ImGuizmo::BeginFrame();
      }

      {
        // rmt_ScopedCPUSample(gui, 0);
        m_gui->DockSpace();
      }

      {
        // rmt_ScopedCPUSample(render, 0);
        glr::ClearBackBuffer(info->Width, info->Height);

        m_gui->Render();
        Platform::Instance().Present();
      }
    }

    SaveState();

    // Destroy the main instance of Remotery.
    // rmt_DestroyGlobalInstance(rmt);

    return 0;
  }

  bool WriteScene(const std::filesystem::path& path)
  {
    std::stringstream ss;
    gltfjson::WriteFunc write = [&ss](std::string_view src) mutable {
      ss.write(src.data(), src.size());
    };
    gltfjson::tree::Exporter exporter{ write };
    exporter.Export(m_runtime->m_table->m_gltf->m_json);
    auto str = ss.str();

    std::ofstream os(path, std::ios::binary);
    if (!os) {
      return false;
    }

    return gltfjson::Glb{
      .JsonChunk = { (const uint8_t*)str.data(), str.size() },
      .BinChunk = m_runtime->m_table->m_bin.Bytes,
    }
      .WriteTo(os);
    return true;
  }

  // expose to lua
  const std::shared_ptr<Gui>& GetGui() const { return m_gui; }
  bool LoadPath(const std::filesystem::path& path)
  {
    auto extension = path.extension().string();
    std::transform(
      extension.begin(), extension.end(), extension.begin(), tolower);

    if (extension == ".gltf" || extension == ".glb" || extension == ".vrm" ||
        extension == ".vrma") {
      return LoadModel(path);
    }
    if (extension == ".bvh") {
      return humanpose::HumanPoseStream::Instance().LoadMotion(path);
    }
    // if (extension == ".fbx") {
    // }
    if (extension == ".hdr") {
      return LoadPbr(path);
    }
    return false;
  }

  bool LoadModel(const std::filesystem::path& path)
  {
    if (auto table = libvrm::LoadPath(path)) {
      auto scene = SetScene(*table);
      // bind time line

      for (auto& animation : scene->m_animations) {
        auto track = m_timeline->AddTrack("gltf", animation->Duration());
        std::weak_ptr<libvrm::RuntimeScene> weak = scene;
        track->Callbacks.push_back([animation, weak](auto time, bool repeat) {
          if (auto scene = weak.lock()) {
            libvrm::AnimationUpdate(
              *animation, time, scene->m_table->m_nodes, scene, repeat);
            return true;
          } else {
            return false;
          }
        });

        // first animation only
        break;
      }

      // update view position
      auto bb = scene->m_table->GetBoundingBox();
      m_staticView->Fit(bb.Min, bb.Max);
      m_runtimeView->Fit(bb.Min, bb.Max);
      m_env->SetShadowHeight(bb.Min.y);

      PLOG_INFO << path.string();

      return true;
    } else {
      PLOG_ERROR << table.error();
      SetScene(std::make_shared<libvrm::GltfRoot>());
      return false;
    }
  }

  bool LoadPbr(const std::filesystem::path& hdr)
  {
    return glr::LoadPbr_LOGL(hdr);
  }

  void LoadLua(const std::filesystem::path& path)
  {
    PLOG_INFO << path.string();
    if (auto ret = LuaEngine::Instance().DoFile(path)) {
      // ok
    } else {
      PLOG_ERROR << ret.error();
    }
  }

  bool AddAssetDir(std::string_view name, const std::filesystem::path& path)
  {
    if (!std::filesystem::is_directory(path)) {
      return false;
    }

    auto asset = std::make_shared<AssetDir>(name, path);
    asset->Update();
    m_assets.push_back(asset);

    auto callback = [this](const std::filesystem::path& path) {
      if (LoadPath(path)) {
        Platform::Instance().SetTitle(path.string());
      } else {
      }
    };

    auto dock = m_assets.back()->CreateDock(callback);
    m_gui->AddDock(dock);

    PLOG_INFO << name << " => " << path.string();
    return true;
  }

  void ShowDock(std::string_view name, bool visible)
  {
    m_gui->SetDockVisible(name, visible);
  }

  void SetShaderDir(const std::filesystem::path& path)
  {
    g_shaderDir = path;
    g_watcher.Watch(path);
    glr::SetShaderDir(path);
  }

  void OnFileUpdated(const std::filesystem::path& path)
  {
    if (auto rel = getRelative(g_shaderDir, path)) {
      glr::UpdateShader(*rel);
    }
  }

  void SetShaderChunkDir(const std::filesystem::path& path)
  {
    glr::SetShaderChunkDir(path);
  }
};
App g_app;

namespace app {

void
SetShaderDir(const std::filesystem::path& path)
{
  g_app.SetShaderDir(path);
}
void
SetShaderChunkDir(const std::filesystem::path& path)
{
  g_app.SetShaderChunkDir(path);
}

void
LoadModel(const std::filesystem::path& path)
{
  g_app.LoadModel(path);
}

void
LoadLua(const std::filesystem::path& path)
{
  g_app.LoadLua(path);
}

void
LoadPath(const std::filesystem::path& path)
{
  g_app.LoadPath(path);
}

void
ProjectMode()
{
  g_app.ProjectMode();
}

void
Run()
{
  g_app.Run();
}

bool
WriteScene(const std::filesystem::path& path)
{
  return g_app.WriteScene(path);
}

const std::shared_ptr<Gui>&
GetGui()
{
  return g_app.GetGui();
}

void
LoadImGuiIni(std::string_view ini)
{
  g_app.LoadImGuiIni(ini);
}

bool
AddAssetDir(std::string_view name, const std::filesystem::path& path)
{
  return g_app.AddAssetDir(name, path);
}

void
ShowDock(std::string_view name, bool visible)
{
  g_app.ShowDock(name, visible);
}

bool
LoadPbr(const std::filesystem::path& hdr)
{
  return g_app.LoadPbr(hdr);
}

} // namespace
