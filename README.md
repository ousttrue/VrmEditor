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
- [x] vrma. expression minimum.

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

### humanoid

- [ ] 󰂹 : bone selector
- [ ] bone assign
- [x] pose retarget

### expression

### vrm

- [ ] meta
- [x] expression
- [ ] lookat
- [ ] firstperson

### timeline

- [x] track
  - [ ] seek
- [ ] keyframe

## tests

### gltf

### vrm

### bvh

### fbx

# milestone
## 1
- read / write vrm-0 & 1

## x
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

- `libvrm::vrm`
- `libvrm::vrm::v0`
- `libvrm::vrm::v1`
- `libvrm::vrma`
- `libvrm::bvh`
- `libvrm::gltf`

# icon
- [ ]  : play
- [ ]  : stop
- [ ] 󰒭 : next
- [ ]  : image icon
- [ ]  : text icon
- [ ]  : buffer_view icon
- [ ]  : accessor icon
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
- [ ] aa
- [ ] ih
- [ ] ou
- [ ] ee
- [ ] oh
- [x] ‿ ‿ blink
- [x] 󰈈 ‿ blink-L
- [x] ‿ 󰈈 blink-R
- [x] 󰈈  lookAt-up
- [x] 󰈈  lookAt-down
- [x] 󰈈  lookAt-left
- [x] 󰈈  lookAt-right
- [ ]  : texture
- [ ]  : material
- [ ] 󰕣 : mesh

