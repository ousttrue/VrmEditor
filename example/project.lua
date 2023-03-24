local alicia = "../UniVRM/Tests/Models/Alicia_vrm-0.51/AliciaSolid_vrm-0.51.vrm"
print(alicia)
if not vrmeditor.load(alicia) then
  print "fail !"
  return
end
print "ok"
