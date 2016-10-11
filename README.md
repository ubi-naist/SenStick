# SenStick

ScenStickは、加速度、角速度、磁界、照度、紫外線強度、湿度と温度、および気圧の7種類のセンシングとそのロギング機能がある、Bluetooth Low Energyでスマートホンと連携するデバイスです。

## ハードウェア
SenStickのハードウェアは、開発者向けのデバイスで、店頭での販売はありません。入手は個別に、Facebookのグループ https://www.facebook.com/UltraTinySensorBoardSenStick/ などへの問い合わせてください。

## ドキュメント

- docs/Scenstick使い方_rev110.pdf
- Senstickデバイス仕様書_rev103.pdf
- Senstickファーム実装概要書_rev100.pdf

## SDKのインストール

cocoapodsをインストールします。

$ sudo gem install cocoapods
$ pod setup

ファイル Podfile を作ります。

```
project 'YourProject.xcodeproj'
target 'YourProject' do
  use_frameworks!
  pod 'SenStickSDK'
end
```

podsをインストールします。
$ pod install

インストールしたあとに、更新する場合は、"update"を使います。
$ pod update

## ライセンス
MITライセンスです。

<!--
$pod lib lint SenStickSDK.podspec
$pod trunk push SenStickSDK.podspec
-->
