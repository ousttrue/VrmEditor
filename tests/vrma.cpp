#include <gtest/gtest.h>
#include <vrm/animation.h>
#include <vrm/exporter.h>
#include <vrm/glb.h>
#include <vrm/gltf_buffer.h>
#include <vrm/node.h>
#include <vrm/scene.h>
#include <vrm/vrma.h>

TEST(VRMA, simple)
{
  gltf::Scene scene;
  // node
  scene.m_nodes.push_back(std::make_shared<gltf::Node>("node0"));
  // animation
  scene.m_animations.push_back(std::make_shared<gltf::Animation>("animation"));
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
  scene.m_vrma = std::make_shared<vrm::animation::Animation>();

  gltf::Exporter exporter;
  exporter.Export(scene);

  gltf::Glb{
    .Json = exporter.JsonChunk,
    .Bin = exporter.BinChunk,
  }
    .WriteTo("out.vrma");

  EXPECT_TRUE(true);
}
