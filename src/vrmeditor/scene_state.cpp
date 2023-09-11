#include "scene_state.h"
#include <gltfjson.h>
#include <gltfjson/glb.h>
#include <gltfjson/json_tree_exporter.h>
#include <plog/Log.h>
#include <vrm/gltfroot.h>
#include <vrm/importer.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

void
SceneState::SetGltf(const std::shared_ptr<libvrm::GltfRoot>& gltf)
{
  m_runtime = libvrm::RuntimeScene::Load(gltf);
  m_lastTime = {};

  for (auto& callback : m_setCallbacks) {
    callback(m_runtime);
  }
}

bool
SceneState::LoadModel(const std::filesystem::path& path)
{
  if (auto gltf = libvrm::LoadPath(path)) {
    SetGltf(gltf);
    auto value = path.string();
    PLOG_INFO << value;
    return true;
  } else {
    // PLOG_ERROR << gltf.error();
    SetGltf(std::make_shared<libvrm::GltfRoot>());
    return false;
  }
}

bool
SceneState::LoadGltfString(const std::string& json)
{
  if (auto gltf = libvrm::LoadGltf(json)) {
    SetGltf(gltf);
    PLOG_INFO << "paste gltf string";
    return true;
  } else {
    // PLOG_ERROR << gltf.error();
    return false;
  }
}

void
SceneState::Update(libvrm::Time time)
{
  if (m_runtime) {
    if (m_lastTime) {
      auto delta = time - *m_lastTime;
      m_runtime->m_timeline->SetDeltaTime(delta);
      m_runtime->NextSpringDelta = delta;
    } else {
      m_runtime->m_timeline->SetDeltaTime({}, true);
    }
    m_lastTime = time;
  }
}

bool
SceneState::WriteScene(const std::filesystem::path& path)
{
  std::stringstream ss;
  gltfjson::StringSink write = [&ss](std::string_view src) mutable {
    ss.write(src.data(), src.size());
  };
  gltfjson::tree::Exporter exporter{ write };
  exporter.Export(*m_runtime->m_base->m_gltf->m_json);
  auto str = ss.str();

  std::ofstream os(path, std::ios::binary);
  if (!os) {
    return false;
  }

  return gltfjson::Glb{
    .JsonChunk = { (const uint8_t*)str.data(), str.size() },
    .BinChunk = m_runtime->m_base->m_bin.Bytes,
  }
    .WriteTo(os);
  return true;
}

std::string
SceneState::CopyVrmPoseText()
{
  if (!m_runtime) {
    return "";
  }
  return m_runtime->CopyVrmPoseText();
}
