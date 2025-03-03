/* Copyright 2024 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"

bool is_alt_tab_active = false;
uint16_t alt_tab_timer = 0;
bool is_ctrl_tab_active = false;
uint16_t ctrl_tab_timer = 0;

enum layers {
    MAC_BASE,
    WIN_BASE,
    MAC_FN,
    WIN_FN,
    COM_FN,
};

enum custom_keycodes {
    ALT_TAB = SAFE_RANGE,
    CTRL_TAB,
    SELWORD,
    // Orbital Mouseのカスタムキーコード（ユーザー領域に配置）
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
// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_ansi_66(
        KC_MUTE,  KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,   KC_BSPC,  KC_MUTE,
        KC_ESC,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,   KC_RBRC,  KC_BSLS,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,   KC_ENT,
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,   KC_UP,	   KC_DEL,
        KC_LCTL,  KC_LOPTN, KC_LCMMD, _______,  KC_SPC,                       KC_SPC,             MO(MAC_FN),MO(COM_FN),KC_LEFT, KC_DOWN,  KC_RGHT),

    [WIN_BASE] = LAYOUT_ansi_66(
        KC_MUTE,  KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,   KC_BSPC,  KC_MUTE,
        KC_ESC,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,   KC_RBRC,  KC_BSLS,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,   KC_ENT,
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,   KC_UP,    KC_DEL,
        KC_LCTL,  KC_LGUI,  KC_LALT,  _______,  KC_SPC,                       KC_SPC,             MO(WIN_FN),MO(COM_FN),KC_LEFT, KC_DOWN,  KC_RGHT),

    [MAC_FN] = LAYOUT_ansi_66(
	    RGB_TOG,  KC_BRID,  KC_BRIU,  KC_MCTRL, KC_LNPAD, RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  RGB_TOG,
        KC_GRV,   BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        RGB_TOG,  RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,                      _______,            _______,  _______,  _______,  _______,  _______),

    [WIN_FN] = LAYOUT_ansi_66(
	    RGB_TOG,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  RGB_TOG,
        KC_GRV,   BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        RGB_TOG,  RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,                      _______,            _______,  _______,  _______,  _______,  _______),

    [COM_FN] = LAYOUT_ansi_66(
        _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   _______,
        KC_TILD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  BAT_LVL,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,                      _______,            _______,  _______,  _______,  _______,  _______)
};

// clang-format on
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][2][2] = {
    [MAC_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU),ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [WIN_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU),ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [MAC_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI),ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [WIN_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI),ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [COM_FN]   = {ENCODER_CCW_CW(_______, _______),ENCODER_CCW_CW(_______, _______)},
};
#endif // ENCODER_MAP_ENABLE


// Super Alt+Tab/Ctrl+Tab機能の実装
#include "features/select_word.h"
#include "features/orbital_mouse.h" // エラー修正済み

// Select Word機能のキーコード設定
uint16_t SELECT_WORD_KEYCODE = SELWORD;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Orbital Mouse機能の処理（標準のマウスキーコード用）
    if (!process_orbital_mouse(keycode, record)) {
        return false;
    }
    
    // Select Word機能の処理
    if (!process_select_word(keycode, record)) {
        return false;
    }

    if (!process_record_keychron_common(keycode, record)) {
        return false;
    }
    
    switch (keycode) {
        // Orbital Mouseのカスタムキーコード処理
        case OM_FORWARD:
            if (record->event.pressed) {
                register_code(OM_U); // OM_U（KC_MS_UP）をエミュレート
            } else {
                unregister_code(OM_U);
            }
            return false;
            
        case OM_BACKWARD:
            if (record->event.pressed) {
                register_code(OM_D); // OM_D（KC_MS_DOWN）をエミュレート
            } else {
                unregister_code(OM_D);
            }
            return false;
            
        case OM_LEFT:
            if (record->event.pressed) {
                register_code(OM_L); // OM_L（KC_MS_LEFT）をエミュレート
            } else {
                unregister_code(OM_L);
            }
            return false;
            
        case OM_RIGHT:
            if (record->event.pressed) {
                register_code(OM_R); // OM_R（KC_MS_RIGHT）をエミュレート
            } else {
                unregister_code(OM_R);
            }
            return false;
            
        case OM_SNIPE:
            if (record->event.pressed) {
                register_code(OM_SLOW); // OM_SLOW（KC_MS_BTN4）をエミュレート
            } else {
                unregister_code(OM_SLOW);
            }
            return false;
            
        case OM_BTN1_KC:
            if (record->event.pressed) {
                register_code(OM_BTN1); // OM_BTN1（KC_MS_BTN1）をエミュレート
            } else {
                unregister_code(OM_BTN1);
            }
            return false;
            
        case OM_BTN2_KC:
            if (record->event.pressed) {
                register_code(OM_BTN2); // OM_BTN2（KC_MS_BTN2）をエミュレート
            } else {
                unregister_code(OM_BTN2);
            }
            return false;
            
        case OM_WHEEL_UP:
            if (record->event.pressed) {
                register_code(OM_W_U); // OM_W_U（KC_MS_WH_UP）をエミュレート
            } else {
                unregister_code(OM_W_U);
            }
            return false;
            
        case OM_WHEEL_DOWN:
            if (record->event.pressed) {
                register_code(OM_W_D); // OM_W_D（KC_MS_WH_DOWN）をエミュレート
            } else {
                unregister_code(OM_W_D);
            }
            return false;
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
        case CTRL_TAB:
            if (record->event.pressed) {
                if (!is_ctrl_tab_active) {
                    is_ctrl_tab_active = true;
                    register_code(KC_LCTL);
                }
                ctrl_tab_timer = timer_read();
                register_code(KC_TAB);
            } else {
                unregister_code(KC_TAB);
            }
            break;
    }
    return true;
}

// config.hに移動

// Super Alt+Tab/Ctrl+Tab機能のタイマー処理とSelect Word機能のタイムアウト処理
void matrix_scan_user(void) {
    if (is_alt_tab_active) {
        if (timer_elapsed(alt_tab_timer) > 1000) {
            unregister_code(KC_LALT);
            is_alt_tab_active = false;
        }
    }
    
    if (is_ctrl_tab_active) {
        if (timer_elapsed(ctrl_tab_timer) > 1000) {
            unregister_code(KC_LCTL);
            is_ctrl_tab_active = false;
        }
    }
}

// Select WordとOrbital Mouse機能のタスク処理
void housekeeping_task_user(void) {
    select_word_task();
    orbital_mouse_task(); // エラー修正済み
}
