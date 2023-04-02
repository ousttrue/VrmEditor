---@meta

---@class vrmeditor
vrmeditor = {}

---@param size integer
function vrmeditor.set_font_size(size) end

---@param path string font file path
function vrmeditor.add_japanese_font(path) end

---@param path string font file path
function vrmeditor.add_icon_font(path) end

---@param path string gltf, glb, vrm, fbx lua
function vrmeditor.load_model(path) end

---@param path string bvh
---@param scale number auto scaling if 0
function vrmeditor.load_motion(path, scale) end

---@param name string
---@param dir string to asset dir. ex. GLTF_SAMPLE_MODELS
function vrmeditor.add_asset_dir(name, dir) end
