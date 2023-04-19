#include "app.h"
#include "assetdir.h"
#include "docks/export_dock.h"
#include "docks/gui.h"
#include "docks/humanoid_dock.h"
#include "docks/imlogger.h"
#include "docks/imtimeline.h"
#include "docks/json_dock.h"
#include "docks/scene_dock.h"
#include "docks/view_dock.h"
#include "docks/vrm_dock.h"
#include "fs_util.h"
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
#include <vrm/timeline.h>

const auto WINDOW_TITLE = "VrmEditor";

App::App()
{
  m_logger = std::make_shared<ImLogger>();
  m_lua = std::make_shared<LuaEngine>();
  auto file = get_home() / ".vrmeditor.ini.lua";
  m_ini = file.u8string();

  m_view = std::make_shared<grapho::OrbitView>();
  m_timeline = std::make_shared<libvrm::Timeline>();

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

  auto track = m_timeline->AddTrack("PoseStream", {});
  track->Callbacks.push_back([pose = PoseStream](auto time, auto repeat) {
    pose->Update(time);
    return true;
  });
}

App::~App() {}

void
App::SetScene(const std::shared_ptr<libvrm::gltf::Scene>& scene)
{
  m_scene = scene;
  std::weak_ptr<libvrm::gltf::Scene> weak = scene;
  PoseStream->HumanPoseChanged.push_back([weak](const auto& pose) {
    if (auto scene = weak.lock()) {
      scene->SetHumanPose(pose);
      return true;
    } else {
      return false;
    }
  });

  auto addDock = [gui = m_gui](const Dock& dock) { gui->AddDock(dock); };
  auto indent = m_gui->m_fontSize * 0.5f;

  {
    JsonDock::Create(addDock, "gltf-json", m_scene, indent);
    HumanoidDock::Create(addDock, "humanoid-body", "humanoid-finger", m_scene);
    auto selection =
      SceneDock::CreateTree(addDock, "scene-hierarchy", m_scene, indent);
    ViewDock::Create(
      addDock, "scene-view", m_scene, selection, m_view, m_timeline);
    VrmDock::CreateVrm(addDock, "vrm", m_scene);
    VrmDock::CreateExpression(addDock, "expression", m_scene);
    ExportDock::Create(addDock, "export", m_timeline, m_scene, indent);
  }
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

  auto [width, height] = m_platform->WindowSize();
  auto maximize = m_platform->IsWindowMaximized();
  os << "vrmeditor.set_window_size(" << width << ", " << height << ", "
     << (maximize ? "true" : "false") << ")\n\n";
}

bool
App::WriteScene(const std::filesystem::path& path)
{
  libvrm::gltf::Exporter exporter;
  exporter.Export(*m_scene);

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
  if (auto scene = libvrm::gltf::Scene::LoadPath(path)) {
    SetScene(*scene);
    // bind time line
    for (auto& animation : (*scene)->m_animations) {
      auto track = m_timeline->AddTrack("gltf", animation->Duration());
      std::weak_ptr<libvrm::gltf::Scene> weak = *scene;
      track->Callbacks.push_back([animation, weak](auto time, bool repeat) {
        if (auto scene = weak.lock()) {
          animation->Update(time, scene->m_nodes, repeat);
          return true;
        } else {
          return false;
        }
      });

      // first animation only
      break;
    }

    // update view position
    auto bb = (*scene)->GetBoundingBox();
    m_view->Fit(bb.Min, bb.Max);

    Log(LogLevel::Info) << path;

    return true;
  } else {
    Log(LogLevel::Error) << scene.error();
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

  auto addDock = [gui = m_gui](const Dock& dock) {
    gui->AddDock(dock);
  };

  ImTimeline::Create(addDock, "timeline", m_timeline);
  ImLogger::Create(addDock, "logger", m_logger);
  glr::CreateDock(addDock, "OpenGL resource");

  PoseStream->CreateDock(addDock);

  std::optional<libvrm::Time> lastTime;
  while (auto info = m_platform->NewFrame()) {
    auto time = info->Time;

    if (lastTime) {
      m_timeline->SetDeltaTime(time - *lastTime);
    } else {
      m_timeline->SetDeltaTime({}, true);
    }
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
