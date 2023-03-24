#include "app.h"
#include "gl3renderer.h"
#include "gui.h"
#include "no_sal2.h"
#include "orbitview.h"
#include "platform.h"
#include <format>
#include <imgui.h>
#include <iostream>
#include <vrm/animation.h>
#include <vrm/mesh.h>
#include <vrm/node.h>
#include <vrm/scene.h>
#include <vrm/vrm0.h>

#include <ImGuizmo.h>
#include <imgui_neo_sequencer.h>

#include <Windows.h>
#include <lua.hpp>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

static std::string WideToMb(const wchar_t *src) {
  auto l = WideCharToMultiByte(932, 0, src, -1, nullptr, 0, nullptr, nullptr);
  std::string dst;
  dst.resize(l);
  l = WideCharToMultiByte(932, 0, src, -1, dst.data(), l, nullptr, nullptr);
  return dst;
}

static int vrmeditor_load(lua_State *l) {
  auto path = luaL_checklstring(l, -1, nullptr);

  auto succeeded = App::instance().load(path);
  lua_pushboolean(App::instance().lua(), succeeded);
  return 1;
}

static int vrmeditor_add_asset_dir(lua_State *l) {
  auto dir = luaL_checklstring(l, -1, nullptr);
  auto name = luaL_checklstring(l, -2, nullptr);
  auto succeeded = App::instance().addAssetDir(name, dir);
  lua_pushboolean(App::instance().lua(), succeeded);
  return 1;
}

static const struct luaL_Reg VrmEditorLuaModule[] = {
    {"load", vrmeditor_load},
    {"add_asset_dir", vrmeditor_add_asset_dir},
    {NULL, NULL},
};

LuaEngine::LuaEngine() {
  L_ = luaL_newstate();
  luaL_openlibs(L_);
  luaL_register(L_, "vrmeditor", VrmEditorLuaModule);
}

LuaEngine::~LuaEngine() { lua_close(L_); }

void LuaEngine::eval(const std::string &script) {
  luaL_dostring(L_, script.c_str());
}

void LuaEngine::dofile(const std::string &path) {
  auto ret = luaL_dofile(L_, path.c_str());
  if (ret != 0) {
    std::cout << "luaL_dofile(): " << ret << std::endl;
    std::cout << lua_tostring(L_, -1) << std::endl;
  }
}
AssetDir::AssetDir(std::string_view name, std::string_view path) : name_(name) {
  root_ = path;
}

void AssetDir::traverse(const AssetEnter &enter, const AssetLeave &leave,
                        const std::filesystem::path &path) {

  if (path.empty()) {
    // root
    // traverse(enter, leave, root_);
    for (auto e : std::filesystem::directory_iterator(root_)) {
      traverse(enter, leave, e);
    }
    return;
  }

  uint64_t id;
  auto found = idMap_.find(path);
  if (found != idMap_.end()) {
    id = found->second;
  } else {
    id = nextId_++;
    idMap_.insert(std::make_pair(path, id));
  }

  if (enter(path, id)) {
    if (std::filesystem::is_directory(path)) {
      for (auto e : std::filesystem::directory_iterator(path)) {
        traverse(enter, leave, e);
      }
    }
    leave();
  }
}

App::App() {}

App::~App() {}

lua_State *App::lua() { return lua_.state(); }

bool App::load(const std::filesystem::path &path) { return scene_->load(path); }

bool App::addAssetDir(std::string_view name, const std::string &path) {

  assets_.push_back(std::make_shared<AssetDir>(name, path));

  return true;
}

