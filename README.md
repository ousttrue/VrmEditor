# vrmeditor

read, write, edit and animation test.

## features

| function                         | read | write | edit |                      |
| -------------------------------- | ---- | ----- | ---- | -------------------- |
| glTF-2.0                         |      |       |      |                      |
| (mesh)                           | ✅   | ✅    | ---  |                      |
| (draco)                          | 🟩   | ---   | ---  |                      |
| (quantity)                       | 🟩   | ---   | ---  |                      |
| (material)                       | ✅   | ✅    | ✅   |                      |
| (material.ColorTexture)          | ✅   | ✅    | ---  |                      |
| (material.mtoon-0.x)             | 🟩   | 🟩    | 🟩   |                      |
| (material.mtoon-1.0)             | 🟩   | 🟩    | 🟩   |                      |
| (animation.TRS)                  | ✅   | ✅    | 🟩   | TODO:keyframe editor |
| (animation.morphTarget)          | ✅   | ✅    | 🟩   |                      |
| (animation.linear_interpolation) | 🟩   | 🟩    | 🟩   |                      |
| (animation.cubic_interpolation)  | 🟩   | 🟩    | 🟩   |                      |
| vrm-0.x                          |      |       |      |                      |
| (meta)                           | 🟩   | 🟩    | 🟩   |                      |
| (expression.morphtarget)         | ✅   | 🟩    | 🟩   |                      |
| (expression.material)            | 🟩   | 🟩    | 🟩   |                      |
| (lookat.bone)                    | 🟩   | 🟩    | 🟩   |                      |
| (lookat.expression)              | 🟩   | 🟩    | 🟩   |                      |
| (springbone)                     | ✅   | 🟩    | 🟩   |                      |
| (humanoid)                       | ✅   | 🟩    | 🟩   |                      |
| (Y180 rotation)                  | 🟩   | 🟩    | ---  | vrm-0.x face -Z      |
| vrm-1.0                          |      |       |      |                      |
| (expression.morphtarget)         | ✅   | 🟩    | 🟩   |                      |
| (expression.material)            | 🟩   | 🟩    | 🟩   |                      |
| (lookat.bone)                    | 🟩   | 🟩    | 🟩   |                      |
| (lookat.expression)              | 🟩   | 🟩    | 🟩   |                      |
| (springbone)                     | ✅   | 🟩    | 🟩   | TODO:capusule        |
| (humanoid)                       | ✅   | 🟩    | 🟩   |                      |
| (constraint)                     | ✅   | 🟩    | 🟩   |                      |
| vrm-animation                    |      |       |      |                      |
| (humanoid)                       | ✅   | 🟩    | 🟩   |                      |
| (expression)                     | ✅   | ✅    | 🟩   |                      |
| (lookat)                         | 🟩   | 🟩    | 🟩   |                      |
| bvh                              | ✅   | ---   | ---  |                      |
| (humanoid mapping)               | ✅   | ---   | ---  |                      |
| fbx                              | 🟩   | ---   | ---  |                      |

## TODO

- BoxInterleaved

## Memo

```
        +------------+
        | Renderer   |
        +------------+
  drawlilst ^
            |
        +------------+
        |libvrm scene| <- Pose/Animation
        +------------+
     import ^|
            |v export
        +--------+
        |gltfjson| <- ImGui/TPose
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

### external

- cuber
- gltfjson
- grapho

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
$ meson setup builddir -Dcpp_std=c++latest
$ meson compile -C builddir
```

```
# clang16 on Ubuntu22.04
$ meson setup builddir -Dcpp_std=c++2b
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

## icon

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
