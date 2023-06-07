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

const auto WINDOW_TITLE = "VrmEditor";

App::App()
{
  m_watcher = std::make_shared<FileWatcher>(
    [=](const std::filesystem::path& path) { this->OnFileUpdated(path); });

  m_lua = std::make_shared<LuaEngine>();
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

  m_platform = std::make_shared<Platform>();
  auto window = m_platform->CreateWindow(2000, 1200, false, WINDOW_TITLE);
  if (!window) {
    throw std::runtime_error("createWindow");
  }
  GL_ErrorClear("CreateWindow");

  glr::Initialize();

  PoseStream = std::make_shared<humanpose::HumanPoseStream>();
  m_gui = std::make_shared<Gui>(window, m_platform->glsl_version.c_str());
  m_gl3gui = std::make_shared<glr::Gl3RendererGui>();

  auto track = m_timeline->AddTrack("PoseStream", {});
  track->Callbacks.push_back([pose = PoseStream](auto time, auto repeat) {
    pose->Update(time);
    return true;
  });

  // glEnable(GL_DEPTH_TEST);
  // // set depth function to less than AND equal for skybox depth trick.
  // glDepthFunc(GL_LEQUAL);
  // // enable seamless cubemap sampling for lower mip levels in the pre-filter
  // // map.
  // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  m_json = std::make_shared<JsonGui>();
}

App::~App() {}

void
App::ProjectMode()
{
  m_gui->DarkMode();
}

std::shared_ptr<libvrm::RuntimeScene>
App::SetScene(const std::shared_ptr<libvrm::GltfRoot>& table)
{
  glr::Release();
  m_runtime = std::make_shared<libvrm::RuntimeScene>(table);
  m_timeline->Tracks.clear();

  std::weak_ptr<libvrm::RuntimeScene> weak = m_runtime;
  PoseStream->HumanPoseChanged.push_back([weak](const auto& pose) {
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
  if (extension == ".hdr") {
    return LoadPbr(path);
  }
  return false;
}

bool
App::LoadModel(const std::filesystem::path& path)
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

bool
App::LoadPbr(const std::filesystem::path& hdr)
{
  return glr::LoadPbr_LOGL(hdr);
}

void
App::LoadLua(const std::filesystem::path& path)
{
  PLOG_INFO << path.string();
  if (auto ret = m_lua->DoFile(path)) {
    // ok
  } else {
    PLOG_ERROR << ret.error();
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

  PLOG_INFO << name << " => " << path.string();
  return true;
}

int
App::Run()
{
  // Create the main instance of Remotery.
  // You need only do this once per program.
  // Remotery* rmt;
  // rmt_CreateGlobalInstance(&rmt);

  GL_ErrorClear("CreateWindow");

  // must after App::App
  m_lua->DoFile(m_ini);

  auto addDock = [gui = m_gui](const grapho::imgui::Dock& dock) {
    gui->AddDock(dock);
  };

#ifndef NDEBUG
  ImTimeline::Create(addDock, "[animation] timeline", m_timeline);
  PoseStream->CreateDock(addDock, "[animation] input-stream");
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

  addDock({
    "hierarchy",
    [hierarchy = m_hierarchy]() { hierarchy->ShowGui(); },
  });

  std::optional<libvrm::Time> lastTime;
  while (auto info = m_platform->NewFrame()) {
    {
      // rmt_ScopedCPUSample(update, 0);

      ERROR_CHECK;

      m_watcher->Update();

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
    }

    {
      // rmt_ScopedCPUSample(gui, 0);
      m_gui->DockSpace();
    }

    {
      // rmt_ScopedCPUSample(render, 0);
      glr::ClearBackBuffer(info->Width, info->Height);

      m_gui->Render();
      m_platform->Present();
    }
  }

  SaveState();

  // Destroy the main instance of Remotery.
  // rmt_DestroyGlobalInstance(rmt);

  return 0;
}

void
App::ShowDock(std::string_view name, bool visible)
{
  m_gui->SetDockVisible(name, visible);
}

void
App::SetShaderDir(const std::filesystem::path& path)
{
  m_shaderDir = path;
  m_watcher->Watch(path);
  glr::SetShaderDir(path);
}

void
App::SetShaderChunkDir(const std::filesystem::path& path)
{
  glr::SetShaderChunkDir(path);
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
App::OnFileUpdated(const std::filesystem::path& path)
{
  if (auto rel = getRelative(m_shaderDir, path)) {
    glr::UpdateShader(*rel);
  }
}
