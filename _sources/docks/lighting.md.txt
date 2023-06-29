# 💡Lighting.md

光源設定です。

![lighting](lighting.jpg){w=600px align=center}

- hdr 画像をロードできます

## KHR_lights_punctual 拡張(WIP)

:::{note}
`v0.11.0` 現在では PBR と MToon で別の Shader を使っているため、
光源の効き方が異なります。
:::

伝統的な `directional`, `point`, `spot` ライトです。

https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/README.md

### default

- directional light 一灯

### glTF 内に KHR_lights_punctual があった場合

デフォルトの directional light を置き換えます

## hdr 画像による環境光源

:::{note}
`v0.11.0` 現在では PBR と MToon で別の Shader を使っているため、
光源の効き方が異なります。
:::

- IBL を追加します。
- SkyBox を追加します。
