---@meta

---@class vrmeditor
vrmeditor = {}

---@param ini string imgui ini
function vrmeditor.load_imgui_ini(ini) end

---@param ini string imnodes ini
function vrmeditor.load_imnodes_ini(ini) end

---@param links table imnodes link{id, start, end}
function vrmeditor.load_imnodes_links(links) end

---@param width integer
---@param height integer
---@param is_maximized boolean
function vrmeditor.set_window_size(width, height, is_maximized) end

---@param size integer
function vrmeditor.set_font_size(size) end

---@param path string font file path
---@param type string font type
function vrmeditor.add_font(path, type) end

---@param path string gltf, glb, vrm, fbx lua
function vrmeditor.load_model(path) end

---@param path string bvh
---@param scale number auto scaling if 0
function vrmeditor.load_motion(path, scale) end

---@param name string
---@param dir string to asset dir. ex. GLTF_SAMPLE_MODELS
function vrmeditor.add_asset_dir(name, dir) end

---@param map table
function vrmeditor.add_human_map(map) end
