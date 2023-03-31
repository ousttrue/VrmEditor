# vrmeditor

read, write, animation test.

## features

### load

- [x] glb. partial WIP
  - [ ] vrm-0.x
    - [x] expression. morhtarget
    - [ ] expression. material
    - [ ] lookat. bone
    - [ ] lookat. expression
    - [x] springbone. basic without collision.
    - [x] humanoid.
  - [ ] vrm-1.0
- [ ] gltf
- [x] bvh
- [ ] fbx

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

msvc17 or clang16 ?

- c++23 std::expected
- c++20 std::span, std::format
- c++17 std::string_view, std::filesystem, std::optional

```
$ meson setup builddir -Dcpp_std=c++latest
$ meson compile -C builddir
```

## naming plan

- formatter clang-format: mozilla
- class, struct name PascalCase
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
