#include <gltfjson/glb.h>
#include <gtest/gtest.h>
#include <vrm/exporter.h>
#include <vrm/gltfroot.h>
#include <vrm/node.h>
#include <vrm/vrma.h>

TEST(VRMA, simple)
{
  libvrm::gltf::GltfRoot scene;
  // node
  scene.m_nodes.push_back(std::make_shared<libvrm::gltf::Node>("node0"));
  // animation
  scene.m_animations.push_back(
    std::make_shared<libvrm::gltf::Animation>("animation"));
  auto& animation = scene.m_animations.back();
  static float times[] = {
    0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f,
  };
  static DirectX::XMFLOAT3 positions[] = {
    { 0.0f, 0.0f, 0.0f }, { 0.1f, 0.0f, 0.0f }, { 0.2f, 0.0f, 0.0f },
    { 0.3f, 0.0f, 0.0f }, { 0.4f, 0.0f, 0.0f }, { 0.5f, 0.0f, 0.0f },
    { 0.6f, 0.0f, 0.0f }, { 0.7f, 0.0f, 0.0f }, { 0.8f, 0.0f, 0.0f },
    { 0.9f, 0.0f, 0.0f },
  };
  animation->AddTranslation(0, times, positions, "track0-translation");

  // vrma
  scene.m_vrma = std::make_shared<libvrm::vrm::animation::Animation>();

  libvrm::gltf::Exporter exporter;
  exporter.Export(scene);

  std::ofstream os("out.vrma", std::ios::binary);
  assert(os);

  gltfjson::Glb{
    .JsonChunk = exporter.JsonChunk,
    .BinChunk = exporter.BinChunk,
  }
    .WriteTo(os);

  EXPECT_TRUE(true);
}
