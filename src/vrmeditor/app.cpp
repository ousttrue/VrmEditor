#include <GL/glew.h>

#include "app.h"
#include "asio_task.h"
#include "config.h"
#include "docks/asset_view.h"
#include "docks/dockspace.h"
#include "docks/gui.h"
#include "docks/hierarchy_gui.h"
#include "docks/humanoid_dock.h"
#include "docks/imlogger.h"
#include "docks/vrm_gui.h"
#include "fbx_loader.h"
#include "filewatcher.h"
#include "fs_util.h"
#include "humanpose/humanpose_stream.h"
#include "jsongui/json_gui.h"
#include "luahost.h"
#include "platform.h"
#include "scene_state.h"
#include "view/animation_view.h"
#include "view/gl3renderer_gui.h"
#include "view/lighting.h"
#include "view/mesh_gui.h"
#include "view/scene_preview.h"
#include <boneskin/skinning_manager.h>
#include <glr/rendering_env.h>
#include <grapho/gl3/error_check.h>
#include <grapho/gl3/texture.h>
#include <vrm/fileutil.h>
#include <vrm/image.h>
#include <vrm/runtime_node.h>
#ifdef _WIN32
#include "windows_helper.h"
#else
#endif

FileWatcher g_watcher;
std::filesystem::path g_shaderDir;

const auto WINDOW_TITLE = "VrmEditor - " PACKAGE_VERSION;

std::filesystem::path g_ini;

class App
{
  std::shared_ptr<JsonGui> m_json;
  std::shared_ptr<ScenePreview> m_preview;
  std::shared_ptr<ScenePreview> m_animationPreview;
  std::shared_ptr<Lighting> m_lighting;
  std::shared_ptr<AnimationView> m_animation;
  std::shared_ptr<glr::RenderingEnv> m_env;
  std::shared_ptr<glr::Gl3RendererGui> m_gl3gui;
  std::shared_ptr<HierarchyGui> m_hierarchy;
  std::shared_ptr<VrmGui> m_vrm;
  std::shared_ptr<HumanoidDock> m_humanoid;
  std::shared_ptr<MeshGui> m_meshGui;

public:
  App()
  {
    m_json = std::make_shared<JsonGui>();
    m_env = std::make_shared<glr::RenderingEnv>();
    m_preview = std::make_shared<ScenePreview>(m_env);
    m_animationPreview = std::make_shared<ScenePreview>(m_env);
    m_lighting = std::make_shared<Lighting>();
    m_gl3gui = std::make_shared<glr::Gl3RendererGui>();
    m_animation = std::make_shared<AnimationView>();
    m_hierarchy = std::make_shared<HierarchyGui>();
    m_vrm =
      std::make_shared<VrmGui>([json = m_json](const std::u8string& jsonpath) {
        json->ClearCache(jsonpath);
      });
    m_humanoid = std::make_shared<HumanoidDock>();
    m_meshGui = std::make_shared<MeshGui>();

    DockSpaceManager::Instance().OnResetCallbacks.push_back(
      [=] { ResetDock(); });

    SceneState::GetInstance().m_setCallbacks.push_back(
      [=](const std::shared_ptr<libvrm::RuntimeScene>& runtime) {
        glr::Release();
        boneskin::SkinningManager::Instance().Release();

        std::weak_ptr<libvrm::RuntimeScene> weak = runtime;
        humanpose::HumanPoseStream::Instance().HumanPoseChanged.push_back(
          [weak](const auto& pose) {
            if (auto scene = weak.lock()) {
              scene->SetHumanPose(pose);
              return true;
            } else {
              return false;
            }
          });

        m_json->SetScene(runtime->m_base);
        m_lighting->SetGltf(runtime->m_base);
        m_animation->SetRuntime(runtime);
        m_hierarchy->SetRuntimeScene(runtime);
        m_preview->SetGltf(runtime->m_base);
        m_animationPreview->SetRuntime(runtime);
        m_vrm->SetRuntime(runtime);
        m_humanoid->SetRuntime(runtime);
      });
  }

  ~App() {}

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  void ResetDock()
  {
    DockSpaceManager::Instance().AddDock({
      "📜Log",
      []() { ImLogger::Instance().Draw(); },
    });

    DockSpaceManager::Instance()
      .AddDock(
        {
          app::DOCKNAME_JSON,
          [json = m_json]() mutable {
            json->ShowSelector(Gui::Instance().Indent());
          },
        },
        { .ShowDefault = true })
      .NoPadding();

    DockSpaceManager::Instance()
      .AddDock(
        {
          app::DOCKNAME_VIEW,
          [preview = m_preview]() { preview->ShowGui(); },
        },
        { .ShowDefault = true })
      .NoScrollBar()
      .NoPadding();

    DockSpaceManager::Instance()
      .AddDock({
        "🌳Hierarchy",
        [hierarchy = m_hierarchy]() { hierarchy->ShowGui(); },
      })
      .NoPadding();

    DockSpaceManager::Instance()
      .AddDock({
        "🎬RuntimeView",
        [runtime = m_animationPreview]() { runtime->ShowGui(); },
      })
      .NoScrollBar()
      .NoPadding();

    DockSpaceManager::Instance()
      .AddDock({
        "🎬Animation",
        [animation = m_animation]() { animation->ShowGui(); },
      })
      .NoPadding();

    DockSpaceManager::Instance()
      .AddDock({
        "💡Lighting",
        [lighting = m_lighting]() { lighting->ShowGui(); },
      })
      .NoPadding();

    // DockSpaceManager::Instance().AddDock({
    //   "🔍GL impl",
    //   [=]() { m_gl3gui->ShowSelectImpl(); },
    // });

    DockSpaceManager::Instance().AddDock({
      "🔍GL selector",
      [=]() {
        //
        m_gl3gui->ShowSelector();
      },
    });

    DockSpaceManager::Instance().AddDock({
      "🔍GL selected shader source",
      [=]() {
        //
        m_gl3gui->ShowSelectedShaderSource();
      },
    });

    DockSpaceManager::Instance().AddDock({
      "🔍GL selected shader variables",
      [=]() {
        //
        m_gl3gui->ShowSelectedShaderVariables();
      },
    });

    // ImTimeline::Create(addDock, "[animation] timeline", m_timeline);
    DockSpaceManager::Instance().AddDock(
      { "🏃Humanoid", [=]() { m_humanoid->ShowGui(); } });

    DockSpaceManager::Instance().AddDock(
      { "🏃Vrm", [vrm = m_vrm]() { vrm->ShowGui(); } });

    DockSpaceManager::Instance().AddDock(
      { "📐MeshAsset", [mesh = m_meshGui]() { mesh->ShowGui(); } });
  }

