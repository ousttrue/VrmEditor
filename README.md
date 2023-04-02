# vrmeditor

read, write, animation test.

## features

### import/export

- [x] gltf import
- [x] glb
  - [x] import
  - [ ] export
  - [ ] vrm-0.x
    - [x] expression. morhtarget
    - [ ] expression. material
    - [ ] lookat. bone
    - [ ] lookat. expression
    - [x] springbone. basic without collision
    - [x] humanoid
  - [ ] vrm-1.0
  - [ ] vrm-animation
    - [ ] import
    - [ ] export
- [x] bvh import
- [ ] fbx import

### material

- [x] unlit
- [ ] pbr
- [ ] mtoon(vrm-0.x)
- [ ] mtoon(vrm-1.0)

### animation

- [x] gltf. tranlsation, rotation, scaling and weights.
- [x] bvh
- [x] vrm0 from bvh
- [ ] vrm1. humanoid retarget

### write

- [ ] glb
- [ ] vrm-0.x
- [ ] vrm-1.0

## UI

### imgui / configuration

- [x] imgui.ini => ~/.vrmeditor.ini
- [ ] ~/.config/vrmeditor/init.lua
- [ ] tree / indent

### assets

- [ ]  : folder
- [ ]  : gltf
- [ ] 󰕣 : glb
- [ ] 󰋦 : vrm
- [ ] 󰕠 : fbx
- [ ] 󰑮 : bvh
- [ ]  : image
- [ ]  : text
- [ ] sort

### 3D view

- [x] Fit camera frustum to bounding box when load
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

- [ ] 󰕣 : mesh icon
- [ ] 󰂹 : humanoid icon
- [ ] 󰚟 : spring icon
- [ ] 󱥔 : spring collider icon
- [ ] selected

### humanoid

- [ ] 󰂹 : bone selector
  - [ ]  : hips
  - [ ] 󱍞 : head
  - [ ] 󰹆 : hand-left
  - [ ] 󰹇 : hand-right
- [ ] node assign
- [ ] pose input stream

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
$ meson setup builddir -Dcpp_std=c++latest
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
- `bvh`
- `gltf`
