local gltf_sample_models = os.getenv "GLTF_SAMPLE_MODELS"
print(gltf_sample_models)
vrmeditor.add_asset_dir("gltf samples", gltf_sample_models .. "/2.0")

local alicia = "../UniVRM/Tests/Models/Alicia_vrm-0.51/AliciaSolid_vrm-0.51.vrm"
print(alicia)
if not vrmeditor.load(alicia) then
  print "fail !"
  return
end
print "ok"
