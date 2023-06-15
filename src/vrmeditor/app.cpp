#include <GL/glew.h>

#include "app.h"
#include "config.h"
#include "docks/asset_view.h"
#include "docks/export_dock.h"
#include "docks/gl3renderer_gui.h"
#include "docks/gui.h"
#include "docks/hierarchy_gui.h"
#include "docks/humanoid_dock.h"
#include "docks/imlogger.h"
#include "docks/imtimeline.h"
#include "docks/lighting.h"
#include "docks/scene_selection.h"
#include "docks/vrm_dock.h"
#include "fbx_loader.h"
#include "filewatcher.h"
#include "fs_util.h"
#include "humanpose/humanpose_stream.h"
#include "jsongui/json_gui.h"
#include "luahost.h"
#include "platform.h"
#include "view/scene_preview.h"
#include <ImGuizmo.h>
#include <cuber/mesh.h>
#include <fstream>
#include <glr/gl3renderer.h>
#include <glr/rendering_env.h>
#include <gltfjson.h>
#include <gltfjson/glb.h>
#include <gltfjson/json_tree_exporter.h>
#include <grapho/gl3/error_check.h>
#include <grapho/gl3/texture.h>
#include <grapho/orbitview.h>
#include <imgui.h>
#include <queue>
#include <vrm/animation_update.h>
#include <vrm/fileutil.h>
#include <vrm/gizmo.h>
#include <vrm/image.h>
#include <vrm/importer.h>
#include <vrm/timeline.h>
#ifdef _WIN32
#include "windows_helper.h"
#else
#endif

FileWatcher g_watcher;
std::filesystem::path g_shaderDir;

const auto WINDOW_TITLE = "VrmEditor - " PACKAGE_VERSION;

std::queue<app::Task> g_tasks;

std::filesystem::path g_ini;

class App
{
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::shared_ptr<JsonGui> m_json;
  std::shared_ptr<libvrm::Timeline> m_timeline;
  std::shared_ptr<ScenePreview> m_preview;
  std::shared_ptr<ScenePreview> m_animationPreview;
  std::shared_ptr<Lighting> m_lighting;

  // std::shared_ptr<HierarchyGui> m_hierarchy;
  // std::shared_ptr<struct SceneNodeSelection> m_selection;
  // std::shared_ptr<grapho::OrbitView> m_staticView;
  // std::shared_ptr<grapho::OrbitView> m_runtimeView;
  // std::shared_ptr<glr::ViewSettings> m_settings;
  std::shared_ptr<glr::RenderingEnv> m_env;

  // std::shared_ptr<glr::Gl3RendererGui> m_gl3gui;

public:
  App()
  {
    m_json = std::make_shared<JsonGui>();
    m_timeline = std::make_shared<libvrm::Timeline>();
    auto track = m_timeline->AddTrack("PoseStream", {});
    track->Callbacks.push_back([](auto time, auto repeat) {
      humanpose::HumanPoseStream::Instance().Update(time);
      return true;
    });
    m_env = std::make_shared<glr::RenderingEnv>();
    m_preview = std::make_shared<ScenePreview>(m_env);
    m_animationPreview = std::make_shared<ScenePreview>(m_env);
    m_lighting = std::make_shared<Lighting>();

    // m_selection = std::make_shared<SceneNodeSelection>();
    // m_hierarchy = std::make_shared<HierarchyGui>();
    // m_staticView = std::make_shared<grapho::OrbitView>();
    // m_runtimeView = std::make_shared<grapho::OrbitView>();
    // m_settings = std::make_shared<glr::ViewSettings>();
    // m_settings->ShowCuber = false;
    // m_gl3gui = std::make_shared<glr::Gl3RendererGui>();

    DockSpaceManager::Instance().OnResetCallbacks.push_back(
      [=] { ResetDock(); });
  }

