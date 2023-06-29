# 🌳Hierarchy

## 🎁Asset tab

![docks](hierarchy.jpg){w=600px align=center}

* glTF の node を親子ツリーで表示します。
* Node を選択できます。
* HumanBone の選択ができます。
* glTF ノード(json)の移動(Translation)・回転(Rotation)・拡縮(Scale) が表示されます。
  * 変更結果は Json/AssetView に反映されます。

:::{note}
正規化済みのモデルでは、回転 = {0,0,0,1} かつ 拡縮 = {1,1,1} になります。
:::

## 🎬Runtime tab

* glTF の node を親子ツリーで表示します。
* Node を選択できます。
* Runtime ノード(animation)の移動(Translation)・回転(Rotation)・拡縮(Scale)が表示されます。
    * 変更結果は RuntimeView に反映されます。
    * Json には反映されません。
    * SpringBone が連動します
    * Constraint が連動します
