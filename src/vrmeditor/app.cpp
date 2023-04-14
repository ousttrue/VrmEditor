#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <GL/glew.h>

#include "app.h"
#include "assetdir.h"
#include "docks/cuber.h"
#include "docks/gui.h"
#include "docks/humanoid_dock.h"
#include "docks/imlogger.h"
#include "docks/imtimeline.h"
#include "docks/json_dock.h"
#include "docks/motion_dock.h"
#include "docks/scene_dock.h"
#include "docks/view_dock.h"
#include "docks/vrm_dock.h"
#include "luahost.h"
#include "platform.h"
#include "udp_receiver.h"
#include <vrm/animation.h>
#include <vrm/bvh.h>
#include <vrm/bvhscene.h>
#include <vrm/fileutil.h>
#include <vrm/node.h>
#include <vrm/srht_update.h>
#include <vrm/timeline.h>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

void
App::HumanBoneMap::Add(std::string_view joint_name, std::string_view bone_name)
{
  if (auto bone = libvrm::vrm::HumanBoneFromName(
        bone_name, libvrm::vrm::VrmVersion::_1_0)) {
    NameBoneMap[{ joint_name.begin(), joint_name.end() }] = *bone;
  } else {
    NoBoneList.insert({ joint_name.begin(), joint_name.end() });
  }
}

bool
App::HumanBoneMap::Match(const libvrm::bvh::Bvh& bvh) const
{
  for (auto& joint : bvh.joints) {
    auto found = NameBoneMap.find(joint.name);
    if (found == NameBoneMap.end()) {
      if (NoBoneList.find(joint.name) != NoBoneList.end()) {
        // no bone joint
        continue;
      }
      return false;
    }
  }
  return true;
}

App::App()
{
  m_logger = std::make_shared<ImLogger>();
  m_lua = std::make_shared<LuaEngine>();
  m_scene = std::make_shared<libvrm::gltf::Scene>();
  m_view = std::make_shared<grapho::OrbitView>();
  m_timeline = std::make_shared<libvrm::Timeline>();
  m_motion = std::make_shared<libvrm::gltf::Scene>();
  m_udp = std::make_shared<UdpReceiver>();

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
  m_cuber = std::make_shared<Cuber>();

  cuber::PushGrid(libvrm::gizmo::lines());
  libvrm::gizmo::fix();

  // retarget human pose
  m_motion->m_sceneUpdated.push_back(
    [dst = m_scene,
     humanBoneMap = std::vector<libvrm::vrm::HumanBones>(),
     rotations = std::vector<DirectX::XMFLOAT4>()](
      const libvrm::gltf::Scene& src) mutable {
      humanBoneMap.clear();
      rotations.clear();
      for (auto& node : src.m_nodes) {
        if (auto humanoid = node->Humanoid) {
          humanBoneMap.push_back(humanoid->HumanBone);
          rotations.push_back(node->Transform.Rotation);
        }
      }

      if (src.m_roots.size()) {
        auto& hips = src.m_roots[0]->Transform.Translation;
        dst->SetHumanPose({ .RootPosition = { hips.x, hips.y, hips.z },
                            .Bones = humanBoneMap,
                            .Rotations = rotations });
      }
    });

  // bind motion to scene
  m_motion->m_sceneUpdated.push_back(
    [cuber = m_cuber](const libvrm::gltf::Scene& scene) {
      cuber->Instances.clear();
      if (scene.m_roots.size()) {
        scene.m_roots[0]->UpdateShapeInstanceRecursive(
          DirectX::XMMatrixIdentity(), cuber->Instances);
      }
    });
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

void
App::ClearMotion()
{
  m_motion->Clear();
  m_cuber->Instances.clear();
  m_timeline->Tracks.clear();
}

bool
App::LoadMotion(const std::filesystem::path& path)
{
  ClearMotion();

  auto bytes = libvrm::fileutil::ReadAllBytes<uint8_t>(path);
  if (bytes.empty()) {
    Log(LogLevel::Error) << "fail to read: " + path.string();
    return false;
  }

  // load bvh
  auto bvh = std::make_shared<libvrm::bvh::Bvh>();
  if (auto parsed = bvh->Parse({ (const char*)bytes.data(), bytes.size() })) {
  } else {
    Log(LogLevel::Error) << "LoadMotion: " << path;
    Log(LogLevel::Error) << "LoadMotion: " << parsed.error();
    return false;
  }
  auto scaling = bvh->GuessScaling();
  Log(LogLevel::Info) << "LoadMotion: " << scaling << ", " << path;

  libvrm::bvh::SetBvh(m_motion, bvh);
  m_motion->m_roots[0]->UpdateShapeInstanceRecursive(
    DirectX::XMMatrixIdentity(), m_cuber->Instances);

  // bind time to motion
  auto track = m_timeline->AddTrack("bvh", bvh->Duration());
  track->Callbacks.push_back([bvh, scene = m_motion](auto time, bool repeat) {
    if (scene->m_roots.size()) {
      libvrm::bvh::ResolveFrame(scene, bvh, time);
      scene->m_roots[0]->CalcWorldMatrix(true);
      scene->RaiseSceneUpdated();
    }
  });

  if (auto map = FindHumanBoneMap(*bvh)) {
    // assign human bone
    for (auto& node : m_motion->m_nodes) {
      auto found = map->NameBoneMap.find(node->Name);
      if (found != map->NameBoneMap.end()) {
        node->Humanoid = libvrm::gltf::NodeHumanoidInfo{
          .HumanBone = found->second,
        };
      }
    }

  } else {
    Log(LogLevel::Wran) << "humanoid map not found";
  }
  return true;
}

std::shared_ptr<App::HumanBoneMap>
App::FindHumanBoneMap(const libvrm::bvh::Bvh& bvh) const
{
  for (auto& map : m_humanBoneMapList) {
    if (map->Match(bvh)) {
      return map;
    }
  }
  return {};
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

  {
    JsonDock::Create(addDock, "gltf-json", m_scene, indent);
    HumanoidDock::Create(addDock, "humanoid-body", "humanoid-finger", m_scene);
    auto selection =
      SceneDock::CreateTree(addDock, "scene-hierarchy", m_scene, indent);
    ViewDock::Create(addDock,
                     "scene-view",
                     m_scene,
                     selection,
                     m_view,
                     m_timeline,
                     m_renderer);
    VrmDock::CreateVrm(addDock, "vrm", m_scene);
    VrmDock::CreateExpression(addDock, "expression", m_scene);
  }

  {
    HumanoidDock::Create(addDock, "motion-body", "motion-finger", m_motion);
    auto selection =
      SceneDock::CreateTree(addDock, "motion-hierarchy", m_motion, indent);

    auto callback = [scene = m_motion,
                     cuber = m_cuber](std::span<const uint8_t> data) {
      // udp update m_motion scene
      libvrm::srht::UpdateScene(scene, cuber->Instances, data);

      if (scene->m_roots.size()) {
        scene->m_roots[0]->CalcWorldMatrix(true);
        scene->RaiseSceneUpdated();
      }
    };

    MotionDock::Create(
      addDock,
      "motion",
      m_cuber,
      selection,
      [this, callback]() {
        ClearMotion();
        m_udp->Start(54345, callback);
      },
      [udp = m_udp]() { udp->Stop(54345); });
  }

  ImTimeline::Create(addDock, "timeline", m_timeline);
  ImLogger::Create(addDock, "logger", m_logger);

  std::optional<libvrm::Time> lastTime;
  while (auto info = m_platform->NewFrame()) {
    auto time = info->Time;

    m_udp->Update();

    libvrm::gizmo::clear();
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