  ~App() {}

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  void ResetDock()
  {
    // #ifndef NDEBUG
    //     ImTimeline::Create(addDock, "[animation] timeline", m_timeline);
    //     humanpose::HumanPoseStream::Instance().CreateDock(
    //       addDock, "[animation] input-stream");
    // #endif

    DockSpaceManager::Instance()
      .AddDock({
        "📜logger",
        []() { ImLogger::Instance().Draw(); },
      })
      .SetVisible(false);

    {
      // addDock(
      //   grapho::imgui::Dock("GL impl", [=]() { m_gl3gui->ShowSelectImpl();
      //   }));

      // addDock(grapho::imgui::Dock("GL selector", [=]() {
      //   //
      //   m_gl3gui->ShowSelector();
      // }));

      // addDock(grapho::imgui::Dock("GL selected shader source", [=]() {
      //   //
      //   m_gl3gui->ShowSelectedShaderSource();
      // }));
      //
      // addDock(grapho::imgui::Dock("GL selected shader variables", [=]() {
      //   //
      //   m_gl3gui->ShowSelectedShaderVariables();
      // }));
    }

    DockSpaceManager::Instance().AddDock({
      app::DOCKNAME_JSON,
      [json = m_json]() mutable { json->ShowSelector(); },
    });
    DockSpaceManager::Instance().AddDock({
      app::DOCKNAME_VIEW,
      [preview = m_preview]() { preview->ShowGui(); },
    });
    DockSpaceManager::Instance()
      .AddDock({
        "🎥Pose",
        [runtime = m_animationPreview]() { runtime->ShowGui(); },
      })
      .SetVisible(false);

    DockSpaceManager::Instance().AddDock(
      { "💡Lighting", [lighting = m_lighting]() { lighting->ShowGui(); } });

    // addDock({
    //   "hierarchy",
    //   [hierarchy = m_hierarchy]() { hierarchy->ShowGui(); },
    // });
  }

  std::shared_ptr<libvrm::RuntimeScene> SetGltf(
    const std::shared_ptr<libvrm::GltfRoot>& gltf)
  {
    glr::Release();
    m_runtime = std::make_shared<libvrm::RuntimeScene>(gltf);
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

    m_json->SetScene(gltf);
    m_lighting->SetGltf(gltf);
    m_preview->SetGltf(gltf);
    m_animationPreview->SetRuntime(m_runtime);
    return m_runtime;
  }

  // HumanoidDock::Create(
  //   addDock, "humanoid-body", "humanoid-finger", m_runtime->m_table);

  // ViewDock::CreateTPose(addDock,
  //                       "3D-View",
  //                       m_runtime->m_table,
  //                       m_env,
  //                       m_staticView,
  //                       m_settings,
  //                       m_selection);
  // static void CreateTPose(const AddDockFunc& addDock,
  //                          std::string_view title,
  //                          const std::shared_ptr<libvrm::GltfRoot>&
  //                          table, const
  //                          std::shared_ptr<glr::RenderingEnv>& env, const
  //                          std::shared_ptr<grapho::OrbitView>& view,
  //                          const std::shared_ptr<glr::ViewSettings>&
  //                          settings, const
  //                          std::shared_ptr<SceneNodeSelection>&
  //                          selection)
  //   {
  //   auto preview =
  //     ScenePreview::Create(table, env, view, settings, selection);
  //
  //   addDock({
  //     .Name = { title.begin(), title.end() },
  //     .OnShow =
  //       [preview, table, settings]() {
  //         preview->ShowFullWindow(table->m_title.c_str(),
  //         settings->Color);
  //       },
  //     .Flags =
  //       ImGuiWindowFlags_NoScrollbar |
  //       ImGuiWindowFlags_NoScrollWithMouse,
  //     .StyleVars = { { ImGuiStyleVar_WindowPadding, { 0, 0 } } },
  //   });
  // }

