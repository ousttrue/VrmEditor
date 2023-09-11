import site
import pathlib

for p in site.getsitepackages():
    dir = pathlib.Path(p)
    zig_dir = dir / "ziglang"
    if zig_dir.exists():
        print(zig_dir)
