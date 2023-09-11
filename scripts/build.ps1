meson setup builddir --prefix "$(pwd)/prefix" --buildtype release --native-file zig.ini -Dcpp_std=c++20 -Dexecutables=true
meson install -C builddir --tags runtime

# Compress-Archive -Path "$(pwd)/vrmeditor" -DestinationPath "vrmeditor.zip"

# iscc vrmeditor.iss /Fvrmeditor
# => Output/vrmeditor.exe

copy vrmeditor.xml vrmeditor.wxs
wix convert vrmeditor.wxs
msbuild /restore
msbuild vrmeditor.wixproj /p:Configuration=Release
# => obj/Release/vrmeditor.msi

