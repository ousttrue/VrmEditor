import pathlib
import os

HERE = pathlib.Path(__file__).parent

TEMPLATE = """
TEST(VrmLoad, {name}) {{
  auto path = get_path("{relative}");
  Scene scene;
  auto result = scene.Load(path);
  EXPECT_TRUE(result) << result.error();
}}
"""


def process(gltf_dir: pathlib.Path, path: pathlib.Path):
    with path.open("w", encoding="utf-8") as w:
        w.write(
            """
#include <cstdlib>
#include <gtest/gtest.h>
#include <vrm/scene.h>

std::filesystem::path get_path(std::string_view relative) {
  std::filesystem::path base = std::getenv("GLTF_SAMPLE_MODELS");
  return base / "2.0" / relative;
}

"""
        )

        for e in gltf_dir.rglob("*"):
            if "Unicode❤♻Test" in str(e):
                # skip
                continue
            if e.is_file:
                ext = e.suffix.lower()
                if ext == ".gltf" or ext == ".glb":
                    rel = e.relative_to(gltf_dir)
                    name = str(rel.parent).replace("\\", "_")
                    w.write(
                        TEMPLATE.format(
                            name=name.replace(" ", "_").replace("-", "_"),
                            relative=str(rel).replace("\\", "/"),
                        )
                    )


if __name__ == "__main__":
    sample_models = os.environ["GLTF_SAMPLE_MODELS"]

    process(pathlib.Path(sample_models) / "2.0", HERE / "gltf_samples.cpp")
