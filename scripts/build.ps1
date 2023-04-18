meson setup builddir --prefix $(pwd)/prefix -Dcpp_std=c++latest
meson install -C builddir

