meson setup builddir --prefix "$(pwd)/prefix" -Dcpp_std=c++latest -Dexecutables=true
meson install -C builddir

