meson setup builddir --prefix "$(pwd)/prefix" --buildtype release -Dcpp_std=c++latest -Dexecutables=true
meson install -C builddir --tags runtime

