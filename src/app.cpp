#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <gl/glew.h>

#include "app.h"
#include "gl3renderer.h"
#include "gui.h"
#include "no_sal2.h"
#include "orbitview.h"
#include "platform.h"
#include <Bvh.h>
#include <chrono>
#include <format>
#include <imgui.h>
#include <imgui_internal.h>
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

#include <glo/fbo.h>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

static std::string WideToMb(uint32_t cp, const wchar_t *src) {
  auto l = WideCharToMultiByte(cp, 0, src, -1, nullptr, 0, nullptr, nullptr);
  if (std::wstring(src).starts_with(L"Two")) {
    auto a = 0;
  }
  std::string dst;
  dst.resize(l);
  l = WideCharToMultiByte(cp, 0, src, -1, dst.data(), l, nullptr, nullptr);
  dst.push_back(0);
  return dst;
}

static int vrmeditor_load_model(lua_State *l) {
  auto path = luaL_checklstring(l, -1, nullptr);

  auto succeeded = App::instance().load_model(path);
  lua_pushboolean(App::instance().lua(), succeeded);
  return 1;
}

static int vrmeditor_load_motion(lua_State *l) {
  auto path = luaL_checklstring(l, -2, nullptr);
  auto scale = luaL_checknumber(l, -1);

  auto succeeded = App::instance().load_motion(path, scale);
  lua_pushboolean(App::instance().lua(), succeeded);
  return 1;
}

static int vrmeditor_add_asset_dir(lua_State *l) {
  auto name = luaL_checklstring(l, -2, nullptr);
  auto dir = luaL_checklstring(l, -1, nullptr);
  auto succeeded = App::instance().addAssetDir(name, dir);
  lua_pushboolean(App::instance().lua(), succeeded);
  return 1;
}

