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

vrmeditor.set_font_size(is_windows and 22 or 16)
-- local base_font = "C:/Windows/Fonts/verdana.ttf"
-- vrmeditor.set_font(base_font)

local icon_font = home .. "/.fonts/HackGenNerdConsole-Regular.ttf"
local japanese_font = is_windows and "C:/Windows/Fonts/msgothic.ttc" or icon_font

local ok = vrmeditor.add_japanese_font(japanese_font)
print(ok, japanese_font)

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
