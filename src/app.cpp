#include "app.h"
#include "camera.h"
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

#include <ImGuizmo.h>
#include <imgui_neo_sequencer.h>

#include <lua.hpp>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

struct TreeContext {
  Node *selected = nullptr;
  Node *new_selected = nullptr;
};

static int vrmeditor_load(lua_State *l) {
  auto path = luaL_checklstring(l, -1, nullptr);

  auto succeeded = App::instance().load(path);
  lua_pushboolean(App::instance().lua(), succeeded);
  return 1;
}

static const struct luaL_Reg VrmEditorLuaModule[] = {
    {"load", vrmeditor_load},
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

App::App() {}

App::~App() {}

lua_State *App::lua() { return lua_.state(); }

bool App::load(const std::string &path) { return scene_->load(path.c_str()); }

int App::run(int argc, char **argv) {

  Platform platform;
  auto window =
      platform.createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    return 1;
  }

  Gl3Renderer gl3r;
  Gui gui(window, platform.glsl_version.c_str());
  Camera camera{};
  OrbitView view;

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

  TreeContext context;

  int32_t currentFrame = 0;
  int32_t startFrame = -10;
  int32_t endFrame = 64;
  // static bool transformOpen = false;

  while (auto info = platform.newFrame()) {
    // newFrame
    gui.newFrame();
    camera.resize(info->width, info->height);
    view.SetSize(info->width, info->height);
    if (auto event = gui.backgroundMouseEvent()) {
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
      auto enter = [](json &item, const std::string &key) {
        static ImGuiTreeNodeFlags base_flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
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
            label =
                std::format("{}: {}", key, (std::string_view)item.at("name"));
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
      ImGui::Begin("json");
      scene_->traverse_json(enter, leave);
      ImGui::End();
    }

    {
      context.selected = context.new_selected;
      auto enter = [&context, &camera](Node &node,
                                       const DirectX::XMFLOAT4X4 &parent) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        static ImGuiTreeNodeFlags base_flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanAvailWidth;
        ImGuiTreeNodeFlags node_flags = base_flags;
        auto is_selected = context.selected == &node;
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

        bool node_open = ImGui::TreeNodeEx((void *)(intptr_t)node.index,
                                           node_flags, "%s", node.name.c_str());
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
          context.new_selected = &node;
        }

        return node.children.size() && node_open;
      };
      auto leave = []() { ImGui::TreePop(); };

      ImGui::Begin("scene");
      scene_->traverse(enter, leave);
      ImGui::End();
    }

    if (context.selected) {
      ImGui::Begin(context.selected->name.c_str());
      if (auto mesh_index = context.selected->mesh) {
        auto mesh = scene_->m_meshes[*mesh_index];
        for (auto &morph : mesh->m_morphTargets) {
          ImGui::SliderFloat(morph->name.c_str(), &morph->weight, 0, 1);
        }
      }
      ImGui::End();
    }

    {
      ImGui::Begin("timeline");
      if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame,
                                   &endFrame)) {
        // Timeline code here
        ImGui::EndNeoSequencer();
      }
      ImGui::End();
    }

    gui.update();
    gui.render();

    platform.present();
  }

  return 0;
}
