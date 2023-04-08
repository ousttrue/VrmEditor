---@return string home
---@return boolean is_windows
local function get_home()
  local home = os.getenv "USERPROFILE"
  if home then
    return home, true
  end
  home = os.getenv "HOME"
  if home then
    return home, false
  end
  os.exit()
end

local home, is_windows = get_home()

--
-- font settings
--
vrmeditor.set_font_size(is_windows and 22 or 16)
-- local base_font = "C:/Windows/Fonts/verdana.ttf"
-- vrmeditor.set_font(base_font)

-- local icon_font = home .. "/.fonts/HackGenNerdConsole-Regular.ttf"
local icon_font = home .. "/.fonts/Symbols-1000-em Nerd Font Complete Mono.ttf"
local japanese_font = is_windows and "C:/Windows/Fonts/msgothic.ttc" or icon_font

local ok = vrmeditor.add_japanese_font(japanese_font)
print(ok, japanese_font)

ok = vrmeditor.add_icon_font(icon_font)
print(ok, icon_font)

--
-- add assets
--
local gltf_sample_models = os.getenv "GLTF_SAMPLE_MODELS"
if gltf_sample_models then
  print(gltf_sample_models)
  ok = vrmeditor.add_asset_dir("gltf", gltf_sample_models .. "/2.0")
end

local vrm_samples = os.getenv "VRM_SAMPLES"
if vrm_samples then
  print(vrm_samples)
  ok = vrmeditor.add_asset_dir("vrm", vrm_samples)
end

local bvh_samples = os.getenv "BVH_SAMPLES"
if bvh_samples then
  print(bvh_samples)
  ok = vrmeditor.add_asset_dir("bvh", bvh_samples)
end

local fbx_samples = os.getenv "FBX_SAMPLES"
if fbx_samples then
  print(fbx_samples)
  ok = vrmeditor.add_asset_dir("fbx", fbx_samples)
end

--
-- bvh
--
vrmeditor.add_human_map {
  Hips = "hips",
  Spine = "spine",
  Spine1 = "chest",
  Neck = "neck",
  Head = "head",
  LeftShoulder = "leftShoulder",
  LeftArm = "leftUpperArm",
  LeftForeArm = "leftLowerArm",
  LeftHand = "leftHand",
  RightShoulder = "rightShoulder",
  RightArm = "rightUpperArm",
  RightForeArm = "rightLowerArm",
  RightHand = "rightHand",
  LeftUpLeg = "leftUpperLeg",
  LeftLeg = "leftLowerLeg",
  LeftFoot = "leftFoot",
  LeftToeBase = "leftToe",
  RightUpLeg = "rightUpperLeg",
  RightLeg = "rightLowerLeg",
  RightFoot = "rightFoot",
  RightToeBase = "rightToe",
}

vrmeditor.add_human_map {
  Hips = "hips",
  Ab = "spine",
  Chest = "chest",
  LeftCollar = "leftShoulder",
  LeftShoulder = "leftUpperArm",
  LeftElbow = "leftLowerArm",
  LeftWrist = "leftHand",
  Neck = "neck",
  Head = "head",
  RightCollar = "rightShoulder",
  RightShoulder = "rightUpperArm",
  RightElbow = "rightLowerArm",
  RightWrist = "rightHand",
  LeftHip = "leftUpperLeg",
  LeftKnee = "leftLowerLeg",
  LeftAnkle = "leftFoot",
  RightHip = "rightUpperLeg",
  RightKnee = "rightLowerLeg",
  RightAnkle = "rightFoot",
}

--
-- load model
--
local alicia = "../UniVRM/Tests/Models/Alicia_vrm-0.51/AliciaSolid_vrm-0.51.vrm"
ok = vrmeditor.load_model(alicia)
print(ok, alicia)

--
-- load motion
--
local bvh = "../UniVRM/Assets/VRM10_Samples/VRM10Viewer/Motions/vrm10viewer_test_motion.txt"
ok = vrmeditor.load_motion(bvh, 0)
print(ok, bvh)
