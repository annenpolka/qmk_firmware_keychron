# QMK ファームウェア開発ガイドライン

## ビルドコマンド
- 基本ビルド: `make keychron/<keyboard>/<variant>:<keymap>`
- 書き込み: `qmk flash -kb keychron/<keyboard>/<variant> -km <keymap>`
- 特定キーボードのビルド例: `make keychron/q15_max/ansi_encoder:annenpolka`
- キーマップ検証: `qmk lint -kb keychron/<keyboard>/<variant>`
- デバッグモード有効化: `CONSOLE_ENABLE=yes` を rules.mk に追加

## ブートローダーモード手順
1. モードスイッチを「Cable」（有線モード）に切り替える
2. ESCキーを押しながらUSBケーブルを接続、または
3. スペースバー下のリセットボタンを押しながらUSB接続
4. STM32 BOOTLOADER が認識されるまで待機
5. `qmk flash` コマンドを実行して書き込み

## コード規約
- C言語: 4スペースインデント、修正版One True Brace Style
- コメント: `/* */` 形式を推奨、意図説明のために積極的に使用
- インクルードガード: `#pragma once` を使用
- ヘッダーファイル命名: 機能を反映した明確な名前を使用
- 関数名: スネークケース `function_name` を使用
- 変数名: スネークケース、グローバル変数は `g_` 接頭辞
- マクロ名: 大文字スネークケース `MACRO_NAME` を使用

## キーマップ作成手順
1. 既存キーマップをコピー: `keyboards/keychron/<kb>/<variant>/keymaps/default` を新しい名前でコピー
2. レイヤー設計: 基本レイヤー（0）から始め、機能別に追加（FN、メディア、マクロなど）
3. キーマップ定義: `keymaps[][]` 配列内でLAYOUT_xxx マクロを使用して各レイヤーのキーを設定
4. カスタム機能: `rules.mk` に必要な機能を追加 (RGBLIGHT_ENABLE, ENCODER_ENABLE など)
5. 設定調整: `config.h` でキーボード固有の設定を調整

## VIA対応キーマップ作成
1. `rules.mk` に `VIA_ENABLE = yes` を追加
2. カスタムキーコード定義:
   ```c
   enum custom_keycodes {
     CUSTOM_KC = SAFE_RANGE,  // VIALでは QK_KB_0 を使用
   };
   ```
3. カスタムキーコード処理:
   ```c
   bool process_record_user(uint16_t keycode, keyrecord_t *record) {
     switch (keycode) {
       case CUSTOM_KC:
         if (record->event.pressed) {
           // キーが押された時の処理
         } else {
           // キーが離された時の処理
         }
         return false; // 標準処理を停止
     }
     return true; // その他のキーは標準処理
   }
   ```
4. エンコーダー対応（オプション）:
   ```c
   #if defined(ENCODER_MAP_ENABLE)
   const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
     [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
     [1] = { ENCODER_CCW_CW(RGB_VAD, RGB_VAI) }
   };
   #endif
   ```

## VIAでの設定方法
1. VIAウェブアプリを開く: https://usevia.app/
2. キーボードを接続し「Authorize device」をクリック
3. 「CONFIGURE」タブでキーマップ編集
4. カスタムキーコード使用:
   - 「Special」→「Any」を選択
   - SAFE_RANGEから始まるキーコードは「0x5F00」から順に割り当て
   - VIALでは「User」タブにカスタムキーコードが表示される
5. エンコーダー設定:
   - 点線円で表示されるエンコーダーをクリックして設定変更
   - 時計回り/反時計回り操作にキーコード割り当て

## エラー処理とデバッグ
- 適切なエラーチェックとフォールバック動作の実装
- デバッグ有効化: `rules.mk` に `CONSOLE_ENABLE = yes` を追加
- デバッグ情報出力: `dprint("Debug: %d\n", value);` などで情報を出力
- QMK Toolboxで出力確認: シリアル接続でログを監視

## メモリ最適化
- AVRチップの制限: フラッシュメモリ32KB、EEPROM 1KB
- STM32チップ: より大きなメモリ領域（128KB+）
- 機能の選択的有効化: 使用しない機能は無効化（RGBLIGHT, AUDIO など）
- EEPROMの節約: VIA設定は多くのEEPROMを使用するため注意