int App::run(int argc, char **argv) {

  Platform platform;
  auto window =
      platform.createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    return 1;
  }

  Gl3Renderer gl3r;
  OrbitView view;

  gui_ = std::make_shared<Gui>(window, platform.glsl_version.c_str());
  scene_ = std::make_shared<Scene>();

  if (argc > 1) {
    std::string_view arg = argv[1];
    if (arg.ends_with(".lua")) {
      lua_.dofile(argv[1]);
    } else {
      scene_->load(argv[1]);
    }
  }

  RenderFunc render = [&gl3r](const Camera &camera, const Mesh &mesh,
                              const float m[16]) {
    gl3r.render(camera, mesh, m);
  };

  float m[16]{
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
  };

  int32_t currentFrame = 0;
  int32_t startFrame = -10;
  int32_t endFrame = 64;
  // static bool transformOpen = false;

  jsonDock();
  sceneDock();

  while (auto info = platform.newFrame()) {
    // newFrame
    gui_->newFrame();
    camera.resize(info->width, info->height);
    view.SetSize(info->width, info->height);
    if (auto event = gui_->backgroundMouseEvent()) {
      if (auto delta = event->rightDrag) {
        view.YawPitch(delta->x, delta->y);
      }
      if (auto delta = event->middleDrag) {
        view.Shift(delta->x, delta->y);
      }
      if (auto wheel = event->wheel) {
        view.Dolly(*wheel);
      }
    }
    view.Update(camera.projection, camera.view);

    // render view
    gl3r.clear(camera);
    scene_->render(camera, render, info->time);

    // render gui
    ImGuizmo::BeginFrame();
    auto vp = ImGui::GetMainViewport();
    ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);
    ImGuizmo::DrawGrid(camera.view, camera.projection, m, 100);
    // ImGuizmo::DrawCubes(camera.view, camera.projection, m, 1);

    {
      ImGui::Begin("timeline");
      if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame,
                                   &endFrame)) {
        // Timeline code here
        ImGui::EndNeoSequencer();
      }
      ImGui::End();
    }

    if (auto vrm = scene_->m_vrm0) {
      ImGui::Begin("vrm-0.x");
      for (auto expression : vrm->m_expressions) {
        ImGui::SliderFloat(expression->label.c_str(), &expression->weight, 0,
                           1);
      }
      ImGui::End();
    }

    for (auto asset : assets_) {
      ImGui::Begin(asset->name().c_str());
      auto enter = [](const std::filesystem::path &path, uint64_t id) {
        static ImGuiTreeNodeFlags base_flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanAvailWidth;
        if (std::filesystem::is_directory(path)) {
          ImGuiTreeNodeFlags node_flags = base_flags;
          return ImGui::TreeNodeEx((void *)(intptr_t)id, node_flags, "%ls",
                                   path.filename().c_str());
        } else {
          auto mb = WideToMb(path.filename().c_str());
          if (ImGui::Button(mb.c_str())) {
            App::instance().load(path);
          }
          return false;
        }
      };
      auto leave = []() { ImGui::TreePop(); };
      asset->traverse(enter, leave);
      ImGui::End();
    }

    gui_->update();

    gui_->render();

    platform.present();
  }

  return 0;
}

struct TreeContext {
  Node *selected = nullptr;
  Node *new_selected = nullptr;
};

void App::sceneDock() {
  auto context = std::make_shared<TreeContext>();

  auto enter = [this, context](Node &node, const DirectX::XMFLOAT4X4 &parent) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    static ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;
    auto is_selected = context->selected == &node;
    if (is_selected) {
      node_flags |= ImGuiTreeNodeFlags_Selected;

      auto m = node.world;

      if (ImGuizmo::Manipulate(camera.view, camera.projection,
                               ImGuizmo::UNIVERSAL, ImGuizmo::LOCAL,
                               (float *)&m, NULL, NULL, NULL, NULL)) {
        // decompose feedback
        node.setWorldMatrix(m, parent);
      }
    }

    if (node.children.empty()) {
      node_flags |=
          ImGuiTreeNodeFlags_Leaf |
          ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    }

    bool node_open = ImGui::TreeNodeEx((void *)(intptr_t)node.index, node_flags,
                                       "%s", node.name.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      context->new_selected = &node;
    }

    return node.children.size() && node_open;
  };
  auto leave = []() { ImGui::TreePop(); };

  gui_->m_docks.push_back(
      Dock("scene", [scene = scene_, enter, leave, context]() {
        context->selected = context->new_selected;

        if (context->selected) {
          ImGui::Begin(context->selected->name.c_str());
          if (auto mesh_index = context->selected->mesh) {
            auto mesh = scene->m_meshes[*mesh_index];
            for (auto &morph : mesh->m_morphTargets) {
              ImGui::SliderFloat(morph->name.c_str(), &morph->weight, 0, 1);
            }
          }
          ImGui::End();
        }

        scene->traverse(enter, leave);
      }));
}

void App::jsonDock() {

  auto enter = [](json &item, const std::string &key) {
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
    scene->traverse_json(enter, leave);
  }));
}
