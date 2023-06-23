#include "hierarchy_gui.h"
#include "gui.h"
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <vrm/humanoid/humanbones.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

static std::optional<libvrm::HumanBones>
BoneSelector(const char* label, std::optional<libvrm::HumanBones> bone)
{
  uint32_t index = -1;
  const char* combo_preview_value = "--";
  if (bone) {
    index = (int)*bone;
    combo_preview_value = libvrm::HumanBonesNamesWithIcon[index];
  }

  // char key[64];
  // snprintf(key, sizeof(key), "##humanbone%d", (int)bone);
  auto ret = bone;
  if (ImGui::BeginCombo(label, combo_preview_value, 0)) {
    for (int i = 0; i < (int)libvrm::HumanBones::VRM_BONE_COUNT; i++) {
      bool is_selected = i == index;
      // auto& node = scene->m_nodes[n];
      if (ImGui::Selectable(libvrm::HumanBonesNamesWithIcon[i], is_selected)) {
        // for (auto& node : scene->m_nodes) {
        //   if (node->Humanoid == bone) {
        //     // clear old bone
        //     node->Humanoid = std::nullopt;
        //   }
        // }
        ret = (libvrm::HumanBones)i;
      }

      // Set the initial focus when opening the combo (scrolling +
      // keyboard navigation focus)
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  return ret;
}

static bool
DescendantHasHumanoid(const std::shared_ptr<libvrm::Node>& node)
{
  if (node->Humanoid) {
    return true;
  }
  for (auto& child : node->Children) {
    if (DescendantHasHumanoid(child)) {
      return true;
    }
  }
  return false;
}
static bool
DescendantHasHumanoid(const std::shared_ptr<libvrm::RuntimeNode>& node)
{
  return DescendantHasHumanoid(node->Node);
}

template<typename T, typename N>
void
Traverse(const std::shared_ptr<T>& scene, const std::shared_ptr<N>& node)
{
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  auto is_leaf = node->Children.size() == 0;
  if (is_leaf) {
    node_flags |=
      ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
  }
  if (node == scene->m_selected) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }
  if (DescendantHasHumanoid(node)) {
    node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
  }

  // auto& label = m_label->Get(item, jsonpath);
  ImGui::TableNextRow();
  ImGui::PushID(node.get());

  // 0
  ImGui::TableNextColumn();
  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);

  bool node_open = ImGui::TreeNodeEx(
    (void*)(intptr_t)node.get(), node_flags, "%s", node->GetLabel());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    scene->m_selected = node;
  }

  // 1
  ImGui::TableNextColumn();
  ImGui::SetNextItemWidth(-1);
  // ImGui::TextUnformatted(libvrm::HumanBoneToNameWithIcon(*humanoid));
  node->SetHumanBone(BoneSelector("##bone", node->GetHumanBone()));

  // T
  ImGui::TableNextColumn();
  ImGui::SetNextItemWidth(-1);
  if (ImGui::InputFloat3("##translation", &node->GetTranslation().x)) {
    node->Calc(true);
  }
  // R
  ImGui::TableNextColumn();
  ImGui::SetNextItemWidth(-1);
  if (ImGui::InputFloat4("##rotation", &node->GetRotation().x)) {
    node->Calc(true);
  }
  // S
  ImGui::TableNextColumn();
  ImGui::SetNextItemWidth(-1);
  if (ImGui::InputFloat3("##scale", &node->GetScale().x)) {
    node->Calc(true);
  }

  ImGui::PopID();
  if (node_open) {
    for (auto& child : node->Children) {
      Traverse(scene, child);
    }
    if (!is_leaf) {
      ImGui::TreePop();
    }
  }
}

template<typename T>
void
_ShowGui(const std::shared_ptr<T>& scene)
{
  if (!scene) {
    return;
  }

  std::array<const char*, 5> cols = {
    "name", "humanoid", "T", "R", "S",
  };

  if (grapho::imgui::BeginTableColumns("##NodeTree", cols)) {
    // tree
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, Gui::Instance().Indent());

    for (auto& root : scene->m_roots) {
      Traverse(scene, root);
    }

    ImGui::PopStyleVar();
    ImGui::EndTable();
  }
}

struct HierarchyGuiImplRuntime
{
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_runtime = runtime;
  }
  void ShowGui() { _ShowGui(m_runtime); }
};

struct HierarchyGuiImplAsset
{
  std::shared_ptr<libvrm::GltfRoot> m_base;
  void SetBase(const std::shared_ptr<libvrm::GltfRoot>& root) { m_base = root; }
  void ShowGui() { _ShowGui(m_base); }
};

HierarchyGui::HierarchyGui()
  : m_asset(new HierarchyGuiImplAsset)
  , m_runtime(new HierarchyGuiImplRuntime)
{
}

HierarchyGui::~HierarchyGui()
{
  delete m_runtime;
  delete m_asset;
}

void
HierarchyGui::SetRuntimeScene(
  const std::shared_ptr<libvrm::RuntimeScene>& rutime)
{
  m_asset->SetBase(rutime->m_base);
  m_runtime->SetRuntime(rutime);
}

void
HierarchyGui::ShowGui()
{
  if (ImGui::BeginTabBar("HierarchyTabs")) {
    if (ImGui::BeginTabItem("🎁Asset")) {
      m_asset->ShowGui();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("🎬Runtime")) {
      m_runtime->ShowGui();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}
