
#include <cstdlib>
#include <gtest/gtest.h>
#include <vrm/scene.h>

std::filesystem::path get_path(std::string_view relative) {
  std::filesystem::path base = std::getenv("GLTF_SAMPLE_MODELS");
  return base / relative;
}


TEST(VrmLoad, 2CylinderEngine_glTF) {
  auto path = get_path("2CylinderEngine/glTF/2CylinderEngine.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, 2CylinderEngine_glTF_Binary) {
  auto path = get_path("2CylinderEngine/glTF-Binary/2CylinderEngine.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, 2CylinderEngine_glTF_Draco) {
  auto path = get_path("2CylinderEngine/glTF-Draco/2CylinderEngine.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, 2CylinderEngine_glTF_Embedded) {
  auto path = get_path("2CylinderEngine/glTF-Embedded/2CylinderEngine.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ABeautifulGame_glTF) {
  auto path = get_path("ABeautifulGame/glTF/ABeautifulGame.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AlphaBlendModeTest_glTF) {
  auto path = get_path("AlphaBlendModeTest/glTF/AlphaBlendModeTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AlphaBlendModeTest_glTF_Binary) {
  auto path = get_path("AlphaBlendModeTest/glTF-Binary/AlphaBlendModeTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AlphaBlendModeTest_glTF_Embedded) {
  auto path = get_path("AlphaBlendModeTest/glTF-Embedded/AlphaBlendModeTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedCube_glTF) {
  auto path = get_path("AnimatedCube/glTF/AnimatedCube.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedMorphCube_glTF) {
  auto path = get_path("AnimatedMorphCube/glTF/AnimatedMorphCube.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedMorphCube_glTF_Binary) {
  auto path = get_path("AnimatedMorphCube/glTF-Binary/AnimatedMorphCube.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedMorphCube_glTF_Quantized) {
  auto path = get_path("AnimatedMorphCube/glTF-Quantized/AnimatedMorphCube.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedMorphSphere_glTF) {
  auto path = get_path("AnimatedMorphSphere/glTF/AnimatedMorphSphere.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedMorphSphere_glTF_Binary) {
  auto path = get_path("AnimatedMorphSphere/glTF-Binary/AnimatedMorphSphere.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedTriangle_glTF) {
  auto path = get_path("AnimatedTriangle/glTF/AnimatedTriangle.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AnimatedTriangle_glTF_Embedded) {
  auto path = get_path("AnimatedTriangle/glTF-Embedded/AnimatedTriangle.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AntiqueCamera_glTF) {
  auto path = get_path("AntiqueCamera/glTF/AntiqueCamera.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AntiqueCamera_glTF_Binary) {
  auto path = get_path("AntiqueCamera/glTF-Binary/AntiqueCamera.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AttenuationTest_glTF) {
  auto path = get_path("AttenuationTest/glTF/AttenuationTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, AttenuationTest_glTF_Binary) {
  auto path = get_path("AttenuationTest/glTF-Binary/AttenuationTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Avocado_glTF) {
  auto path = get_path("Avocado/glTF/Avocado.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Avocado_glTF_Binary) {
  auto path = get_path("Avocado/glTF-Binary/Avocado.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Avocado_glTF_Draco) {
  auto path = get_path("Avocado/glTF-Draco/Avocado.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Avocado_glTF_Quantized) {
  auto path = get_path("Avocado/glTF-Quantized/Avocado.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BarramundiFish_glTF) {
  auto path = get_path("BarramundiFish/glTF/BarramundiFish.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BarramundiFish_glTF_Binary) {
  auto path = get_path("BarramundiFish/glTF-Binary/BarramundiFish.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BarramundiFish_glTF_Draco) {
  auto path = get_path("BarramundiFish/glTF-Draco/BarramundiFish.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoomBox_glTF) {
  auto path = get_path("BoomBox/glTF/BoomBox.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoomBox_glTF_Binary) {
  auto path = get_path("BoomBox/glTF-Binary/BoomBox.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoomBox_glTF_Draco) {
  auto path = get_path("BoomBox/glTF-Draco/BoomBox.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoomBoxWithAxes_glTF) {
  auto path = get_path("BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Box_glTF) {
  auto path = get_path("Box/glTF/Box.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Box_glTF_Binary) {
  auto path = get_path("Box/glTF-Binary/Box.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Box_glTF_Draco) {
  auto path = get_path("Box/glTF-Draco/Box.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Box_glTF_Embedded) {
  auto path = get_path("Box/glTF-Embedded/Box.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Box_With_Spaces_glTF) {
  auto path = get_path("Box With Spaces/glTF/Box With Spaces.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxAnimated_glTF) {
  auto path = get_path("BoxAnimated/glTF/BoxAnimated.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxAnimated_glTF_Binary) {
  auto path = get_path("BoxAnimated/glTF-Binary/BoxAnimated.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxAnimated_glTF_Embedded) {
  auto path = get_path("BoxAnimated/glTF-Embedded/BoxAnimated.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxInterleaved_glTF) {
  auto path = get_path("BoxInterleaved/glTF/BoxInterleaved.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxInterleaved_glTF_Binary) {
  auto path = get_path("BoxInterleaved/glTF-Binary/BoxInterleaved.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxInterleaved_glTF_Embedded) {
  auto path = get_path("BoxInterleaved/glTF-Embedded/BoxInterleaved.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxTextured_glTF) {
  auto path = get_path("BoxTextured/glTF/BoxTextured.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxTextured_glTF_Binary) {
  auto path = get_path("BoxTextured/glTF-Binary/BoxTextured.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxTextured_glTF_Embedded) {
  auto path = get_path("BoxTextured/glTF-Embedded/BoxTextured.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxTexturedNonPowerOfTwo_glTF) {
  auto path = get_path("BoxTexturedNonPowerOfTwo/glTF/BoxTexturedNonPowerOfTwo.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxTexturedNonPowerOfTwo_glTF_Binary) {
  auto path = get_path("BoxTexturedNonPowerOfTwo/glTF-Binary/BoxTexturedNonPowerOfTwo.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxTexturedNonPowerOfTwo_glTF_Embedded) {
  auto path = get_path("BoxTexturedNonPowerOfTwo/glTF-Embedded/BoxTexturedNonPowerOfTwo.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxVertexColors_glTF) {
  auto path = get_path("BoxVertexColors/glTF/BoxVertexColors.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxVertexColors_glTF_Binary) {
  auto path = get_path("BoxVertexColors/glTF-Binary/BoxVertexColors.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BoxVertexColors_glTF_Embedded) {
  auto path = get_path("BoxVertexColors/glTF-Embedded/BoxVertexColors.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BrainStem_glTF) {
  auto path = get_path("BrainStem/glTF/BrainStem.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BrainStem_glTF_Binary) {
  auto path = get_path("BrainStem/glTF-Binary/BrainStem.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BrainStem_glTF_Draco) {
  auto path = get_path("BrainStem/glTF-Draco/BrainStem.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, BrainStem_glTF_Embedded) {
  auto path = get_path("BrainStem/glTF-Embedded/BrainStem.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Buggy_glTF) {
  auto path = get_path("Buggy/glTF/Buggy.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Buggy_glTF_Binary) {
  auto path = get_path("Buggy/glTF-Binary/Buggy.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Buggy_glTF_Draco) {
  auto path = get_path("Buggy/glTF-Draco/Buggy.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Buggy_glTF_Embedded) {
  auto path = get_path("Buggy/glTF-Embedded/Buggy.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Cameras_glTF) {
  auto path = get_path("Cameras/glTF/Cameras.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Cameras_glTF_Embedded) {
  auto path = get_path("Cameras/glTF-Embedded/Cameras.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMan_glTF) {
  auto path = get_path("CesiumMan/glTF/CesiumMan.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMan_glTF_Binary) {
  auto path = get_path("CesiumMan/glTF-Binary/CesiumMan.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMan_glTF_Draco) {
  auto path = get_path("CesiumMan/glTF-Draco/CesiumMan.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMan_glTF_Embedded) {
  auto path = get_path("CesiumMan/glTF-Embedded/CesiumMan.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMilkTruck_glTF) {
  auto path = get_path("CesiumMilkTruck/glTF/CesiumMilkTruck.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMilkTruck_glTF_Binary) {
  auto path = get_path("CesiumMilkTruck/glTF-Binary/CesiumMilkTruck.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMilkTruck_glTF_Draco) {
  auto path = get_path("CesiumMilkTruck/glTF-Draco/CesiumMilkTruck.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, CesiumMilkTruck_glTF_Embedded) {
  auto path = get_path("CesiumMilkTruck/glTF-Embedded/CesiumMilkTruck.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ClearCoatTest_glTF) {
  auto path = get_path("ClearCoatTest/glTF/ClearCoatTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ClearCoatTest_glTF_Binary) {
  auto path = get_path("ClearCoatTest/glTF-Binary/ClearCoatTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Corset_glTF) {
  auto path = get_path("Corset/glTF/Corset.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Corset_glTF_Binary) {
  auto path = get_path("Corset/glTF-Binary/Corset.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Corset_glTF_Draco) {
  auto path = get_path("Corset/glTF-Draco/Corset.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Cube_glTF) {
  auto path = get_path("Cube/glTF/Cube.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, DamagedHelmet_glTF) {
  auto path = get_path("DamagedHelmet/glTF/DamagedHelmet.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, DamagedHelmet_glTF_Binary) {
  auto path = get_path("DamagedHelmet/glTF-Binary/DamagedHelmet.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, DamagedHelmet_glTF_Embedded) {
  auto path = get_path("DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, DragonAttenuation_glTF) {
  auto path = get_path("DragonAttenuation/glTF/DragonAttenuation.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, DragonAttenuation_glTF_Binary) {
  auto path = get_path("DragonAttenuation/glTF-Binary/DragonAttenuation.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Duck_glTF) {
  auto path = get_path("Duck/glTF/Duck.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Duck_glTF_Binary) {
  auto path = get_path("Duck/glTF-Binary/Duck.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Duck_glTF_Draco) {
  auto path = get_path("Duck/glTF-Draco/Duck.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Duck_glTF_Embedded) {
  auto path = get_path("Duck/glTF-Embedded/Duck.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Duck_glTF_Quantized) {
  auto path = get_path("Duck/glTF-Quantized/Duck.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, EmissiveStrengthTest_glTF) {
  auto path = get_path("EmissiveStrengthTest/glTF/EmissiveStrengthTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, EmissiveStrengthTest_glTF_Binary) {
  auto path = get_path("EmissiveStrengthTest/glTF-Binary/EmissiveStrengthTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, EnvironmentTest_glTF) {
  auto path = get_path("EnvironmentTest/glTF/EnvironmentTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, EnvironmentTest_glTF_IBL) {
  auto path = get_path("EnvironmentTest/glTF-IBL/EnvironmentTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, FlightHelmet_glTF) {
  auto path = get_path("FlightHelmet/glTF/FlightHelmet.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Fox_glTF) {
  auto path = get_path("Fox/glTF/Fox.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Fox_glTF_Binary) {
  auto path = get_path("Fox/glTF-Binary/Fox.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, GearboxAssy_glTF) {
  auto path = get_path("GearboxAssy/glTF/GearboxAssy.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, GearboxAssy_glTF_Binary) {
  auto path = get_path("GearboxAssy/glTF-Binary/GearboxAssy.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, GearboxAssy_glTF_Draco) {
  auto path = get_path("GearboxAssy/glTF-Draco/GearboxAssy.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, GearboxAssy_glTF_Embedded) {
  auto path = get_path("GearboxAssy/glTF-Embedded/GearboxAssy.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, GlamVelvetSofa_glTF) {
  auto path = get_path("GlamVelvetSofa/glTF/GlamVelvetSofa.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, GlamVelvetSofa_glTF_Binary) {
  auto path = get_path("GlamVelvetSofa/glTF-Binary/GlamVelvetSofa.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, InterpolationTest_glTF) {
  auto path = get_path("InterpolationTest/glTF/InterpolationTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, InterpolationTest_glTF_Binary) {
  auto path = get_path("InterpolationTest/glTF-Binary/InterpolationTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescenceDielectricSpheres_glTF) {
  auto path = get_path("IridescenceDielectricSpheres/glTF/IridescenceDielectricSpheres.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescenceLamp_glTF) {
  auto path = get_path("IridescenceLamp/glTF/IridescenceLamp.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescenceLamp_glTF_Binary) {
  auto path = get_path("IridescenceLamp/glTF-Binary/IridescenceLamp.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescenceMetallicSpheres_glTF) {
  auto path = get_path("IridescenceMetallicSpheres/glTF/IridescenceMetallicSpheres.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescenceSuzanne_glTF) {
  auto path = get_path("IridescenceSuzanne/glTF/IridescenceSuzanne.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescenceSuzanne_glTF_Binary) {
  auto path = get_path("IridescenceSuzanne/glTF-Binary/IridescenceSuzanne.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescentDishWithOlives_glTF) {
  auto path = get_path("IridescentDishWithOlives/glTF/IridescentDishWithOlives.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, IridescentDishWithOlives_glTF_Binary) {
  auto path = get_path("IridescentDishWithOlives/glTF-Binary/IridescentDishWithOlives.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Lantern_glTF) {
  auto path = get_path("Lantern/glTF/Lantern.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Lantern_glTF_Binary) {
  auto path = get_path("Lantern/glTF-Binary/Lantern.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Lantern_glTF_Draco) {
  auto path = get_path("Lantern/glTF-Draco/Lantern.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Lantern_glTF_Quantized) {
  auto path = get_path("Lantern/glTF-Quantized/Lantern.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, LightsPunctualLamp_glTF) {
  auto path = get_path("LightsPunctualLamp/glTF/LightsPunctualLamp.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, LightsPunctualLamp_glTF_Binary) {
  auto path = get_path("LightsPunctualLamp/glTF-Binary/LightsPunctualLamp.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MaterialsVariantsShoe_glTF) {
  auto path = get_path("MaterialsVariantsShoe/glTF/MaterialsVariantsShoe.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MaterialsVariantsShoe_glTF_Binary) {
  auto path = get_path("MaterialsVariantsShoe/glTF-Binary/MaterialsVariantsShoe.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MetalRoughSpheres_glTF) {
  auto path = get_path("MetalRoughSpheres/glTF/MetalRoughSpheres.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MetalRoughSpheres_glTF_Binary) {
  auto path = get_path("MetalRoughSpheres/glTF-Binary/MetalRoughSpheres.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MetalRoughSpheres_glTF_Embedded) {
  auto path = get_path("MetalRoughSpheres/glTF-Embedded/MetalRoughSpheres.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MetalRoughSpheresNoTextures_glTF) {
  auto path = get_path("MetalRoughSpheresNoTextures/glTF/MetalRoughSpheresNoTextures.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MetalRoughSpheresNoTextures_glTF_Binary) {
  auto path = get_path("MetalRoughSpheresNoTextures/glTF-Binary/MetalRoughSpheresNoTextures.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MorphPrimitivesTest_glTF) {
  auto path = get_path("MorphPrimitivesTest/glTF/MorphPrimitivesTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MorphPrimitivesTest_glTF_Binary) {
  auto path = get_path("MorphPrimitivesTest/glTF-Binary/MorphPrimitivesTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MorphPrimitivesTest_glTF_Draco) {
  auto path = get_path("MorphPrimitivesTest/glTF-Draco/MorphPrimitivesTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MorphStressTest_glTF) {
  auto path = get_path("MorphStressTest/glTF/MorphStressTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MorphStressTest_glTF_Binary) {
  auto path = get_path("MorphStressTest/glTF-Binary/MorphStressTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MosquitoInAmber_glTF) {
  auto path = get_path("MosquitoInAmber/glTF/MosquitoInAmber.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MosquitoInAmber_glTF_Binary) {
  auto path = get_path("MosquitoInAmber/glTF-Binary/MosquitoInAmber.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MultiUVTest_glTF) {
  auto path = get_path("MultiUVTest/glTF/MultiUVTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MultiUVTest_glTF_Binary) {
  auto path = get_path("MultiUVTest/glTF-Binary/MultiUVTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, MultiUVTest_glTF_Embedded) {
  auto path = get_path("MultiUVTest/glTF-Embedded/MultiUVTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, NormalTangentMirrorTest_glTF) {
  auto path = get_path("NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, NormalTangentMirrorTest_glTF_Binary) {
  auto path = get_path("NormalTangentMirrorTest/glTF-Binary/NormalTangentMirrorTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, NormalTangentMirrorTest_glTF_Embedded) {
  auto path = get_path("NormalTangentMirrorTest/glTF-Embedded/NormalTangentMirrorTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, NormalTangentTest_glTF) {
  auto path = get_path("NormalTangentTest/glTF/NormalTangentTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, NormalTangentTest_glTF_Binary) {
  auto path = get_path("NormalTangentTest/glTF-Binary/NormalTangentTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, NormalTangentTest_glTF_Embedded) {
  auto path = get_path("NormalTangentTest/glTF-Embedded/NormalTangentTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, OrientationTest_glTF) {
  auto path = get_path("OrientationTest/glTF/OrientationTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, OrientationTest_glTF_Binary) {
  auto path = get_path("OrientationTest/glTF-Binary/OrientationTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, OrientationTest_glTF_Embedded) {
  auto path = get_path("OrientationTest/glTF-Embedded/OrientationTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ReciprocatingSaw_glTF) {
  auto path = get_path("ReciprocatingSaw/glTF/ReciprocatingSaw.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ReciprocatingSaw_glTF_Binary) {
  auto path = get_path("ReciprocatingSaw/glTF-Binary/ReciprocatingSaw.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ReciprocatingSaw_glTF_Draco) {
  auto path = get_path("ReciprocatingSaw/glTF-Draco/ReciprocatingSaw.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ReciprocatingSaw_glTF_Embedded) {
  auto path = get_path("ReciprocatingSaw/glTF-Embedded/ReciprocatingSaw.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RecursiveSkeletons_glTF) {
  auto path = get_path("RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RecursiveSkeletons_glTF_Binary) {
  auto path = get_path("RecursiveSkeletons/glTF-Binary/RecursiveSkeletons.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedFigure_glTF) {
  auto path = get_path("RiggedFigure/glTF/RiggedFigure.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedFigure_glTF_Binary) {
  auto path = get_path("RiggedFigure/glTF-Binary/RiggedFigure.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedFigure_glTF_Draco) {
  auto path = get_path("RiggedFigure/glTF-Draco/RiggedFigure.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedFigure_glTF_Embedded) {
  auto path = get_path("RiggedFigure/glTF-Embedded/RiggedFigure.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedSimple_glTF) {
  auto path = get_path("RiggedSimple/glTF/RiggedSimple.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedSimple_glTF_Binary) {
  auto path = get_path("RiggedSimple/glTF-Binary/RiggedSimple.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedSimple_glTF_Draco) {
  auto path = get_path("RiggedSimple/glTF-Draco/RiggedSimple.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, RiggedSimple_glTF_Embedded) {
  auto path = get_path("RiggedSimple/glTF-Embedded/RiggedSimple.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SciFiHelmet_glTF) {
  auto path = get_path("SciFiHelmet/glTF/SciFiHelmet.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SheenChair_glTF) {
  auto path = get_path("SheenChair/glTF/SheenChair.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SheenChair_glTF_Binary) {
  auto path = get_path("SheenChair/glTF-Binary/SheenChair.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SheenCloth_glTF) {
  auto path = get_path("SheenCloth/glTF/SheenCloth.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleMeshes_glTF) {
  auto path = get_path("SimpleMeshes/glTF/SimpleMeshes.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleMeshes_glTF_Embedded) {
  auto path = get_path("SimpleMeshes/glTF-Embedded/SimpleMeshes.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleMorph_glTF) {
  auto path = get_path("SimpleMorph/glTF/SimpleMorph.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleMorph_glTF_Embedded) {
  auto path = get_path("SimpleMorph/glTF-Embedded/SimpleMorph.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleSkin_glTF) {
  auto path = get_path("SimpleSkin/glTF/SimpleSkin.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleSkin_glTF_Embedded) {
  auto path = get_path("SimpleSkin/glTF-Embedded/SimpleSkin.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleSparseAccessor_glTF) {
  auto path = get_path("SimpleSparseAccessor/glTF/SimpleSparseAccessor.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SimpleSparseAccessor_glTF_Embedded) {
  auto path = get_path("SimpleSparseAccessor/glTF-Embedded/SimpleSparseAccessor.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SpecGlossVsMetalRough_glTF) {
  auto path = get_path("SpecGlossVsMetalRough/glTF/SpecGlossVsMetalRough.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SpecGlossVsMetalRough_glTF_Binary) {
  auto path = get_path("SpecGlossVsMetalRough/glTF-Binary/SpecGlossVsMetalRough.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SpecularTest_glTF) {
  auto path = get_path("SpecularTest/glTF/SpecularTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, SpecularTest_glTF_Binary) {
  auto path = get_path("SpecularTest/glTF-Binary/SpecularTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Sponza_glTF) {
  auto path = get_path("Sponza/glTF/Sponza.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, StainedGlassLamp_glTF) {
  auto path = get_path("StainedGlassLamp/glTF/StainedGlassLamp.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, StainedGlassLamp_glTF_JPG_PNG) {
  auto path = get_path("StainedGlassLamp/glTF-JPG-PNG/StainedGlassLamp.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, StainedGlassLamp_glTF_KTX_BasisU) {
  auto path = get_path("StainedGlassLamp/glTF-KTX-BasisU/StainedGlassLamp.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Suzanne_glTF) {
  auto path = get_path("Suzanne/glTF/Suzanne.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureCoordinateTest_glTF) {
  auto path = get_path("TextureCoordinateTest/glTF/TextureCoordinateTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureCoordinateTest_glTF_Binary) {
  auto path = get_path("TextureCoordinateTest/glTF-Binary/TextureCoordinateTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureCoordinateTest_glTF_Embedded) {
  auto path = get_path("TextureCoordinateTest/glTF-Embedded/TextureCoordinateTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureEncodingTest_glTF) {
  auto path = get_path("TextureEncodingTest/glTF/TextureEncodingTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureEncodingTest_glTF_Binary) {
  auto path = get_path("TextureEncodingTest/glTF-Binary/TextureEncodingTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureLinearInterpolationTest_glTF) {
  auto path = get_path("TextureLinearInterpolationTest/glTF/TextureLinearInterpolationTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureLinearInterpolationTest_glTF_Binary) {
  auto path = get_path("TextureLinearInterpolationTest/glTF-Binary/TextureLinearInterpolationTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureSettingsTest_glTF) {
  auto path = get_path("TextureSettingsTest/glTF/TextureSettingsTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureSettingsTest_glTF_Binary) {
  auto path = get_path("TextureSettingsTest/glTF-Binary/TextureSettingsTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureSettingsTest_glTF_Embedded) {
  auto path = get_path("TextureSettingsTest/glTF-Embedded/TextureSettingsTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureTransformMultiTest_glTF) {
  auto path = get_path("TextureTransformMultiTest/glTF/TextureTransformMultiTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureTransformMultiTest_glTF_Binary) {
  auto path = get_path("TextureTransformMultiTest/glTF-Binary/TextureTransformMultiTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TextureTransformTest_glTF) {
  auto path = get_path("TextureTransformTest/glTF/TextureTransformTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ToyCar_glTF) {
  auto path = get_path("ToyCar/glTF/ToyCar.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, ToyCar_glTF_Binary) {
  auto path = get_path("ToyCar/glTF-Binary/ToyCar.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TransmissionRoughnessTest_glTF) {
  auto path = get_path("TransmissionRoughnessTest/glTF/TransmissionRoughnessTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TransmissionRoughnessTest_glTF_Binary) {
  auto path = get_path("TransmissionRoughnessTest/glTF-Binary/TransmissionRoughnessTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TransmissionTest_glTF) {
  auto path = get_path("TransmissionTest/glTF/TransmissionTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TransmissionTest_glTF_Binary) {
  auto path = get_path("TransmissionTest/glTF-Binary/TransmissionTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Triangle_glTF) {
  auto path = get_path("Triangle/glTF/Triangle.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, Triangle_glTF_Embedded) {
  auto path = get_path("Triangle/glTF-Embedded/Triangle.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TriangleWithoutIndices_glTF) {
  auto path = get_path("TriangleWithoutIndices/glTF/TriangleWithoutIndices.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TriangleWithoutIndices_glTF_Embedded) {
  auto path = get_path("TriangleWithoutIndices/glTF-Embedded/TriangleWithoutIndices.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, TwoSidedPlane_glTF) {
  auto path = get_path("TwoSidedPlane/glTF/TwoSidedPlane.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, UnlitTest_glTF) {
  auto path = get_path("UnlitTest/glTF/UnlitTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, UnlitTest_glTF_Binary) {
  auto path = get_path("UnlitTest/glTF-Binary/UnlitTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, VC_glTF) {
  auto path = get_path("VC/glTF/VC.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, VC_glTF_Binary) {
  auto path = get_path("VC/glTF-Binary/VC.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, VC_glTF_Draco) {
  auto path = get_path("VC/glTF-Draco/VC.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, VC_glTF_Embedded) {
  auto path = get_path("VC/glTF-Embedded/VC.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, VertexColorTest_glTF) {
  auto path = get_path("VertexColorTest/glTF/VertexColorTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, VertexColorTest_glTF_Binary) {
  auto path = get_path("VertexColorTest/glTF-Binary/VertexColorTest.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, VertexColorTest_glTF_Embedded) {
  auto path = get_path("VertexColorTest/glTF-Embedded/VertexColorTest.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, WaterBottle_glTF) {
  auto path = get_path("WaterBottle/glTF/WaterBottle.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, WaterBottle_glTF_Binary) {
  auto path = get_path("WaterBottle/glTF-Binary/WaterBottle.glb");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}

TEST(VrmLoad, WaterBottle_glTF_Draco) {
  auto path = get_path("WaterBottle/glTF-Draco/WaterBottle.gltf");
  Scene scene;
  EXPECT_TRUE(scene.Load(path));
}
