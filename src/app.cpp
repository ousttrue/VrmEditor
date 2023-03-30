#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <gl/glew.h>

#include "app.h"
#include "assetdir.h"
#include "gl3renderer.h"
#include "gui.h"
#include "imtimeline.h"
#include "luahost.h"
#include "platform.h"
#include "rendertarget.h"
#include "windows_helper.h"
#include <Bvh.h>
#include <BvhSolver.h>
#include <ImGuiFileDialog.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <iostream>
#include <vrm/animation.h>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/scene.h>
#include <vrm/vrm0.h>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

App::App() {
  m_lua = std::make_shared<LuaEngine>();
  m_scene = std::make_shared<Scene>();
  m_timeline = std::make_shared<Timeline>();

  m_platform = std::make_shared<Platform>();
  auto window =
      m_platform->createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    throw std::runtime_error("createWindow");
  }
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("glewInit");
  }

  m_gui = std::make_shared<Gui>(window, m_platform->glsl_version.c_str());

  cuber::PushGrid(gizmo::lines());
  gizmo::fix();
}

App::~App() {}

void App::ClearScene() {
  m_timeline->Tracks.clear();
  m_scene->Clear();
}

bool App::LoadModel(const std::filesystem::path &path) {
  if (!m_scene->Load(path)) {
    return false;
  }

  // bind time line
  for (auto &animation : m_scene->m_animations) {
    auto track = m_timeline->AddTrack("gltf", animation->duration());
    track->Callbacks.push_back(
        [animation, scene = m_scene](auto time, bool repeat) {
          animation->update(time, scene->m_nodes, repeat);
        });

    // first animation only
    break;
  }

  return true;
}

