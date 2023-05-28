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
  LeftToeBase = "leftToes",
  RightUpLeg = "rightUpperLeg",
  RightLeg = "rightLowerLeg",
  RightFoot = "rightFoot",
  RightToeBase = "rightToes",
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

vrmeditor.add_human_map {
  Hips = "hips",
  Chest = "spine",
  Chest2 = "chest",
  Chest3 = "--nothing--",
  Chest4 = "upperChest",
  Neck = "neck",
  Head = "head",
  RightCollar = "rightShoulder",
  RightShoulder = "rightUpperArm",
  RightElbow = "rightLowerArm",
  RightWrist = "rightHand",
  LeftCollar = "leftShoulder",
  LeftShoulder = "leftUpperArm",
  LeftElbow = "leftLowerArm",
  LeftWrist = "leftHand",
  RightHip = "rightUpperLeg",
  RightKnee = "rightLowerLeg",
  RightAnkle = "rightFoot",
  RightToe = "rightToes",
  LeftHip = "leftUpperLeg",
  LeftKnee = "leftLowerLeg",
  LeftAnkle = "leftFoot",
  LeftToe = "leftToes",
}

vrmeditor.add_human_map {
  hip = "hips",
  abdomen = "spine",
  chest = "chest",
  neck = "neck",
  head = "head",
  leftEye = "leftEye",
  rightEye = "rightEye",
  rCollar = "rightShoulder",
  rShldr = "rightUpperArm",
  rForeArm = "rightLowerArm",
  rHand = "rightHand",
  rThumb1 = "rightThumbMetacarpal",
  rThumb2 = "rightThumbProximal",
  rIndex1 = "rightIndexProximal",
  rIndex2 = "rightIndexIntermediate",
  rMid1 = "rightMiddleProximal",
  rMid2 = "rightMiddleIntermediate",
  rRing1 = "rightRingProximal",
  rRing2 = "rightRingIntermediate",
  rPinky1 = "rightLittleProximal",
  rPinky2 = "rightLittleIntermediate",
  lCollar = "leftShoulder",
  lShldr = "leftUpperArm",
  lForeArm = "leftLowerArm",
  lHand = "leftHand",
  lThumb1 = "leftThumbMetacarpal",
  lThumb2 = "leftThumbProximal",
  lIndex1 = "leftIndexProximal",
  lIndex2 = "leftIndexIntermediate",
  lMid1 = "leftMiddleProximal",
  lMid2 = "leftMiddleIntermediate",
  lRing1 = "leftRingProximal",
  lRing2 = "leftRingIntermediate",
  lPinky1 = "leftLittleProximal",
  lPinky2 = "leftLittleIntermediate",
  rButtock = "--nothing--",
  rThigh = "rightUpperLeg",
  rShin = "rightLowerLeg",
  rFoot = "rightFoot",
  lButtock = "--nothing--",
  lThigh = "leftUpperLeg",
  lShin = "leftLowerLeg",
  lFoot = "leftFoot",
}

vrmeditor.add_human_map {
  root = "hips",
  torso_1 = "--nothing--",
  torso_2 = "--nothing--",
  torso_3 = "spine",
  torso_4 = "--nothing--",
  torso_5 = "chest",
  torso_6 = "upperChest",
  torso_7 = "--nothing--",
  neck_1 = "neck",
  neck_2 = "--nothing--",
  head = "head",
  l_shoulder = "leftShoulder",
  l_up_arm = "leftUpperArm",
  l_low_arm = "leftLowerArm",
  l_hand = "leftHand",
  r_shoulder = "rightShoulder",
  r_up_arm = "rightUpperArm",
  r_low_arm = "rightLowerArm",
  r_hand = "rightHand",
  l_up_leg = "leftUpperLeg",
  l_low_leg = "leftLowerLeg",
  l_foot = "leftFoot",
  l_toes = "leftToes",
  r_up_leg = "rightUpperLeg",
  r_low_leg = "rightLowerLeg",
  r_foot = "rightFoot",
  r_toes = "rightToes",
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

--
-- shader env
--
local self = string.sub(debug.getinfo(1).source, 2)
self = string.gsub(self, "\\", "/")

local function split(src, delimiter)
  local result = {}
  local i = 1
  while true do
    local found = string.find(src, delimiter, i)
    if found then
      local d = found - i
      if d > 0 then
        table.insert(result, string.sub(src, i, found - 1))
      end
      i = found + 1
    else
      break
    end
  end
  local d = #src - i
  if d > 0 then
    table.insert(result, string.sub(src, i))
  end

  return result
end

local function concat(src, delimiter)
  local dst = ""
  for i, s in ipairs(src) do
    if i > 1 then
      dst = dst .. delimiter
    end
    dst = dst .. s
  end
  return dst
end

-- print(self)
local splitted = split(self, "/")
table.remove(splitted, #splitted)
local here = concat(splitted, "/")
-- print(here)

vrmeditor.set_shaderpath(here .. "/shaders")
vrmeditor.set_shader_chunk_path(here .. "/threejs_shader_chunks")
