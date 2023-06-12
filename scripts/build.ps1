meson setup builddir --prefix "$(pwd)/vrmeditor" --buildtype release -Dcpp_std=c++latest -Dexecutables=true
meson install -C builddir --tags runtime

# Compress-Archive -Path "$(pwd)/vrmeditor" -DestinationPath "vrmeditor.zip"

iscc vrmeditor.iss /Fvrmeditor_setup

