# Keychron Q15 Max カスタムキーマップ

このキーマップはKeychron Q15 Max用のカスタムキーマップで、以下の機能を提供します：

## 主な機能

- **Orbital Mouse機能**: キーボードでマウスカーソルを円運動のように操作
- **Select Word機能**: 単語や行を効率的に選択
- **Super Alt+Tab/Ctrl+Tab機能**: ウィンドウ/タブの高速切り替え
- **VIA対応**: VIAでのキーマップカスタマイズをサポート
- **エンコーダーサポート**: 回転エンコーダーによる音量調整やRGB明るさ制御

## レイヤー構成

- **MAC_BASE**: Macの基本レイヤー
- **WIN_BASE**: Windowsの基本レイヤー
- **MAC_FN**: MacのFnレイヤー
- **WIN_FN**: WindowsのFnレイヤー
- **COM_FN**: 共通Fnレイヤー（F1-F12キーなど）

## カスタムキーコード

カスタムキーコードはKeychron固有領域（QK_KB）に配置されています：

```c
enum kb_keycodes {
    // Keychron共通キーコードの続きから定義
    ALT_TAB = NEW_SAFE_RANGE,
    CTRL_TAB,
    SELWORD,
    // Orbital Mouseのカスタムキーコード
    OM_FORWARD,  // 前進
    OM_BACKWARD, // 後退
    OM_LEFT,     // 左回転
    OM_RIGHT,    // 右回転
    OM_SNIPE,    // 低速モード
    OM_BTN1_KC,  // マウス左ボタン
    OM_BTN2_KC,  // マウス右ボタン
    OM_WHEEL_UP, // ホイール上
    OM_WHEEL_DOWN // ホイール下
};
```

## ビルドと書き込み

```bash
# ビルド
make keychron/q15_max/ansi_encoder:annenpolka

# 書き込み
qmk flash -kb keychron/q15_max/ansi_encoder -km annenpolka
```

## VIA対応

`via.json`ファイルをVIAアプリ（https://usevia.app/）にインポートすることで、カスタム機能を含めたキーマッピングをGUIで編集できます。

## カスタムキーコードの編集手順

新しいカスタムキーコードを追加する場合は、以下の2つのファイルを必ず合わせて編集します：

1. **keymap.c**
   - `enum kb_keycodes`にキーコードを追加
   - `process_record_user()`内に処理コードを実装

2. **via.json**
   - `customKeycodes`配列に新しいキーコードを追加
   - 例：
     ```json
     {"name": "新機能", "title": "新機能の説明", "shortName": "略称"}
     ```

## 注意事項

- カスタムキーコードはNEW_SAFE_RANGEから始まるKeychron固有領域を使用
- VIA設定を変更した場合は、`git add -f via.json`コマンドで強制的に追加する必要あり
- 処理が複雑な機能は`features/`ディレクトリ内に分離して実装

## ライセンス

GNU General Public License v2.0