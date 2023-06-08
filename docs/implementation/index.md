# Implementation Note

C++ と OpenGL で実装しています。
主に３つのパーツにわかれます。

## Json

Json を読み込んで、JsonTree を保持します。
JsonTree のまま描画したり、編集したりすることで `glTF` との親和性を高める方針です。

## Runtime

主に、`Bone Animation` と `Morph Target` の解決をします。

* glTF Animation
* Humanoid
* Expression

## Renderer

MorphTarget適用、スキニング解決、描画

* OpenGL
* PBR shader: KHRONOS の [glTF-Sample-Viewer](https://github.com/KhronosGroup/glTF-Sample-Viewer) を使用します
* MToon shader: Pixiv の [three-vrm](https://github.com/pixiv/three-vrm) を使用します
