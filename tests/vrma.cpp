#include <gtest/gtest.h>
#include <vrm/exporter.h>
#include <vrm/glb.h>
#include <vrm/gltf_buffer.h>
#include <vrm/scene.h>

TEST(VRMA, simple)
{
  gltf::Scene scene;
  // node
  // animation

  gltf::Exporter exporter;
  exporter.Export(scene);

  gltf::Glb{
    .Json = exporter.JsonChunk,
    .Bin = exporter.BinChunk,
  }
    .WriteTo("out.vrma");

  EXPECT_TRUE(true);
}
