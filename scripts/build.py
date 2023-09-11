from typing import Optional
import pathlib
import site
import pathlib
import logging
import subprocess
import os
import shutil
import io

LOGGER = logging.getLogger(__name__)
HERE = pathlib.Path(__file__).parent
ROOT = HERE.parent


def get_package_dir(name: str) -> Optional[pathlib.Path]:
    for p in site.getsitepackages():
        dir = pathlib.Path(p)
        fullpath = dir / name
        if fullpath.exists():
            return fullpath
    raise Exception(f"{name} not found")


def system(cmd: str):
    if subprocess.call(cmd, shell=True) < 0:
        raise Exception(cmd)


# https://github.com/mesonbuild/meson/pull/11918/files
MESON_ZIG_PATCH_ALREADY = """elif o.startswith('zig ld '):"""

MESON_ZIG_PATCH = """
    elif o.startswith('zig ld '):
        linker = linkers.LLVMDynamicLinker(
            compiler, for_machine, comp_class.LINKER_PREFIX, override, version=v)
"""

MESON_ZIG_PATCH_INSERT = """elif e.startswith('lld-link: '):"""


def patch_meson(mesonbuild: pathlib.Path):
    dst = mesonbuild / "linkers/detect.py"
    if not dst.exists():
        raise Exception(f"{dst} not found")

    w = io.StringIO()
    done = False
    for l in dst.read_text().splitlines(True):
        strip = l.strip()
        if strip == MESON_ZIG_PATCH_ALREADY:
            done = True
        if not done and strip == MESON_ZIG_PATCH_INSERT:
            w.write(MESON_ZIG_PATCH)
            done = True
        w.write(l)

    dst.write_text(w.getvalue())


def main():
    logging.basicConfig(level=logging.DEBUG)

    meson_dir = get_package_dir("mesonbuild")
    if not meson_dir:
        raise Exception("lib: [mesonbuild] not found")
    patch_meson(meson_dir)

    zig_dir = get_package_dir("ziglang")
    if not zig_dir:
        raise Exception("lib: [ziglang] not found")
    LOGGER.info(f"zig_dir: {zig_dir}")
    os.environ["PATH"] = os.environ["PATH"] + f";{zig_dir}"

    # meson setup
    system(
        f'meson setup builddir --prefix "{ROOT}/prefix" --buildtype release --native-file zig.ini -Dcpp_std=c++20 -Dexecutables=true',
    )

    # meson install
    system("meson install -C builddir --tags runtime")

    # wix
    shutil.copy("vrmeditor.xml", "vrmeditor.wxs")
    system("wix convert vrmeditor.wxs")
    system("msbuild /restore")
    system("msbuild vrmeditor.wixproj /p:Configuration=Release")


if __name__ == "__main__":
    main()
