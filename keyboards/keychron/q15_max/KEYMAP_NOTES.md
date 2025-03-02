# Keychron Q15 Max カスタムキーマップ

## annenpolka キーマップ

### 機能概要
- VIA対応キーマップ（`VIA_ENABLE = yes`）
- Super Alt+Tab機能の実装
- エンコーダマップ対応（デフォルト設定）

### カスタムキーコード
```c
enum custom_keycodes {
    ALT_TAB = SAFE_RANGE,
};
```

### Super Alt+Tab 実装
この機能を使うと、Alt+Tabの自動解放機能が使えます。キーを押すとAltキーを保持した状態でTabを送信し、1秒間何も操作がなければAltキーを自動的に解放します。

```c
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case ALT_TAB:
            if (record->event.pressed) {
                if (!is_alt_tab_active) {
                    is_alt_tab_active = true;
                    register_code(KC_LALT);
                }
                alt_tab_timer = timer_read();
                register_code(KC_TAB);
            } else {
                unregister_code(KC_TAB);
            }
            break;
    }
    return true;
}

void matrix_scan_user(void) {
    if (is_alt_tab_active) {
        if (timer_elapsed(alt_tab_timer) > 1000) {
            unregister_code(KC_LALT);
            is_alt_tab_active = false;
        }
    }
}
```

### エンコーダマップ設定
デフォルト設定:
- 通常レイヤー: 音量調整
- FNレイヤー: RGB明るさ調整

```c
const uint16_t PROGMEM encoder_map[][2][2] = {
    [MAC_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [WIN_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [MAC_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI), ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [WIN_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI), ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [COM_FN]   = {ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______)},
};
```

### VIAでの使用方法
1. VIAウェブアプリ（https://usevia.app/）にアクセス
2. キーボードを接続してauthorize
3. ALT_TABキーコードを割り当てる場合:
   - Specialタブ
   - Anyキー選択
   - 0x5F00と入力（SAFE_RANGEの最初の値）
   - 必要なキーに割り当て
4. エンコーダ設定:
   - 点線の円をクリック
   - キーコードを選択して割り当て

### 備考
- このキーマップはKeychron Q15 Max ANSI Encoder用に設計されています
- VIA経由で設定を変更する場合は設定が保存されます
- エンコーダへのSuper Alt+Tab機能の割り当ても可能です