# vrmeditor

read, write, animation test.

## features

### import

- [x] gltf import
- [x] glb
  - [x] import
    - [ ] draco
    - [ ] quantity
  - [ ] vrm-0.x
    - [x] expression. morphtarget
    - [ ] expression. material
    - [ ] lookat. bone
    - [ ] lookat. expression
    - [x] springbone. basic without collision
    - [x] humanoid
      - [ ] retarget Y180 rotation.
  - [ ] vrm-1.0
    - [ ] expression. morphtarget
    - [ ] expression. material
    - [ ] lookat. bone
    - [ ] lookat. expression
    - [ ] springbone. basic without collision
    - [x] humanoid
    - [ ] constraint
  - [ ] vrm-animation
    - [ ] import
    - [x] export
    - [ ] humanoid
    - [x] expression
    - [ ] lookat
- [x] bvh import
  - [ ] humanoid mapping
- [ ] fbx import

### export

- [x] glb
- [ ] vrm-0.x
- [ ] vrm-1.0

### material

- [x] unlit
- [ ] pbr
- [ ] mtoon(vrm-0.x)
- [ ] mtoon(vrm-1.0)

### animation

- [x] gltf. tranlsation, rotation, scaling and weights.
- [x] bvh
- [x] vrm0 from bvh
- [x] vrm1. humanoid retarget
- [ ] linear interpolation
- [ ] cubic interpolation

## UI

### imgui / configuration

- [x] imgui.ini => ~/.vrmeditor.ini
- [x] ~/.vrmeditor.lua
- [x] tree / indent

### assets

- [x]  : gltf
- [x] 󰕣 : glb
- [x] 󰋦 : vrm
- [ ] 󰕠 : fbx
- [x] 󰑮 : bvh
- [x] sort

### 3D view

- [x] Fit camera frustum to bounding box when load
- [ ] camera reset
- [x] TR gizmo
- [ ] gizmo undo / history

### json

- [ ]  : image icon
- [ ]  : text icon
- [ ]  : buffer_view icon
- [ ]  : accessor icon
- [ ] selected

### resource

- [ ]  : texture
- [ ]  : material
- [ ] 󰕣 : mesh
- [ ] selected

### scene

- [x] 󰕣 : mesh icon
- [x] 󰂹 : humanoid icon
- [x] 󰚟 : spring icon
- [ ] 󱥔 : spring collider icon
- [x]  : morph
- [ ] selected

### humanoid

- [ ] 󰂹 : bone selector
  - [ ]  : hips
  - [ ] 󱍞 : head
  - [ ] 󰹆 : hand-left
  - [ ] 󰹇 : hand-right
- [ ] bone assign
- [ ] pose retarget

### vrm

- [ ] meta
- [ ] expression
- [ ] lookat
- [ ] firstperson

### timeline

- [x] track
  - [ ] seek
- [ ] keyframe
- [ ]  : play
- [ ]  : stop
- [ ] 󰒭 : next

## tests

### gltf

### vrm

### bvh

### fbx

## dependencies

- imgui
  - ImGuizmo
  - ImGuiFileDialog
  - ImNodes
- glfw3
- glew
- DirectXMath
- nlohmann-json
- lua-jit
- stb
- IconFontCppHeaders
- googletest

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

### namespace

- `vrm`
- `vrm::v0`
- `vrm::v1`
- `bvh`
- `gltf`
