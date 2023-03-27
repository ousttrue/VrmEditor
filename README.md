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
    - [ ] springbone.
    - [ ] humanoid.
  - [ ] vrm-1.0
- [ ] gltf
- [ ] bvh. WIP
- [ ] fbx

### material

- [x] unlit.
- [ ] pbr.
- [ ] mtoon(vrm-0.x)
- [ ] mtoon(vrm-1.0)

### animation

- [x] gltf. tranlsation, rotation, scaling. TODO: morphTarget
- [ ] bvh. WIP
- [ ] vrm. humanoid retarget

### write

- [ ] glb
- [ ] vrm-0.x
- [ ] vrm-1.0

## dependencies

- imgui
  - ImGuizmo
- glfw3
- glew
- DirectXMath
- nlohmann-json
- lua-jit
- stb

## build

msvc17 or clang16 ?

- c++23 std::expected
- c++20 std::span, std::format
- c++17 std::string_view, std::filesystem, std::optional

```
$ meson setup builddir -Dcpp_std=c++latest
$ meson compile -C builddir
```

## 命名規則方針

- formatter clang-format: mozilla
- class, struct name PascalCase
- public member: PascalCase
- non public member variable: prefix `m_`
- free function: camelCase
- static variable: prefix `s_`
- global variable: prefix `g_`
- non public memver function: camelCase
- local variable: lower_snake
- enum value: UPPER_SNAKE
- enum class value: Pascal

