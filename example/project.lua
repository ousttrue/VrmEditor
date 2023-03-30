vrmeditor.set_font_size(22)
-- local base_font = "C:/Windows/Fonts/verdana.ttf"
-- vrmeditor.set_font(base_font)
local japanese_font = "C:/Windows/Fonts/msgothic.ttc"
local ok = vrmeditor.add_japanese_font(japanese_font)
print(ok, japanese_font)
local icon_font = os.getenv "USERPROFILE" .. "/.fonts/HackGenNerdConsole-Regular.ttf"
ok = vrmeditor.add_icon_font(icon_font)
print(ok, icon_font)

local gltf_sample_models = os.getenv "GLTF_SAMPLE_MODELS"
print(gltf_sample_models)
ok = vrmeditor.add_asset_dir("gltf samples", gltf_sample_models .. "/2.0")

local alicia = "../UniVRM/Tests/Models/Alicia_vrm-0.51/AliciaSolid_vrm-0.51.vrm"
ok = vrmeditor.load_model(alicia)
print(ok, alicia)

local bvh = "../UniVRM/Assets/VRM10_Samples/VRM10Viewer/Motions/vrm10viewer_test_motion.txt"
ok = vrmeditor.load_motion(bvh, 0)
print(ok, bvh)
