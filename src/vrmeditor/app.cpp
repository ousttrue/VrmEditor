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
#include <queue>
#include <vrm/animation_update.h>
#include <vrm/fileutil.h>
#include <vrm/gizmo.h>
#include <vrm/importer.h>
#include <vrm/timeline.h>
#ifdef _WIN32
#include "windows_helper.h"
#else
#endif

FileWatcher g_watcher;
std::filesystem::path g_shaderDir;

const auto WINDOW_TITLE = "VrmEditor";

std::queue<app::Task> g_tasks;

class App
{
  std::filesystem::path m_ini;
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
      Platform::Instance().WindowCreate(2000, 1200, false, WINDOW_TITLE);
    if (!window) {
      throw std::runtime_error("createWindow");
    }
    GL_ErrorClear("CreateWindow");

    Platform::Instance().OnDrops.push_back([=](auto& path) { LoadPath(path); });

    glr::Initialize();

    Gui::Instance().SetWindow(window,
                              Platform::Instance().glsl_version.c_str());
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

    auto addDock = [](const grapho::imgui::Dock& dock) {
      Gui::Instance().AddDock(dock);
    };
    auto indent = Gui::Instance().FontSize * 0.5f;

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
       << Gui::Instance().SaveState() << "\n]===]\n\n";

    // dock visibility
    for (auto& dock : Gui::Instance().Docks) {
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

    auto addDock = [](const grapho::imgui::Dock& dock) {
      Gui::Instance().AddDock(dock);
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

    auto indent = Gui::Instance().FontSize * 0.5f;
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
    while (true) {

      if (!g_tasks.empty()) {
        // dequeue task
        g_tasks.front()();
        g_tasks.pop();
      }

      auto info = Platform::Instance().NewFrame();
      if (!info) {
        break;
      }

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
        if (Gui::Instance().NewFrame()) {
          SaveState();
        }
        ImGuizmo::BeginFrame();
      }

      {
        // rmt_ScopedCPUSample(gui, 0);
        Gui::Instance().DockSpace();
      }

      {
        // rmt_ScopedCPUSample(render, 0);
        glr::ClearBackBuffer(info->Width, info->Height);

        Gui::Instance().Render();
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
    Gui::Instance().AddDock(dock);

    PLOG_INFO << name << " => " << path.string();
    return true;
  }
};
App g_app;

namespace app {

void
PostTask(const Task& task)
{
  g_tasks.push(task);
}

void
SetShaderDir(const std::filesystem::path& path)
{
  // g_app.SetShaderDir(path);
  g_shaderDir = path;
  g_watcher.Watch(path);
  glr::SetShaderDir(path);
}

void
SetShaderChunkDir(const std::filesystem::path& path)
{
  glr::SetShaderChunkDir(path);
}

void
LoadModel(const std::filesystem::path& path)
{
  g_app.LoadModel(path);
}

void
LoadPath(const std::filesystem::path& path)
{
  g_app.LoadPath(path);
}

void
ProjectMode()
{
  Gui::Instance().DarkMode();
}

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

void
Run(std::span<const char*> args)
{
  auto exe = GetExe();
  auto base = exe.parent_path().parent_path();
  app::SetShaderDir(base / "shaders");
  app::SetShaderChunkDir(base / "threejs_shader_chunks");

  // load user ~/.vrmeditor.lua
  auto user_conf = get_home() / ".vrmeditor.lua";
  if (std::filesystem::exists(user_conf)) {
    LuaEngine::Instance().DoFile(user_conf);
  }

  for (auto _arg : args) {
    std::string_view arg = _arg;
    if (arg.ends_with(".lua")) {
      // prooject mode
      ProjectMode();
      LuaEngine::Instance().DoFile(arg);
    } else {
      // viewermode
      LoadModel(arg);
    }
    // LoadPath(arg);
  }

  g_watcher.AddCallback([](const std::filesystem::path& path) {
    if (auto rel = getRelative(g_shaderDir, path)) {
      glr::UpdateShader(*rel);
    }
  });

  g_app.Run();
}

bool
WriteScene(const std::filesystem::path& path)
{
  return g_app.WriteScene(path);
}

void
LoadImGuiIni(std::string_view ini)
{
  Gui::Instance().LoadState(ini);
}

bool
AddAssetDir(std::string_view name, const std::filesystem::path& path)
{
  return g_app.AddAssetDir(name, path);
}

void
ShowDock(std::string_view name, bool visible)
{
  Gui::Instance().SetDockVisible(name, visible);
}

bool
LoadPbr(const std::filesystem::path& hdr)
{
  return g_app.LoadPbr(hdr);
}

} // namespace