  // HumanoidDock::Create(
  //   addDock, "humanoid-body", "humanoid-finger", m_runtime->m_table);

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
      if (dock.IsOpen) {
        os << "vrmeditor.show_dock('" << dock.Name << "', true)\n";
      } else {
        os << "vrmeditor.show_dock('" << dock.Name << "', false)\n";
      }
    }
    os << "\n";

    auto [width, height] = Platform::Instance().WindowSize();
    auto maximize = Platform::Instance().IsWindowMaximized();
    os << "vrmeditor.set_window_size(" << width << ", " << height << ", "
       << (maximize ? "true" : "false") << ")\n\n";
  }

  bool LoadFbx(const std::filesystem::path& path)
  {
    FbxLoader fbx;
    if (auto gltf = fbx.Load(path)) {
      PLOG_DEBUG << "ufbx success: " << path.string();
      SceneState::GetInstance().SetGltf(gltf);
      return false;
    } else {
      PLOG_ERROR << fbx.Error();
      return false;
    }
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
    while (true) {

      AsioTask::Instance().Poll();

      auto info = Platform::Instance().NewFrame();
      if (!info) {
        break;
      }

      {
        // rmt_ScopedCPUSample(update, 0);
        assert(!grapho::gl3::TryGetError());

        g_watcher.Update();

        auto time = info->Time;

        SceneState::GetInstance().Update(time);

        humanpose::HumanPoseStream::Instance().Update(time);

        // newFrame
        if (Gui::Instance().NewFrame()) {
          SaveState();
        }
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

  // expose to lua
  bool LoadPath(const std::filesystem::path& path)
  {
    if (std::filesystem::is_directory(path)) {
      return AddAssetDir(path.stem().string(), path);
    }

    auto extension = path.extension().string();
    std::transform(
      extension.begin(), extension.end(), extension.begin(), tolower);

    if (extension == ".gltf" || extension == ".glb" || extension == ".vrm" ||
        extension == ".vci" || extension == ".vrma") {
      return SceneState::GetInstance().LoadModel(path);
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

    PLOG_WARNING << "unknown type: " << gltfjson::from_u8(path.u8string());
    return false;
  }

  bool LoadImage(const std::filesystem::path& path)
  {
    auto bytes = libvrm::ReadAllBytes(path);
    if (bytes.empty()) {
      return false;
    }

    libvrm::Image image(path.filename().string());
    if (!image.Load(bytes)) {
      return false;
    }

    auto texture = glr::CreateTexture(image);
    if (!texture) {
      return false;
    }
    DockSpaceManager::Instance().AddDock(
      { std::string("🖼") + path.filename().string(),
        [texture, width = image.Width(), height = image.Height()]() {
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
      { .ShowDefault = true, .Temporary = true });

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
      { .ShowDefault = true, .Temporary = true });
    return true;
  }

  bool LoadHdr(const std::filesystem::path& path)
  {
    return m_lighting->LoadHdr(path);
  }

  bool AddAssetDir(const std::string& name, const std::filesystem::path& path)
  {
    if (!std::filesystem::is_directory(path)) {
      return false;
    }

    auto asset = std::make_shared<AssetView>(name, path);
    auto title = std::string("🎁") + std::string{ name.data(), name.size() };
    DockSpaceManager::Instance().AddDock(
      { title, [asset]() { asset->ShowGui(); } },
      { .ShowDefault = true, .Temporary = true });

    PLOG_INFO << title << " => " << path.string();

    asset->ReloadAsync();

    // AsioTask::Instance().PostThreadTask<std::shared_ptr<AssetView>>(
    //   [name, path]() {
    //     return asset;
    //   },
    //   [name, path](auto asset) {
    //   });

    return true;
  }
};
App g_app;

namespace app {

void
TaskLoadModel(const std::filesystem::path& path)
{
  AsioTask::Instance().PostTask(
    [path]() { SceneState::GetInstance().LoadModel(path); });
}

void
TaskLoadPath(const std::filesystem::path& path)
{
  AsioTask::Instance().PostTask([path]() { g_app.LoadPath(path); });
}

void
TaskLoadHdr(const std::filesystem::path& hdr)
{
  AsioTask::Instance().PostTask([hdr]() { g_app.LoadHdr(hdr); });
}

void
LoadGltfString(const std::string& json)
{
  SceneState::GetInstance().LoadGltfString(json);
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
  return SceneState::GetInstance().WriteScene(path);
}

bool
AddAssetDir(std::string_view name, const std::filesystem::path& path)
{
  return g_app.AddAssetDir({ name.begin(), name.end() }, path);
}

std::string
CopyVrmPoseText()
{
  return SceneState::GetInstance().CopyVrmPoseText();
}

} // namespace
