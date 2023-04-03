#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <GL/glew.h>

#include "app.h"
#include "assetdir.h"
#include "gl3renderer.h"
#include "gui.h"
#include "imlogger.h"
#include "imtimeline.h"
#include "luahost.h"
#include "platform.h"
#include "rendertarget.h"
#include <Bvh.h>
#include <BvhSolver.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <fstream>
#include <sstream>
#include <vrm/animation.h>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/scene.h>
#include <vrm/vrm0.h>
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
  m_timeline->Tracks.clear();
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
  m_motion = Bvh::ParseFile(path);
  if (!m_motion) {
    return false;
  }

  m_motionSolver = std::make_shared<BvhSolver>();
  m_motionSolver->Initialize(m_motion);

  auto track = m_timeline->AddTrack("bvh", m_motion->Duration());
  track->Callbacks.push_back([this](auto time, bool repeat) {
    auto index = m_motion->TimeToIndex(time);
    auto frame = m_motion->GetFrame(index);
    m_motionSolver->ResolveFrame(frame);

    // apply vrm
    if (m_scene->m_vrm0) {
      auto& hips = m_motionSolver->instances_[0];
      m_scene->SetHumanPose(m_humanBoneMap,
                            { hips._41, hips._42, hips._43 },
                            m_motionSolver->localRotations);
    }
  });
  return true;
}

void
App::motionDock()
{
  auto rt = std::make_shared<RenderTarget>(std::make_shared<OrbitView>());
  rt->color[0] = 0.4f;
  rt->color[1] = 0.2f;
  rt->color[2] = 0.2f;
  rt->color[3] = 1.0f;

  auto cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
  auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  std::vector<grapho::LineVertex> lines;
  cuber::PushGrid(lines);

  rt->render = [this, cuber, liner, lines](const ViewProjection& camera) {
    if (m_motionSolver) {
      cuber->Render(camera.projection,
                    camera.view,
                    m_motionSolver->instances_.data(),
                    m_motionSolver->instances_.size());
    }
    liner->Render(camera.projection, camera.view, lines);
  };

  auto gl3r = std::make_shared<Gl3Renderer>();

  m_gui->m_docks.push_back(Dock("motion", [rt](bool* p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    if (ImGui::Begin("motion",
                     p_open,
                     ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoScrollWithMouse)) {
      auto pos = ImGui::GetWindowPos();
      pos.y += ImGui::GetFrameHeight();
      auto size = ImGui::GetContentRegionAvail();
      rt->show_fbo(pos.x, pos.y, size.x, size.y);
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }));
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
  jsonDock();
  sceneDock();
  timelineDock();
  motionDock();
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

struct TreeContext
{
  gltf::Node* selected = nullptr;
  gltf::Node* new_selected = nullptr;
};

void
App::sceneDock()
{
  //
  // scene tree
  //
  auto context = std::make_shared<TreeContext>();

  auto enter = [context](gltf::Node& node) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;

    if (node.children.empty()) {
      node_flags |=
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    }
    if (context->selected == &node) {
      node_flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool hasRotation = node.hasRotation();
    if (hasRotation) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
    }

    bool node_open = ImGui::TreeNodeEx(
      (void*)(intptr_t)node.index, node_flags, "%s", node.name.c_str());

    if (hasRotation) {
      ImGui::PopStyleColor();
    }

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      context->new_selected = &node;
    }

    return node.children.size() && node_open;
  };
  auto leave = []() { ImGui::TreePop(); };

  m_gui->m_docks.push_back(
    Dock("scene", [scene = m_scene, enter, leave, context]() {
      context->selected = context->new_selected;
      scene->Traverse(enter, leave);
    }));

  m_gui->m_docks.push_back(Dock("selected", [scene = m_scene, context]() {
    if (context->selected) {
      ImGui::Text("%s", context->selected->name.c_str());
      if (auto mesh_index = context->selected->mesh) {
        auto mesh = scene->m_meshes[*mesh_index];
        auto instance = context->selected->Instance;
        for (int i = 0; i < mesh->m_morphTargets.size(); ++i) {
          auto& morph = mesh->m_morphTargets[i];
          ImGui::SliderFloat(morph->name.c_str(), &instance->weights[i], 0, 1);
        }
      }
    }
  }));

  m_gui->m_docks.push_back(Dock("vrm-0.x", [scene = m_scene]() {
    if (auto vrm = scene->m_vrm0) {
      for (auto expression : vrm->m_expressions) {
        ImGui::SliderFloat(
          expression->label.c_str(), &expression->weight, 0, 1);
      }
    }
  }));

  //
  // 3d view
  //
  auto rt = std::make_shared<RenderTarget>(m_view);
  rt->color[0] = 0.2f;
  rt->color[1] = 0.2f;
  rt->color[2] = 0.2f;
  rt->color[3] = 1.0f;

  auto gl3r = std::make_shared<Gl3Renderer>();

  rt->render = [timeline = m_timeline,
                scene = m_scene,
                gl3r,
                selection = context](const ViewProjection& camera) {
    gl3r->clear(camera);

    auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

    RenderFunc render = [gl3r, liner](const ViewProjection& camera,
                                      const gltf::Mesh& mesh,
                                      const gltf::MeshInstance& instance,
                                      const float m[16]) {
      gl3r->render(camera, mesh, instance, m);
    };
    scene->Render(timeline->CurrentTime, camera, render);
    liner->Render(camera.projection, camera.view, gizmo::lines());

    // gizmo
    if (auto node = selection->selected) {
      // TODO: conflict mouse event(left) with ImageButton
      auto m = node->world;
      ImGuizmo::GetContext().mAllowActiveHoverItem = true;
      if (ImGuizmo::Manipulate(camera.view,
                               camera.projection,
                               ImGuizmo::TRANSLATE | ImGuizmo::ROTATE,
                               ImGuizmo::LOCAL,
                               (float*)&m,
                               nullptr,
                               nullptr,
                               nullptr,
                               nullptr)) {
        // decompose feedback
        node->setWorldMatrix(m);
      }
      ImGuizmo::GetContext().mAllowActiveHoverItem = false;
    }
  };

  m_gui->m_docks.push_back(Dock("view", [rt, scene = m_scene](bool* p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    if (ImGui::Begin("render target",
                     p_open,
                     ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoScrollWithMouse)) {
      auto pos = ImGui::GetWindowPos();
      pos.y += ImGui::GetFrameHeight();
      auto size = ImGui::GetContentRegionAvail();
      rt->show_fbo(pos.x, pos.y, size.x, size.y);
    }

    ImGui::End();
    ImGui::PopStyleVar();
  }));
}

