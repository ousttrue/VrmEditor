#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vrm/exporter.h>
#include <gltfjson/glb.h>
#include <vrm/gltfroot.h>

// https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_003_MinimalGltfFile.md
const std::string SRC = R"({
  "scene": 0,
  "scenes" : [
    {
      "nodes" : [ 0 ]
    }
  ],
  
  "nodes" : [
    {
      "mesh" : 0
    }
  ],
  
  "meshes" : [
    {
      "primitives" : [ {
        "attributes" : {
          "POSITION" : 1
        },
        "indices" : 0
      } ]
    }
  ],

  "buffers" : [
    {
      "uri" : "data:application/octet-stream;base64,AAABAAIAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAACAPwAAAAA=",
      "byteLength" : 44
    }
  ],
  "bufferViews" : [
    {
      "buffer" : 0,
      "byteOffset" : 0,
      "byteLength" : 6,
      "target" : 34963
    },
    {
      "buffer" : 0,
      "byteOffset" : 8,
      "byteLength" : 36,
      "target" : 34962
    }
  ],
  "accessors" : [
    {
      "bufferView" : 0,
      "byteOffset" : 0,
      "componentType" : 5123,
      "count" : 3,
      "type" : "SCALAR",
      "max" : [ 2 ],
      "min" : [ 0 ]
    },
    {
      "bufferView" : 1,
      "byteOffset" : 0,
      "componentType" : 5126,
      "count" : 3,
      "type" : "VEC3",
      "max" : [ 1.0, 1.0, 0.0 ],
      "min" : [ 0.0, 0.0, 0.0 ]
    }
  ],
  
  "asset" : {
    "version" : "2.0"
  }
})";

TEST(GlbExport, minimal)
{
  auto src = nlohmann::json::parse(SRC);
  src["buffers"].clear();
  for (auto& b : src.at("bufferViews")) {
    b.erase("target");
  }
  for (auto& a : src.at("accessors")) {
    a.erase("max");
    a.erase("min");
  }

  libvrm::gltf::GltfRoot scene;
  std::span<const uint8_t> span{ (const uint8_t*)SRC.data(),
                                 (const uint8_t*)SRC.data() + SRC.size() };
  // EXPECT_TRUE(scene.Load(span));
  libvrm::gltf::Exporter exporter;
  exporter.Export(scene);
  auto dst = nlohmann::json::parse(exporter.JsonChunk);
  dst["buffers"].clear();
  for (auto& b : dst.at("bufferViews")) {
    b.erase("target");
  }
  for (auto& a : dst.at("accessors")) {
    a.erase("max");
    a.erase("min");
  }

  // // DEBUG
  // gltf::Glb{
  //   .Json = exporter.JsonChunk,
  //   .Bin = exporter.BinChunk,
  // }
  //   .WriteTo("minimal.glb");

  EXPECT_EQ(src, dst);
}