  //     ViewDock::Create(addDock,
  //                      "Runtime-View",
  //                      m_runtime,
  //                      m_env,
  //                      m_runtimeView,
  //                      m_settings,
  //                      m_selection);
  // static void Create(const AddDockFunc& addDock,
  //                    std::string_view title,
  //                    const std::shared_ptr<libvrm::RuntimeScene>& scene,
  //                    const std::shared_ptr<glr::RenderingEnv>& env,
  //                    const std::shared_ptr<grapho::OrbitView>& view,
  //                    const std::shared_ptr<glr::ViewSettings>& settings,
  //                    const std::shared_ptr<SceneNodeSelection>&
  //                    selection)
  // {
  //   auto preview = ScenePreview::Create(scene, env, view, settings,
  //   selection);
  //
  //   addDock({
  //     .Name = { title.begin(), title.end() },
  //     .OnShow =
  //       [preview, scene, settings]() {
  //         preview->ShowFullWindow(scene->m_table->m_title.c_str(),
  //                                 settings->Color);
  //       },
  //     .Flags =
  //       ImGuiWindowFlags_NoScrollbar |
  //       ImGuiWindowFlags_NoScrollWithMouse,
  //     .StyleVars = { { ImGuiStyleVar_WindowPadding, { 0, 0 } } },
  //   });
  // }

  // VrmDock::CreateVrm(addDock, "vrm", m_runtime);

  // #ifndef NDEBUG
  //       ExportDock::Create(addDock, "[debug] export", m_runtime);
  // #endif

  // m_hierarchy->SetRuntimeScene(m_runtime);