void
App::jsonDock()
{

  auto enter = [](nlohmann::json& item, const std::string& key) {
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;
    auto is_leaf = !item.is_object() && !item.is_array();
    if (is_leaf) {
      node_flags |=
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    }
    std::stringstream ss;

    // std::string label;
    if (item.is_object()) {
      if (item.find("name") != item.end()) {
        ss << key << ": " << (std::string_view)item.at("name");
      } else {
        ss << key << ": object";
      }
    } else if (item.is_array()) {
      ss << key << ": [" << item.size() << "]";
    } else {
      ss << key << ": " << item.dump();
    }
    auto label = ss.str();
    bool node_open = ImGui::TreeNodeEx(
      (void*)(intptr_t)&item, node_flags, "%s", label.c_str());
    return node_open && !is_leaf;
  };
  auto leave = []() { ImGui::TreePop(); };

  m_gui->m_docks.push_back(Dock("gltf", [scene = m_scene, enter, leave]() {
    scene->TraverseJson(enter, leave);
  }));
}

void
App::timelineDock()
{
  auto timelineGui = std::make_shared<ImTimeline>(m_scene);

  m_gui->m_docks.push_back(
    Dock("timeline", [timeline = m_timeline, timelineGui]() {
      timelineGui->show(timeline);
    }));
}

void
App::loggerDock()
{
  m_gui->m_docks.push_back(
    Dock("logger", [logger = m_logger]() { logger->Draw(); }));
}
