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

### 3D view

- [ ] TR gizmo

### json

- [ ] image icon
- [ ] text icon
- [ ] accessor/buffer_view icon
- [ ] selected

### scene

- [ ] mesh icon
- [ ] humanoid icon
- [ ] spring icon
- [ ] spring collider icon
- [ ] selected

### humanoid

- [ ] bone selector
- [ ] node assign
- [ ] pose input stream

### vrm

- [ ] meta
- [ ] expression
- [ ] lookat
- [ ] firstperson

### timeline

- [ ] track seek
- [ ] keyframe

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
