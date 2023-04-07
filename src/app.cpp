#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <GL/glew.h>

#include "app.h"
#include "assetdir.h"
#include "docks/gui.h"
#include "docks/imlogger.h"
#include "docks/imtimeline.h"
#include "docks/json_dock.h"
#include "docks/motion_dock.h"
#include "docks/scene_dock.h"
#include "luahost.h"
#include "platform.h"
#include <vrm/animation.h>
#include <vrm/bvhsolver.h>
#include <vrm/bvhsource.h>
#include <vrm/node.h>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

void
App::HumanBoneMap::Add(std::string_view joint_name, std::string_view bone_name)
{
  if (auto bone = vrm::HumanBoneFromName(bone_name, vrm::VrmVersion::_1_0)) {
    NameBoneMap[{ joint_name.begin(), joint_name.end() }] = *bone;
  } else {
    std::cout << bone_name << " not found" << std::endl;
  }
}

App::App()
{
  m_logger = std::make_shared<ImLogger>();
  m_lua = std::make_shared<LuaEngine>();
  m_scene = std::make_shared<gltf::Scene>();
  m_view = std::make_shared<OrbitView>();
  m_timeline = std::make_shared<Timeline>();
  m_motion = std::make_shared<bvh::MotionSource>(m_scene);

  m_platform = std::make_shared<Platform>();
  auto window =
    m_platform->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    throw std::runtime_error("createWindow");
  }

  Log(LogLevel::Info) << "GL_VERSION: " << glGetString(GL_VERSION);
  Log(LogLevel::Info) << "GL_VENDOR: " << glGetString(GL_VENDOR);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("glewInit");
  }

  m_gui = std::make_shared<Gui>(window, m_platform->glsl_version.c_str());
  m_renderer = std::make_shared<Gl3Renderer>();

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
    return LoadMotion(path);
  }
  // if (extension == ".fbx") {
  // }
  return false;
}

bool
App::LoadModel(const std::filesystem::path& path)
{
  m_renderer->Release();
  m_scene->Clear();
  if (auto result = m_scene->LoadPath(path)) {
    // bind time line
    for (auto& animation : m_scene->m_animations) {
      auto track = m_timeline->AddTrack("gltf", animation->Duration());
      track->Callbacks.push_back(
        [animation, scene = m_scene](auto time, bool repeat) {
          animation->Update(time, scene->m_nodes, repeat);
        });

      // first animation only
      break;
    }

    // update view position
    auto bb = m_scene->GetBoundingBox();
    m_view->Fit(bb.Min, bb.Max);

    Log(LogLevel::Info) << path;

    return true;
  } else {
    Log(LogLevel::Error) << result.error();
    return false;
  }
}

bool
App::LoadMotion(const std::filesystem::path& path, float scaling)
{
  Log(LogLevel::Info) << path;
  if (!m_motion->LoadMotion(path, scaling, m_timeline)) {
    return false;
  }

  // search human bone map
  for (auto& map : m_humanBoneMapList) {
    for (auto& node : m_motion->MotionSolver->Scene->m_nodes) {
      auto found = map->NameBoneMap.find(node->Name);
      if (found != map->NameBoneMap.end()) {
        node->Humanoid = gltf::NodeHumanoidInfo{
          .HumanBone = found->second,
        };
      }
    }
  }

  return true;
}

void
App::LoadLua(const std::filesystem::path& path)
{
  Log(LogLevel::Info) << path;
  m_lua->dofile(path);
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
  m_gui->m_docks.push_back(dock);

  Log(LogLevel::Info) << name << " => " << path;
  return true;
}

std::shared_ptr<App::HumanBoneMap>
App::AddHumanBoneMap()
{
  auto ptr = std::make_shared<App::HumanBoneMap>();
  m_humanBoneMapList.push_back(ptr);
  return ptr;
}

int
App::Run()
{
  auto addDock = [gui = m_gui](const Dock& dock) {
    gui->m_docks.push_back(dock);
  };
  auto indent = m_gui->m_fontSize * 0.5f;
  JsonDock::Create(addDock, m_scene, indent);
  SceneDock::Create(addDock, m_scene, m_view, m_timeline, m_renderer, indent);
  MotionDock::Create(addDock, m_motion);
  ImTimeline::Create(addDock, m_timeline);
  ImLogger::Create(addDock, m_logger);

  std::optional<Time> lastTime;
  while (auto info = m_platform->NewFrame()) {
    auto time = info->Time;

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

    glViewport(0, 0, info->Width, info->Height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gui->Render();
    m_platform->Present();
  }

  return 0;
}
