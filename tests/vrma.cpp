#include <gtest/gtest.h>
#include <vrm/animation.h>
#include <vrm/exporter.h>
#include <vrm/glb.h>
#include <vrm/gltf_buffer.h>
#include <vrm/node.h>
#include <vrm/scene.h>

TEST(VRMA, simple)
{
  gltf::Scene scene;
  // node
  scene.m_nodes.push_back(std::make_shared<gltf::Node>("node0"));
  // animation
  scene.m_animations.push_back(std::make_shared<gltf::Animation>("animation"));

  gltf::Exporter exporter;
  exporter.Export(scene);

  gltf::Glb{
    .Json = exporter.JsonChunk,
    .Bin = exporter.BinChunk,
  }
    .WriteTo("out.vrma");

  EXPECT_TRUE(true);
}
