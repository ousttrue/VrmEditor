# Shader

VrmEditor で動作する Shader について。
glTF モデルのマテリアル設定に応じて４つの分岐があり、２つのShaderが使用されます。

- VRM-1.0 の MToon である => three-vrm へ
- VRM-0.x の MToon である => three-vrm へ
- KHR_materials_unlit である => khronos-sample-viewer へ
- PBR とみなす => khronos-sample-viewer へ

```{toctree}
:maxdepth: 2
khronos_shader.md
three_vrm_shader.md
ibl.md
```
