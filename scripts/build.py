from typing import Optional
import pathlib
import site
import pathlib
import logging
import subprocess
import os
import shutil

LOGGER = logging.getLogger(__name__)
HERE = pathlib.Path(__file__).parent
ROOT = HERE.parent


def get_zig_dir() -> Optional[pathlib.Path]:
    for p in site.getsitepackages():
        dir = pathlib.Path(p)
        zig_dir = dir / "ziglang"
        if zig_dir.exists():
            return zig_dir


def system(cmd: str) -> int:
    return subprocess.call(cmd, shell=True)


def main():
    logging.basicConfig(level=logging.DEBUG)

    zig_dir = get_zig_dir()
    if not zig_dir:
        raise Exception("zig not found")
    LOGGER.info(f"zig_dir: {zig_dir}")
    os.environ["PATH"] = os.environ["PATH"] + f";{zig_dir}"

    # meson setup
    retcode = system(
        f'meson setup builddir --prefix "{ROOT}/prefix" --buildtype release --native-file zig.ini -Dcpp_std=c++20 -Dexecutables=true',
    )
    if retcode < 0:
        raise Exception("meson seup")

    # meson install
    retcode = system(
        "meson install -C builddir --tags runtime",
    )
    if retcode < 0:
        raise Exception("meson install")

    # wix
    shutil.copy("vrmeditor.xml", "vrmeditor.wxs")
    system("wix convert vrmeditor.wxs")
    system("msbuild /restore")
    system("msbuild vrmeditor.wixproj /p:Configuration=Release")


if __name__ == "__main__":
    main()
