local gltf_sample_models = os.getenv "GLTF_SAMPLE_MODELS"
print(gltf_sample_models)
vrmeditor.add_asset_dir("gltf samples", gltf_sample_models .. "/2.0")

local alicia = "../UniVRM/Tests/Models/Alicia_vrm-0.51/AliciaSolid_vrm-0.51.vrm"
local ok = vrmeditor.load_model(alicia)
print(ok, alicia)

local bvh = "../UniVRM/Assets/VRM10_Samples/VRM10Viewer/Motions/vrm10viewer_test_motion.txt"
ok = vrmeditor.load_motion(bvh, 0)
print(ok, bvh)
