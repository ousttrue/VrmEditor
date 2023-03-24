---@meta

---@class vrmeditor
vrmeditor = {}

---@param path string gltf, glb, vrm, fbx bvh, lua
function vrmeditor.load(path) end

---@param name string
---@param dir string to asset dir. ex. GLTF_SAMPLE_MODELS
function vrmeditor.add_asset_dir(name, dir) end
