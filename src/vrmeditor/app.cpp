#include <GL/glew.h>

#include "app.h"
#include "assetdir.h"
#include "docks/export_dock.h"
#include "docks/gui.h"
#include "docks/humanoid_dock.h"
#include "docks/imlogger.h"
#include "docks/imtimeline.h"
#include "docks/json_gui.h"
#include "docks/scene_dock.h"
#include "docks/scene_selection.h"
#include "docks/view_dock.h"
#include "docks/vrm_dock.h"
#include "fs_util.h"
#include "glr/gl3renderer.h"
#include "glr/rendering_env.h"
#include "humanpose/humanpose_stream.h"
#include "luahost.h"
#include "platform.h"
#include <ImGuizmo.h>
#include <cuber/mesh.h>
#include <fstream>
#include <grapho/orbitview.h>
#include <imgui.h>
#include <vrm/animation.h>
#include <vrm/exporter.h>
#include <vrm/fileutil.h>
#include <vrm/gizmo.h>
#include <vrm/glb.h>
#include <vrm/importer.h>
#include <vrm/runtimescene/animation_update.h>
#include <vrm/timeline.h>

const auto WINDOW_TITLE = "VrmEditor";

App::App()
{
  m_logger = std::make_shared<ImLogger>();
  m_lua = std::make_shared<LuaEngine>();
  auto file = get_home() / ".vrmeditor.ini.lua";
  m_ini = file.u8string();
  m_selection = std::make_shared<SceneNodeSelection>();

  m_view = std::make_shared<grapho::OrbitView>();
  m_timeline = std::make_shared<libvrm::Timeline>();
  m_env = std::make_shared<glr::RenderingEnv>();
  m_settings = std::make_shared<glr::ViewSettings>();
  m_settings->ShowCuber = false;

  m_platform = std::make_shared<Platform>();
  auto window = m_platform->CreateWindow(2000, 1200, false, WINDOW_TITLE);
  if (!window) {
    throw std::runtime_error("createWindow");
  }

  Log(LogLevel::Info) << "GL_VERSION: " << glGetString(GL_VERSION);
  Log(LogLevel::Info) << "GL_VENDOR: " << glGetString(GL_VENDOR);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("glewInit");
  }

  PoseStream = std::make_shared<humanpose::HumanPoseStream>();
  m_gui = std::make_shared<Gui>(window, m_platform->glsl_version.c_str());

  // auto track = m_timeline->AddTrack("PoseStream", {});
  // track->Callbacks.push_back([pose = PoseStream](auto time, auto repeat) {
  //   pose->Update(time);
  //   return true;
  // });

  m_jsonGui = std::make_shared<JsonGui>();
  auto indent = m_gui->FontSize * 0.5f;
  m_gui->AddDock(
    Dock("gltf-json",
         [jsonGui = m_jsonGui, indent](const char* title, bool* p_open) {
           jsonGui->Show(title, p_open, indent);
         }));
}

App::~App() {}

std::shared_ptr<runtimescene::RuntimeScene>
App::SetScene(const std::shared_ptr<libvrm::gltf::Scene>& table)
{
  m_scene = std::make_shared<runtimescene::RuntimeScene>(table);
  m_timeline->Tracks.clear();

  std::weak_ptr<runtimescene::RuntimeScene> weak = m_scene;
  PoseStream->HumanPoseChanged.push_back([weak](const auto& pose) {
    if (auto scene = weak.lock()) {
      // scene->m_table->SetHumanPose(pose);
      return true;
    } else {
      return false;
    }
  });

  auto addDock = [gui = m_gui](const Dock& dock) { gui->AddDock(dock); };
  auto indent = m_gui->FontSize * 0.5f;

  {
    m_jsonGui->SetScene(m_scene->m_table);
    HumanoidDock::Create(
      addDock, "humanoid-body", "humanoid-finger", m_scene->m_table);
    SceneDock::CreateTree(
      addDock, "scene-hierarchy", m_scene, m_selection, indent);

    ViewDock::Create(
      addDock, "scene-view", m_scene, m_env, m_view, m_settings, m_selection);

    ViewDock::Create(
      addDock, "T-Pose", m_scene, m_env, m_view, m_settings, m_selection);

    VrmDock::CreateVrm(addDock, "vrm", m_scene->m_table);
    ExportDock::Create(addDock, "export", m_scene, indent);
  }

  return m_scene;
}

LogStream
App::Log(LogLevel level)
{
  return {
    m_logger->Begin(level),
    [logger = m_logger]() {
      // flush
      logger->End();
    },
  };
}

void
App::LoadImGuiIni(std::string_view ini)
{
  m_gui->LoadState(ini);
}

