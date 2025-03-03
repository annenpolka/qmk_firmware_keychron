/* Copyright 2022 Pascal Kolkman (@pascalkolkman)
 * Copyright 2022 Twitter, Inc. and its affiliates.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "select_word.h"

#ifdef OS_DETECTION_ENABLE
#    include "os_detection.h"
#endif

// 変数はkeymap.cで定義されるため、externとして宣言
extern uint16_t SELECT_WORD_KEYCODE;

#if defined(SELECT_WORD_TIMEOUT) && SELECT_WORD_TIMEOUT > 0
static uint32_t select_word_timeout_timer = 0;
#endif

// State is 0 for no recent presses of the macro; 1 for pressed word select, 2 for
// pressed line select. If the state is nonzero, it's expected that the modifier keys were
// left pressed, and on the next press of SELECT_WORD_KEYCODE they should be extended.
static int state = 0;

static void select_word_clear_state(void) {
    state = 0;
}

void select_word_task(void) {
#if defined(SELECT_WORD_TIMEOUT) && SELECT_WORD_TIMEOUT > 0
    if (state != 0 && timer_elapsed32(select_word_timeout_timer) > SELECT_WORD_TIMEOUT) {
        select_word_clear_state();
    }
#endif
}

static bool use_mac_hotkeys(void) {
#if defined(SELECT_WORD_OS_DYNAMIC)
    return select_word_host_is_mac();
#elif defined(OS_DETECTION_ENABLE)
    switch (detected_host_os()) {
        case OS_MACOS: return true;
        case OS_WINDOWS:
        case OS_LINUX:
        case OS_IOS:
        case OS_ANDROID:
        case OS_UNSURE: break;
    }
    // OS Detection did not decide Mac, so fall back to the default behavior.
#endif

#if defined(SELECT_WORD_OS_MAC)
    return true;
#else
    return false;
#endif
}

/**
 * @brief Move to beginning of word.
 */
static void move_beginning_of_word(void) {
    if (use_mac_hotkeys()) {
        register_code(KC_LALT);
        tap_code(KC_LEFT);
        unregister_code(KC_LALT);
    } else {
        register_code(KC_LCTL);
        tap_code(KC_LEFT);
        unregister_code(KC_LCTL);
    }
}

/**
 * @brief Move to end of word.
 * Used internally in select_word functions.
 */
static void move_end_of_word(void) __attribute__((unused));
static void move_end_of_word(void) {
    if (use_mac_hotkeys()) {
        register_code(KC_LALT);
        tap_code(KC_RIGHT);
        unregister_code(KC_LALT);
    } else {
        register_code(KC_LCTL);
        tap_code(KC_RIGHT);
        unregister_code(KC_LCTL);
    }
}

/**
 * @brief Move to end of word with selection.
 */
bool select_word_register(char action) {
    if (action == 'B') {
        // For backward (left of cursor) selection.
        if (use_mac_hotkeys()) {
            register_code(KC_LSFT);
            register_code(KC_LALT);
            tap_code(KC_LEFT);
        } else {
            register_code(KC_LSFT);
            register_code(KC_LCTL);
            tap_code(KC_LEFT);
        }
    } else if (action == 'W') {
        // For word selection.
        if (use_mac_hotkeys()) {
            register_code(KC_LSFT);
            register_code(KC_LALT);
            tap_code(KC_RIGHT);
        } else {
            register_code(KC_LSFT);
            register_code(KC_LCTL);
            tap_code(KC_RIGHT);
        }
    } else if (action == 'L') {
        // For line selection.
        if (use_mac_hotkeys()) {
            // On Mac: Cmd+Left, Shift+Cmd+Right
            register_code(KC_LCMD);
            tap_code(KC_LEFT);
            register_code(KC_LSFT);
            tap_code(KC_RIGHT);
        } else {
            // On Windows/Linux: Home, Shift+End
            tap_code(KC_HOME);
            register_code(KC_LSFT);
            tap_code(KC_END);
        }
    } else {
        return false;
    }
    return true;
}

/**
 * @brief Unregister a word or line selection.
 */
void select_word_unregister(void) {
    unregister_code(KC_LSFT);
    unregister_code(KC_LCTL);
    unregister_code(KC_LALT);
    unregister_code(KC_LCMD);
}

/**
 * @brief Tap (register and unregister) select word or line action key.
 */
bool select_word_tap(char action) {
    if (!select_word_register(action)) {
        return false;
    }
    select_word_unregister();
    return true;
}

bool process_select_word(uint16_t keycode, keyrecord_t* record) {
    if (keycode != SELECT_WORD_KEYCODE) {
        if (keycode == KC_LEFT || keycode == KC_RIGHT) {
            select_word_clear_state();
        }
        return true;
    }

    if (record->event.pressed) {
#if defined(SELECT_WORD_TIMEOUT) && SELECT_WORD_TIMEOUT > 0
        select_word_timeout_timer = timer_read32();
#endif

        if ((get_mods() & MOD_MASK_SHIFT) != 0) {
            // With shift pressed: line selection.
            if (state == 0 || state == 1) {
                if (state == 0) {
                    // First press: go to beginning of line, then select to end.
                    select_word_tap('L');
                } else {
                    // Subsequent press: unregister all to clear selection.
                    select_word_unregister();
                }
                // Leave the shift key held to extend to next line.
                if (use_mac_hotkeys()) {
                    register_code(KC_LSFT);
                    register_code(KC_DOWN);
                } else {
                    register_code(KC_LSFT);
                    register_code(KC_DOWN);
                }
                state = 2;
            } else if (state == 2) {
                // Extend line selection.
                tap_code(KC_DOWN);
            }
        } else {
            // Without shift pressed: word selection.
            if (state == 0 || state == 2) {
                // First press: goto beginning of word, then select to end.
                // After doing a line selection, also restart from scratch.
                if (state != 0) {
                    // Release all modifiers.
                    select_word_unregister();
                }
                move_beginning_of_word();
                select_word_register('W');
                state = 1;
            } else if (state == 1) {
                // Subsequent press: extend to select next word.
                tap_code(KC_RIGHT);
            }
        }
    } else {
        if (state == 1) {
            // Word selection: releasing Ctrl+Shift and keep selected by releasing SELECT_WORD_KEYCODE.
            select_word_unregister();
        } else if (state == 2) {
            // Line selection: releasing Shift and keep selected by releasing SELECT_WORD_KEYCODE.
            select_word_unregister();
        }
    }
    return false;
}