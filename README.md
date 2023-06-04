# vrmeditor

read, write, edit and animation test.

## features

- [x] glTF-2.0
  - [ ] animation.linear_interpolation
  - [ ] animation.cubic_interpolation
- [x] bvh
- [ ] fbx

### glTF Extensions

- [ ] KHR_draco_mesh_compression
- [ ] KHR_lights_punctual
- [ ] KHR_materials_clearcoat
- [ ] KHR_materials_emissive_strength
- [ ] KHR_materials_ior
- [ ] KHR_materials_iridescence
- [ ] KHR_materials_sheen
- [ ] KHR_materials_specular
- [ ] KHR_materials_transmission
- [x] KHR_materials_unlit
- [ ] KHR_materials_variants
- [ ] KHR_materials_volume
- [ ] KHR_mesh_quantization
- [ ] KHR_texture_basisu
- [ ] KHR_texture_transform
- [ ] KHR_xmp_json_ld
- [x] vrm-0.x
- [ ] vrm-1.0

## TODO

- GLTF_SAMPLE_MODELS: BoxInterleaved

## Memo

```
        +------------+
        | Renderer   |
        +------------+
  drawlilst ^
            |
        +------------+
        |libvrm scene| <- Skinning/MorphTarget/Pose/Animation
        +------------+
     import ^|
            |v export
        +--------+
        |gltfjson| <- json dom tree: ImGui Edit
        +--------+
deserialize ^|
            |v serialize
        gltf/glb
```

## milestone

### 1

- read / write vrm-0.x & 1.0

### x

- fbx import
- vrma export
- motion merge
- pbr material
- mtoon material
- keyframe edit

## dependencies

- imgui
  - ImGuizmo
  - ImGuiFileDialog
  - ImNodes
- glfw3
- glew
- DirectXMath
- lua-jit
- stb
- IconFontCppHeaders
- googletest
- asio
- simplefilewatcher

### shaders

- PBR: https://github.com/JoeyDeVries/LearnOpenGL/tree/master/src/6.pbr/2.2.2.ibl_specular_textured
- PBR/Unlit: https://github.com/KhronosGroup/glTF-Sample-Viewer
- MToon: https://github.com/pixiv/three-vrm
  - depends: https://github.com/mrdoob/three.js/tree/r150 (r150 required)

### external

- gltfjson(JSON utility)
- grapho(GPU API wrapper)
- cuber(bone draw helper)

## build

|                  | msvc17      | clang16          |          |
| ---------------- | ----------- | ---------------- | -------- |
| std::spanstream  |             |                  | not used |
| std::expected    | `c++latest` | `c++2b` `libc++` | OK       |
| std::format      | `c++latest` |                  | removed  |
| std::span        | `c++latest` | `c++20`          | OK       |
| std::string_view | `c++latest` | `c++20`          | OK       |
| std::filesystem  | `c++latest` | `c++20`          | OK       |
| std::optional    | `c++latest` | `c++20`          | OK       |

```
# msvc17
$ meson setup builddir -Dcpp_std=c++latest -Dexecutable=true
$ meson compile -C builddir
```

```
# clang16 on Ubuntu22.04
$ meson setup builddir -Dcpp_std=c++2b -Dexecutable=true
$ meson compile -C builddir
```

## naming plan

- formatter clang-format: mozilla
- class, struct name PascalCase => camelCase. conflict public member name
- free function: lower_snake
- non public memver function: lower_snake

### variables

Naming conventions for variables with a wider scope than local variables.

- local variable: camelCase
- public member: PascalCase
- non public member variable: prefix `m_`
- static variable: prefix `s_`
- global variable: prefix `g_`
- const, enum value: UPPER_SNAKE
- enum class value: Pascal

### folder / namespace

#### core

node(gltf) / mesh / skinning / deformed_mesh

#### animation

node(animation / pose) / animation / springbone / constraint

#### humanoid

udp / bvh

#### namespace

- `libvrm` (Node hierarchy / BaseMesh)
- `libvrm::vrm` (Humanoid, Expression, SpringBone, Constraint, LookAt)
- `libvrm::vrma`
- `libvrm::bvh`
- `libvrm::runtime` (NodeAnimation, MorphTarget, Skinning)
- `libvrm::serialization` (Udp pose)

## nerdfont icon

- [ ]  : play
- [ ]  : stop
- [ ] 󰒭 : next
- [x]  : image icon
- [x]  : text icon
- [x]  : buffer_view icon
- [x]  : accessor icon
- [x] 󰕣 : mesh icon
- [x] 󰂹 : humanoid icon
- [x] 󰚟 : spring icon
- [ ] 󱥔 : spring collider icon
- [x]  : morph
- [x]  : hips
- [x] 󱍞 : head
- [x] 󰹆 : hand-left
- [x] 󰹇 : hand-right
- [x] 󱗈 : foot, toes
- [x] 󰱰 喜
- [x] 󰱩 怒
- [x] 󰱶 哀
- [x] 󰱱 楽
- [x] 󰱮 驚
- [x] ‿ ‿ blink
- [x] 󰈈 ‿ blink-L
- [x] ‿ 󰈈 blink-R
- [x] 󰈈  lookAt-up
- [x] 󰈈  lookAt-down
- [x] 󰈈  lookAt-left
- [x] 󰈈  lookAt-right
- [x]  : texture
- [x]  : material
- [x] 󰕣 : mesh