bool App::LoadMotion(const std::filesystem::path &path, float scaling) {
  m_motion = Bvh::ParseFile(path);

  m_motionSolver = std::make_shared<BvhSolver>();
  m_motionSolver->Initialize(m_motion);

  auto track = m_timeline->AddTrack("bvh", m_motion->Duration());
  track->Callbacks.push_back([this](auto time, bool repeat) {
    auto index = m_motion->TimeToIndex(time);
    auto frame = m_motion->GetFrame(index);
    m_motionSolver->ResolveFrame(frame);

    // apply vrm
    if (m_scene->m_vrm0) {
      auto &hips = m_motionSolver->instances_[0];
      m_scene->SetHumanPose(m_humanBoneMap, {hips._41, hips._42, hips._43},
                            m_motionSolver->localRotations);
    }
  });

  auto rt = std::make_shared<RenderTarget>();
  rt->color[0] = 0.4f;
  rt->color[1] = 0.2f;
  rt->color[2] = 0.2f;
  rt->color[3] = 1.0f;

  auto cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
  auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  std::vector<grapho::LineVertex> lines;
  cuber::PushGrid(lines);

  rt->render = [this, cuber, liner, lines](const Camera &camera) {
    cuber->Render(camera.projection, camera.view,
                  m_motionSolver->instances_.data(),
                  m_motionSolver->instances_.size());
    liner->Render(camera.projection, camera.view, lines);
  };

  auto gl3r = std::make_shared<Gl3Renderer>();

  m_gui->m_docks.push_back(Dock("motion", [rt](bool *p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    if (ImGui::Begin("motion", p_open,
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

  return m_motion != nullptr;
}

void App::LoadLua(const std::filesystem::path &path) { m_lua->dofile(path); }

bool App::AddAssetDir(std::string_view name, const std::string &path) {

  m_assets.push_back(std::make_shared<AssetDir>(name, path));

  return true;
}

static void drawGui() {
  // open Dialog Simple
  if (ImGui::Button("Open File Dialog"))
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File",
                                            ".cpp,.h,.hpp", ".");

  // display
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
    // action if OK
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
      // action
    }

    // close
    ImGuiFileDialog::Instance()->Close();
  }
}

int App::Run() {

  jsonDock();
  sceneDock();
  timelineDock();
  assetsDock();

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
    drawGui();

    glViewport(0, 0, info->width, info->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gui->Render();
    m_platform->present();
  }

  return 0;
}

struct TreeContext {
  Node *selected = nullptr;
  Node *new_selected = nullptr;
};

void App::sceneDock() {
  //
  // scene tree
  //
  auto context = std::make_shared<TreeContext>();

  auto enter = [this, context](Node &node) {
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

    bool node_open = ImGui::TreeNodeEx((void *)(intptr_t)node.index, node_flags,
                                       "%s", node.name.c_str());

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
        for (auto &morph : mesh->m_morphTargets) {
          ImGui::SliderFloat(morph->name.c_str(), &morph->weight, 0, 1);
        }
      }
    }
  }));

  m_gui->m_docks.push_back(Dock("vrm-0.x", [scene = m_scene]() {
    if (auto vrm = scene->m_vrm0) {
      for (auto expression : vrm->m_expressions) {
        ImGui::SliderFloat(expression->label.c_str(), &expression->weight, 0,
                           1);
      }
    }
  }));

  //
  // 3d view
  //
  auto rt = std::make_shared<RenderTarget>();
  rt->color[0] = 0.2f;
  rt->color[1] = 0.2f;
  rt->color[2] = 0.2f;
  rt->color[3] = 1.0f;

  auto gl3r = std::make_shared<Gl3Renderer>();

  rt->render = [timeline = m_timeline, scene = m_scene, gl3r,
                selection = context](const Camera &camera) {
    gl3r->clear(camera);

    auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

    RenderFunc render = [gl3r, liner](const Camera &camera, const Mesh &mesh,
                                      const float m[16]) {
      gl3r->render(camera, mesh, m);
    };
    scene->Render(timeline->CurrentTime, camera, render);
    liner->Render(camera.projection, camera.view, gizmo::lines());

    // gizmo
    if (auto node = selection->selected) {
      // TODO: conflict mouse event(left) with ImageButton
      auto m = node->world;
      ImGuizmo::GetContext().mAllowActiveHoverItem = true;
      if (ImGuizmo::Manipulate(camera.view, camera.projection,
                               ImGuizmo::UNIVERSAL, ImGuizmo::LOCAL,
                               (float *)&m, NULL, NULL, NULL, NULL)) {
        // decompose feedback
        node->setWorldMatrix(m);
      }
      ImGuizmo::GetContext().mAllowActiveHoverItem = false;
    }
  };

  m_gui->m_docks.push_back(Dock("view", [rt, scene = m_scene](bool *p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    if (ImGui::Begin("render target", p_open,
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

void App::jsonDock() {

  auto enter = [](nlohmann::json &item, const std::string &key) {
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
    std::string label;
    if (item.is_object()) {
      if (item.find("name") != item.end()) {
        label = std::format("{}: {}", key, (std::string_view)item.at("name"));
      } else {
        label = std::format("{}: object", key);
      }
    } else if (item.is_array()) {
      label = std::format("{}: [{}]", key, item.size());
    } else {
      label = std::format("{}: {}", key, item.dump());
    }
    bool node_open = ImGui::TreeNodeEx((void *)(intptr_t)&item, node_flags,
                                       "%s", label.c_str());
    return node_open && !is_leaf;
  };
  auto leave = []() { ImGui::TreePop(); };

  m_gui->m_docks.push_back(Dock("json", [scene = m_scene, enter, leave]() {
    scene->TraverseJson(enter, leave);
  }));
}

void App::timelineDock() {
  auto timelineGui = std::make_shared<ImTimeline>(m_scene);

  m_gui->m_docks.push_back(
      Dock("timeline", [timeline = m_timeline, timelineGui]() {
        timelineGui->show(timeline);
      }));
}

void App::assetsDock() {
  for (auto asset : m_assets) {
    auto enter = [this](const std::filesystem::path &path, uint64_t id) {
      static ImGuiTreeNodeFlags base_flags =
          ImGuiTreeNodeFlags_OpenOnArrow |
          ImGuiTreeNodeFlags_OpenOnDoubleClick |
          ImGuiTreeNodeFlags_SpanAvailWidth;
      auto mb = WideToMb(CP_OEMCP, path.filename().c_str());
      if (std::filesystem::is_directory(path)) {
        ImGuiTreeNodeFlags node_flags = base_flags;
        return ImGui::TreeNodeEx((void *)(intptr_t)id, node_flags, "%s",
                                 mb.c_str());
      } else {
        if (ImGui::Button(mb.c_str())) {
          ClearScene();
          LoadModel(path);
        }
        return false;
      }
    };
    auto leave = []() { ImGui::TreePop(); };
    m_gui->m_docks.push_back(
        Dock(std::format("asset: {}", asset->name()),
             [asset, enter, leave]() { asset->traverse(enter, leave); }));
  }
}
