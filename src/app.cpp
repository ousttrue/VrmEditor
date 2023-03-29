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
  lua_ = std::make_shared<LuaEngine>();
  scene_ = std::make_shared<Scene>();
  m_timeline = std::make_shared<Timeline>();

  platform_ = std::make_shared<Platform>();
  auto window =
      platform_->createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    throw std::runtime_error("createWindow");
  }
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("glewInit");
  }

  gui_ = std::make_shared<Gui>(window, platform_->glsl_version.c_str());

  cuber::PushGrid(gizmo::lines());
  gizmo::fix();
}

App::~App() {}

void App::clear_scene() {
  m_timeline->Tracks.clear();
  scene_->Clear();
}

bool App::load_model(const std::filesystem::path &path) {
  if (!scene_->Load(path)) {
    return false;
  }

  // bind time line
  for (auto &animation : scene_->m_animations) {
    auto track = m_timeline->AddTrack("gltf", animation->m_duration);
    track->Callback = [animation, scene = scene_](auto time, bool repeat) {
      animation->update(time, scene->m_nodes, repeat);
    };

    // first animation only
    break;
  }

  return true;
}

bool App::load_motion(const std::filesystem::path &path, float scaling) {
  motion_ = Bvh::ParseFile(path);

  motionSolver_ = std::make_shared<BvhSolver>();
  motionSolver_->Initialize(motion_);

  auto track = m_timeline->AddTrack("bvh", motion_->Duration());
  track->Callback = [this](auto time, bool repeat) {
    auto index = motion_->TimeToIndex(time);
    auto frame = motion_->GetFrame(index);
    motionSolver_->ResolveFrame(frame);

    // apply vrm
    if (scene_->m_vrm0) {
      auto &hips = motionSolver_->instances_[0];
      scene_->SetHumanPose(humanBoneMap_, {hips._41, hips._42, hips._43},
                           motionSolver_->localRotations);
    }
  };

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
                  motionSolver_->instances_.data(),
                  motionSolver_->instances_.size());
    liner->Render(camera.projection, camera.view, lines);
  };

  auto gl3r = std::make_shared<Gl3Renderer>();

  gui_->m_docks.push_back(Dock("motion", [rt](bool *p_open) {
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

  return motion_ != nullptr;
}

void App::load_lua(const std::filesystem::path &path) { lua_->dofile(path); }

bool App::addAssetDir(std::string_view name, const std::string &path) {

  assets_.push_back(std::make_shared<AssetDir>(name, path));

  return true;
}

int App::run() {

  jsonDock();
  sceneDock();
  timelineDock();
  assetsDock();

  std::optional<Time> lastTime;
  while (auto info = platform_->newFrame()) {
    // double sec to int64_t milliseconds
    auto time = std::chrono::duration_cast<Time>(info->time);
    m_timeline->SetTime(time);

    gizmo::clear();
    if (lastTime) {
      scene_->UpdateDeltaTime(time - *lastTime);
    } else {
      scene_->UpdateDeltaTime(time);
    }
    lastTime = time;

    // newFrame
    gui_->newFrame();
    ImGuizmo::BeginFrame();

    gui_->update();

    glViewport(0, 0, info->width, info->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gui_->render();
    platform_->present();
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

  auto enter = [this, context](Node &node, const DirectX::XMFLOAT4X4 &parent) {
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
    bool node_open = ImGui::TreeNodeEx((void *)(intptr_t)node.index, node_flags,
                                       "%s", node.name.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      context->new_selected = &node;
    }

    return node.children.size() && node_open;
  };
  auto leave = []() { ImGui::TreePop(); };

  auto timelineGui = std::make_shared<ImTimeline>();
  gui_->m_docks.push_back(
      Dock("scene", [scene = scene_, enter, leave, context, timelineGui]() {
        ImGui::Checkbox("IsPlaying", &scene->m_isPlaying);
        ImGui::BeginDisabled(scene->m_isPlaying);
        if (ImGui::Button("next frame")) {
          scene->m_timeline->SetDeltaTime(Time(1.0 / 60));
        }
        ImGui::EndDisabled();

        timelineGui->show(scene->m_timeline);

        context->selected = context->new_selected;
        scene->Traverse(enter, leave);
      }));

  gui_->m_docks.push_back(Dock("selected", [scene = scene_, context]() {
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

  gui_->m_docks.push_back(Dock("vrm-0.x", [scene = scene_]() {
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

  rt->render = [scene = scene_, gl3r,
                selection = context](const Camera &camera) {
    gl3r->clear(camera);

    auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

    RenderFunc render = [gl3r, liner](const Camera &camera, const Mesh &mesh,
                                      const float m[16]) {
      gl3r->render(camera, mesh, m);
    };
    scene->Render(camera, render);
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

  gui_->m_docks.push_back(Dock("view", [rt, scene = scene_](bool *p_open) {
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

  gui_->m_docks.push_back(Dock("json", [scene = scene_, enter, leave]() {
    scene->TraverseJson(enter, leave);
  }));
}

void App::timelineDock() {
  auto timelineGui = std::make_shared<ImTimeline>();

  gui_->m_docks.push_back(
      Dock("timeline", [timeline = m_timeline, timelineGui]() {
        timelineGui->show(timeline);
      }));
}

void App::assetsDock() {
  for (auto asset : assets_) {
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
          clear_scene();
          load_model(path);
        }
        return false;
      }
    };
    auto leave = []() { ImGui::TreePop(); };
    gui_->m_docks.push_back(
        Dock(std::format("asset: {}", asset->name()),
             [asset, enter, leave]() { asset->traverse(enter, leave); }));
  }
}