  void SaveState()
  {
    std::ofstream os((const char*)g_ini.u8string().c_str());

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
    for (auto& dock : DockSpaceManager::Instance().Docks) {
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

  int Run(GLFWwindow* window)
  {
    glr::Initialize();
    Gui::Instance().SetWindow(window,
                              Platform::Instance().glsl_version.c_str());

    // Create the main instance of Remotery.
    // You need only do this once per program.
    // Remotery* rmt;
    // rmt_CreateGlobalInstance(&rmt);
    ResetDock();

    assert(!grapho::gl3::TryGetError());
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
        assert(!grapho::gl3::TryGetError());

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
        DockSpaceManager::Instance().ShowGui();
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
    if (extension == ".fbx") {
      return LoadFbx(path);
    }
    if (extension == ".hdr") {
      return LoadHdr(path);
    }
    if (extension == ".png" || extension == ".jpg" || extension == ".gif") {
      return LoadImage(path);
    }
    if (extension == ".txt" || extension == ".md") {
      return LoadText(path);
    }

    PLOG_WARNING << "unknown tpe: " << path.string();
    return false;
  }

  bool LoadImage(const std::filesystem::path& path)
  {
    auto bytes = libvrm::ReadAllBytes(path);
    if (bytes.empty()) {
      return false;
    }

    auto image = std::make_shared<libvrm::Image>(path.filename().string());
    if (!image->Load(bytes)) {
      return false;
    }

    auto texture = glr::CreateTexture(image);
    if (!texture) {
      return false;
    }
    DockSpaceManager::Instance().AddDock(
      { std::string("🖼") + path.filename().string(),
        [texture, width = image->Width(), height = image->Height()]() {
          auto size = ImGui::GetContentRegionAvail();
          auto w = size.x;
          auto x = size.x / width;
          auto y = size.y / height;
          if (x < y) {
            // fit horizontal
            size.y = height * x;
          } else {
            // fit vertical
            size.x = width * y;
          }
          //
          ImGui::SetCursorPosX((w - size.x) / 2);
          ImGui::Image((ImTextureID)(intptr_t)texture->Handle(), size);
        } },
      true);

    return true;
  }

  bool LoadText(const std::filesystem::path& path)
  {
    auto bytes = libvrm::ReadAllBytes(path);
    if (bytes.empty()) {
      return false;
    }

    DockSpaceManager::Instance().AddDock(
      { std::string("📄") + path.filename().string(),
        [text = std::string((const char*)bytes.data(), bytes.size())]() {
          //
          ImGui::TextWrapped("%s", text.c_str());
        } },
      true);
    return true;
  }

  bool LoadModel(const std::filesystem::path& path)
  {
    if (auto gltf = libvrm::LoadPath(path)) {
      auto scene = SetGltf(*gltf);
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
      // m_staticView->Fit(bb.Min, bb.Max);
      // m_runtimeView->Fit(bb.Min, bb.Max);
      // m_env->SetShadowHeight(bb.Min.y);

      PLOG_INFO << path.string();

      return true;
    } else {
      PLOG_ERROR << gltf.error();
      SetGltf(std::make_shared<libvrm::GltfRoot>());
      return false;
    }
  }

  bool LoadFbx(const std::filesystem::path& path)
  {
    FbxLoader fbx;
    if (auto gltf = fbx.Load(path)) {
      PLOG_DEBUG << "ufbx success: " << path.string();
      auto scene = SetGltf(gltf);
      return false;
    } else {
      PLOG_ERROR << fbx.Error();
      return false;
    }
  }

  bool LoadHdr(const std::filesystem::path& path)
  {
    return m_lighting->LoadHdr(path);
  }

  bool AddAssetDir(std::string_view name, const std::filesystem::path& path)
  {
    if (!std::filesystem::is_directory(path)) {
      return false;
    }

    auto asset = std::make_shared<AssetView>(name, path);
    asset->Reload();

    auto title = std::string("🎁") + std::string{ name.data(), name.size() };
    DockSpaceManager::Instance().AddDock(
      { title, [asset]() { asset->ShowGui(); } }, true);

    PLOG_INFO << title << " => " << path.string();
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
TaskLoadModel(const std::filesystem::path& path)
{
  PostTask([path]() { g_app.LoadModel(path); });
}

void
TaskLoadPath(const std::filesystem::path& path)
{
  PostTask([path]() { g_app.LoadPath(path); });
}

void
TaskLoadHdr(const std::filesystem::path& hdr)
{
  PostTask([hdr]() { g_app.LoadHdr(hdr); });
}

void
SetShaderDir(const std::filesystem::path& path)
{
  // g_app.SetShaderDir(path);
  g_shaderDir = path;
  g_watcher.Watch(path);
  glr::SetShaderDir(path);
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
  auto file = get_home() / ".vrmeditor.ini.lua";
  g_ini = file.u8string();
  LuaEngine::Instance().DoFile(g_ini);

  auto exe = GetExe();
  auto base = exe.parent_path().parent_path();
  app::SetShaderDir(base / "shaders");
  glr::SetShaderChunkDir(base / "threejs_shader_chunks");

  // load user ~/.vrmeditor.lua
  auto user_conf = get_home() / ".vrmeditor.lua";
  if (std::filesystem::exists(user_conf)) {
    LuaEngine::Instance().DoFile(user_conf);
  }

  for (auto _arg : args) {
    std::string_view arg = _arg;
    if (arg.ends_with(".lua")) {
      // prooject mode
      Gui::Instance().DarkMode();
      LuaEngine::Instance().DoFile(arg);
    } else {
      // viewermode
      TaskLoadModel(arg);
      std::filesystem::path path(arg);
      AddAssetDir("current", path.parent_path());
    }
    // LoadPath(arg);
  }

  g_watcher.AddCallback([](const std::filesystem::path& path) {
    if (auto rel = getRelative(g_shaderDir, path)) {
      glr::UpdateShader(*rel);
    }
  });

  auto window = Platform::Instance().WindowCreate(WINDOW_TITLE);
  if (!window) {
    throw std::runtime_error("createWindow");
  }
  assert(!grapho::gl3::TryGetError());
  Platform::Instance().OnDrops.push_back(
    [](auto& path) { g_app.LoadPath(path); });

  g_app.Run(window);
}

bool
WriteScene(const std::filesystem::path& path)
{
  return g_app.WriteScene(path);
}

bool
AddAssetDir(std::string_view name, const std::filesystem::path& path)
{
  return g_app.AddAssetDir(name, path);
}

} // namespace
