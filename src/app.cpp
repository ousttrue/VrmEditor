#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <GL/glew.h>

#include "app.h"
#include "assetdir.h"
#include "docks/gui.h"
#include "docks/json_dock.h"
#include "docks/motion_dock.h"
#include "docks/scene_dock.h"
#include "docks/imtimeline.h"
#include "imlogger.h"
#include "luahost.h"
#include "platform.h"
#include <Bvh.h>
#include <BvhSolver.h>
#include <fstream>
#include <imnodes.h>
#include <sstream>
#include <vrm/animation.h>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/scene.h>
#include <vrm/vrm0.h>
#include <vrm/vrm1.h>
#ifdef _WIN32
#include "windows_helper.h"
#endif

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

App::App()
{
  m_logger = std::make_shared<ImLogger>();
  m_lua = std::make_shared<LuaEngine>();
  m_scene = std::make_shared<Scene>();
  m_view = std::make_shared<OrbitView>();
  m_timeline = std::make_shared<Timeline>();
  m_motion = std::make_shared<MotionSource>();

  // pose to scene
  m_motion->PoseCallbacks.push_back([scene = m_scene](const vrm::HumanPose& pose) {
    scene->SetHumanPose(pose);
  });

  m_platform = std::make_shared<Platform>();
  auto window =
    m_platform->createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    throw std::runtime_error("createWindow");
  }

  Log(LogLevel::Info) << "GL_VERSION: " << glGetString(GL_VERSION);
  Log(LogLevel::Info) << "GL_VENDOR: " << glGetString(GL_VENDOR);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("glewInit");
  }

  m_gui = std::make_shared<Gui>(window, m_platform->glsl_version.c_str());

  cuber::PushGrid(gizmo::lines());
  gizmo::fix();
}

App::~App() {}

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
App::ClearScene()
{
  m_scene->Clear();
}

bool
App::WriteScene(const std::filesystem::path& path)
{
  auto bytes = m_scene->ToGlb();
  if (bytes.size()) {
    std::ofstream ofs(path);
    if (ofs) {
      ofs.write((const char*)bytes.data(), bytes.size());
      return true;
    }
  }
  return false;
}

bool
App::LoadModel(const std::filesystem::path& path)
{
  if (auto result = m_scene->Load(path)) {
    // bind time line
    for (auto& animation : m_scene->m_animations) {
      auto track = m_timeline->AddTrack("gltf", animation->duration());
      track->Callbacks.push_back(
        [animation, scene = m_scene](auto time, bool repeat) {
          animation->update(time, scene->m_nodes, repeat);
        });

      // first animation only
      break;
    }

    // update view position
    auto bb = m_scene->GetBoundingBox();
    m_view->Fit(bb.Min, bb.Max);

    return true;
  } else {
    Log(LogLevel::Error) << result.error();
    return false;
  }
}

bool
App::LoadMotion(const std::filesystem::path& path, float scaling)
{
  return m_motion->LoadMotion(path, scaling, m_timeline);
}

void
App::LoadLua(const std::filesystem::path& path)
{
  m_lua->dofile(path);
}

bool
App::AddAssetDir(std::string_view name, const std::filesystem::path& path)
{
  if (!std::filesystem::is_directory(path)) {
    return false;
  }

  m_assets.push_back(std::make_shared<AssetDir>(name, path));

  auto callback = [this](const std::filesystem::path& path) {
    if (path.extension().u8string().ends_with(u8".bvh")) {
      LoadMotion(path);
    } else {
      ClearScene();
      LoadModel(path);
    }
  };

  auto dock = m_assets.back()->CreateDock(callback);
  m_gui->m_docks.push_back(dock);

  return true;
}

int
App::Run()
{
  auto addDock = [gui = m_gui](const Dock& dock) {
    gui->m_docks.push_back(dock);
  };
  JsonDock::Create(addDock, m_scene);
  SceneDock::Create(addDock, m_scene, m_view, m_timeline);
  MotionDock::Create(addDock, m_motion);
  ImTimeline::Create(addDock, m_timeline);

  loggerDock();

  std::optional<Time> lastTime;
  while (auto info = m_platform->newFrame()) {
    auto time = info->time;

    gizmo::clear();
    if (lastTime) {
      m_timeline->SetDeltaTime(time - *lastTime);
    } else {
      m_timeline->SetDeltaTime({}, true);
    }
    lastTime = time;

    // newFrame
    m_gui->NewFrame();
    ImGuizmo::BeginFrame();

    m_gui->DockSpace();

    glViewport(0, 0, info->width, info->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gui->Render();
    m_platform->present();
  }

  return 0;
}

void
App::loggerDock()
{
  m_gui->m_docks.push_back(
    Dock("logger", [logger = m_logger]() { logger->Draw(); }));
}
