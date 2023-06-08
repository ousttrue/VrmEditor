% hello documentation master file, created by
% sphinx-quickstart on Sun Jan  2 22:20:10 2022.
% You can adapt this file completely to your liking, but it should at least
% contain the root `toctree` directive.

# VrmEditor

`VrmEditor` は `vrm` に特化した `glTF` エディターです。

## 特徴

`glTF` の座標系 `右手系-YUP`, `テクスチャー左上原点` を採用。
PBRテクスチャーは、`occlusion` は Red, `roughness` は Green, `metallic` は Blue という glTF と同じ構成です。
内部データを `Json` そのものにすることで、変更が即座に `glTF` の変更として有効になるようになっています。
簡単に `glTF` の読み書きができるように設計しています。

```{toctree}
:maxdepth: 2
features
json_editor/index
```

## Indices and tables

* {ref}`genindex`
* {ref}`modindex`
* {ref}`search`