static const struct luaL_Reg VrmEditorLuaModule[] = {
    {"load_model", vrmeditor_load_model},
    {"load_motion", vrmeditor_load_motion},
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

App::App() { scene_ = std::make_shared<Scene>(); }

App::~App() {}

lua_State *App::lua() { return lua_.state(); }

bool App::load_model(const std::filesystem::path &path) {
  return scene_->load(path);
}

bool App::load_motion(const std::filesystem::path &path, float scaling) {
  motion_ = Bvh::ParseFile(path);
  return motion_ != nullptr;
}

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

  gui_ = std::make_shared<Gui>(window, platform.glsl_version.c_str());

  if (argc > 1) {
    std::string_view arg = argv[1];
    if (arg.ends_with(".lua")) {
      lua_.dofile(argv[1]);
    } else {
      scene_->load(argv[1]);
    }
  }

  jsonDock();
  sceneDock();
  timelineDock();
  assetsDock();

  while (auto info = platform.newFrame()) {
    scene_->update(
        std::chrono::duration_cast<std::chrono::milliseconds>(info->time));
    // newFrame
    gui_->newFrame();
    ImGuizmo::BeginFrame();
    time_ = info->time;

    gui_->update();

    glViewport(0, 0, info->width, info->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gui_->render();
    platform.present();
  }

  gui_ = nullptr;

  return 0;
}

struct TreeContext {
  Node *selected = nullptr;
  Node *new_selected = nullptr;
};

struct RenderTarget {
  Camera camera;
  OrbitView view;
  std::shared_ptr<glo::Fbo> fbo;
  float color[4];
  std::function<void(const Camera &camera)> render;
  std::shared_ptr<TreeContext> selection;

  uint32_t clear(int width, int height) {
    if (width == 0 || height == 0) {
      return 0;
    }

    if (fbo) {
      if (fbo->texture->width_ != width || fbo->texture->height_ != height) {
        fbo = nullptr;
      }
    }
    if (!fbo) {
      fbo = glo::Fbo::Create(width, height);
    }

    fbo->Bind();
    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);
    glClearColor(color[0] * color[3], color[1] * color[3], color[2] * color[3],
                 color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    return fbo->texture->texture_;
  }

  void show_fbo(float x, float y, float w, float h) {
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(x, y, w, h);

    assert(w);
    assert(h);
    auto texture = clear(int(w), int(h));
    if (texture) {
      // image button. capture mouse event
      ImGui::ImageButton((ImTextureID)texture, {w, h}, {0, 1}, {1, 0}, 0,
                         {1, 1, 1, 1}, {1, 1, 1, 1});
      ImGui::ButtonBehavior(ImGui::GetCurrentContext()->LastItemData.Rect,
                            ImGui::GetCurrentContext()->LastItemData.ID,
                            nullptr, nullptr,
                            ImGuiButtonFlags_MouseButtonMiddle |
                                ImGuiButtonFlags_MouseButtonRight);

      // update camera
      auto &io = ImGui::GetIO();
      camera.resize(w, h);
      view.SetSize(w, h);
      if (ImGui::IsItemActive()) {
        if (io.MouseDown[ImGuiMouseButton_Right]) {
          view.YawPitch(io.MouseDelta.x, io.MouseDelta.y);
        }
        if (io.MouseDown[ImGuiMouseButton_Middle]) {
          view.Shift(io.MouseDelta.x, io.MouseDelta.y);
        }
      }
      if (ImGui::IsItemHovered()) {
        view.Dolly(io.MouseWheel);
      }
      view.Update(camera.projection, camera.view);
      render(camera);

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
    }
    fbo->Unbind();
  }
};

void App::sceneDock() {
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

  gui_->m_docks.push_back(
      Dock("scene", [scene = scene_, enter, leave, context]() {
        context->selected = context->new_selected;

        scene->traverse(enter, leave);
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
      ImGui::Begin("vrm-0.x");
      for (auto expression : vrm->m_expressions) {
        ImGui::SliderFloat(expression->label.c_str(), &expression->weight, 0,
                           1);
      }
      ImGui::End();
    }
  }));

  auto rt = std::make_shared<RenderTarget>();
  rt->color[0] = 0.2;
  rt->color[1] = 0.2;
  rt->color[2] = 0.2;
  rt->color[3] = 1.0;
  rt->selection = context;

  auto gl3r = std::make_shared<Gl3Renderer>();

  rt->render = [scene = scene_, gl3r](const Camera &camera) {
    gl3r->clear(camera);

    RenderFunc render = [gl3r](const Camera &camera, const Mesh &mesh,
                               const float m[16]) {
      gl3r->render(camera, mesh, m);
    };
    scene->render(camera, render);
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

// cursor
// start      end
// +-----------+
// |           |
// +-----------+
struct TimeDraw {
  ImVec2 cursor;
  ImVec2 size;
  std::chrono::milliseconds start;
  std::chrono::milliseconds end;
  ImU32 color = IM_COL32_BLACK;
  TimeDraw(const ImVec2 &cursor, const ImVec2 &size,
           std::chrono::milliseconds start, std::chrono::milliseconds end)
      : cursor(cursor), size(size), start(start), end(end) {}
  TimeDraw(const ImVec2 &cursor, const ImVec2 &size,
           std::chrono::milliseconds center)
      : TimeDraw(cursor, size,
                 center - std::chrono::milliseconds((int64_t)size.x * 5),
                 center + std::chrono::milliseconds((int64_t)size.x * 5)) {}
  bool drawLine(ImDrawList *drawList, std::chrono::milliseconds time) {

    if (time < start) {
      return false;
    }
    if (time > end) {
      return false;
    }

    auto delta = time - start;
    auto x = cursor.x + delta.count() * 0.1f;
    drawList->AddLine({x, cursor.y}, {x, cursor.y + size.y}, color, 1.0f);

    char text[64];
    sprintf_s(text, sizeof(text), "%ld", time.count());
    drawList->AddText({x, cursor.y}, color, text, nullptr);

    return true;
  }

  void drawLines(ImDrawList *drawList) {
    auto count = start.count() / 1000;
    for (auto current = std::chrono::milliseconds(count * 1000); current < end;
         current += std::chrono::milliseconds(1000)) {
      drawLine(drawList, current);
    }
  }

  void drawNow(ImDrawList *drawList, std::chrono::milliseconds time) {
    auto delta = time - start;
    auto x = cursor.x + delta.count() * 0.1f;
    drawList->AddLine({x, cursor.y}, {x, cursor.y + size.y},
                      IM_COL32(255, 0, 0, 255), 1.0f);

    char text[64];
    sprintf_s(text, sizeof(text), "%ld", time.count());
    drawList->AddText({x, cursor.y}, color, text, nullptr);
  }
};

class ImTimeline {
public:
  ImTimeline() {}
  ~ImTimeline() {}

  void show(std::chrono::milliseconds now, const ImVec2 &size = {0, 0}) {
    // ImGuiContext &g = *GImGui;
    // ImGuiWindow *window = ImGui::GetCurrentWindow();
    // const auto &imStyle = ImGui::GetStyle();
    const auto drawList = ImGui::GetWindowDrawList();
    const auto cursor = ImGui::GetCursorScreenPos();
    const auto area = ImGui::GetContentRegionAvail();
    // const auto cursorBasePos = ImGui::GetCursorScreenPos() + window->Scroll;

    auto textSize = ImGui::CalcTextSize(" ");
    auto lineheight = textSize.y;

    auto realSize = ImFloor(size);
    if (realSize.x <= 0.0f)
      realSize.x = ImMax(4.0f, area.x);
    if (realSize.y <= 0.0f)
      realSize.y = ImMax(4.0f, lineheight);

    TimeDraw(cursor, {realSize.x, lineheight + lineheight}, now)
        .drawNow(drawList, now);

    TimeDraw({cursor.x, cursor.y + lineheight}, realSize, now)
        .drawLines(drawList);
  }
};

void App::timelineDock() {
  auto timeline = std::make_shared<ImTimeline>();

  gui_->m_docks.push_back(Dock("timeline", [this, timeline]() {
    timeline->show(
        std::chrono::duration_cast<std::chrono::milliseconds>(time_));
  }));
}

void App::assetsDock() {
  for (auto asset : assets_) {
    auto enter = [scene = scene_](const std::filesystem::path &path,
                                  uint64_t id) {
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
          scene->clear();
          scene->load(path);
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