void
App::SetWindowSize(int width, int height, bool maximize)
{
  m_platform->SetWindowSize(width, height, maximize);
}

void
App::SaveState()
{
  std::ofstream os((const char*)m_ini.u8string().c_str());

  os << "-- This file is auto generated ini file.\n\n";

  // imnodes
  os << "vrmeditor.imnodes_load_ini [===[\n"
     << PoseStream->Save() << "\n]===]\n\n";

  for (auto& link : PoseStream->Links) {
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

  auto [width, height] = m_platform->WindowSize();
  auto maximize = m_platform->IsWindowMaximized();
  os << "vrmeditor.set_window_size(" << width << ", " << height << ", "
     << (maximize ? "true" : "false") << ")\n\n";
}

bool
App::WriteScene(const std::filesystem::path& path)
{
  libvrm::gltf::Exporter exporter;
  exporter.Export(*m_scene->m_table);

  std::ofstream os(path, std::ios::binary);
  if (!os) {
    return false;
  }

  return libvrm::gltf::Glb{
    .JsonChunk = exporter.JsonChunk,
    .BinChunk = exporter.BinChunk,
  }
    .WriteTo(os);
}

bool
App::LoadPath(const std::filesystem::path& path)
{
  auto extension = path.extension().string();
  std::transform(
    extension.begin(), extension.end(), extension.begin(), tolower);

  if (extension == ".gltf" || extension == ".glb" || extension == ".vrm" ||
      extension == ".vrma") {
    return LoadModel(path);
  }
  if (extension == ".bvh") {
    return PoseStream->LoadMotion(path);
  }
  // if (extension == ".fbx") {
  // }
  return false;
}

bool
App::LoadModel(const std::filesystem::path& path)
{
  if (auto table = libvrm::gltf::LoadPath(path)) {
    auto scene = SetScene(*table);
    // bind time line

    for (auto& animation : scene->m_table->m_animations) {
      auto track = m_timeline->AddTrack("gltf", animation->Duration());
      std::weak_ptr<runtimescene::RuntimeScene> weak = scene;
      track->Callbacks.push_back([animation, weak](auto time, bool repeat) {
        if (auto scene = weak.lock()) {
          runtimescene::AnimationUpdate(
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
    m_view->Fit(bb.Min, bb.Max);

    Log(LogLevel::Info) << path;

    return true;
  } else {
    Log(LogLevel::Error) << table.error();
    SetScene(std::make_shared<libvrm::gltf::Scene>());
    return false;
  }
}

void
App::LoadLua(const std::filesystem::path& path)
{
  Log(LogLevel::Info) << path;
  if (auto ret = m_lua->DoFile(path)) {
    // ok
  } else {
    Log(LogLevel::Error) << ret.error();
  }
}

bool
App::AddAssetDir(std::string_view name, const std::filesystem::path& path)
{
  if (!std::filesystem::is_directory(path)) {
    return false;
  }

  auto asset = std::make_shared<AssetDir>(name, path);
  asset->Update();
  m_assets.push_back(asset);

  auto callback = [this](const std::filesystem::path& path) {
    if (LoadPath(path)) {
      m_platform->SetTitle(path.string());
    } else {
    }
  };

  auto dock = m_assets.back()->CreateDock(callback);
  m_gui->AddDock(dock);

  Log(LogLevel::Info) << name << " => " << path;
  return true;
}

int
App::Run()
{
  // must after App::App
  m_lua->DoFile(m_ini);

  auto addDock = [gui = m_gui](const Dock& dock) { gui->AddDock(dock); };

  ImTimeline::Create(addDock, "timeline", m_timeline);
  ImLogger::Create(addDock, "logger", m_logger);
  glr::CreateDock(addDock, "OpenGL resource");
  ViewDock::CreateSetting(addDock, "view-settings", m_env, m_view, m_settings);

  PoseStream->CreateDock(addDock);

  std::optional<libvrm::Time> lastTime;
  while (auto info = m_platform->NewFrame()) {
    auto time = info->Time;

    if (lastTime) {
      auto delta = time - *lastTime;
      m_timeline->SetDeltaTime(delta);
    } else {
      m_timeline->SetDeltaTime({}, true);
    }
    PoseStream->Update(time);
    lastTime = time;

    // newFrame
    if (m_gui->NewFrame()) {
      SaveState();
    }
    ImGuizmo::BeginFrame();

    m_gui->DockSpace();

    glViewport(0, 0, info->Width, info->Height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gui->Render();
    m_platform->Present();
  }

  SaveState();

  return 0;
}

void
App::ShowDock(std::string_view name, bool visible)
{
  m_gui->SetDockVisible(name, visible);
}
