#pragma once

#include "quantum.h"

/**
 * @brief Word selection macro.
 * @return false when the macro handles its custom keycode, true to let QMK process the keycode normally.
 */
bool process_select_word(uint16_t keycode, keyrecord_t* record);

/**
 * @brief Register a selection action.
 *
 * Register (press) selection action. This must be followed by a call to select_word_unregister().
 * @param action 'W' word selection, 'B' backward word selection, or 'L' line selection.
 * @return True if the action was valid, false otherwise.
 */
bool select_word_register(char action);

/**
 * @brief Unregister a selection action (previously registered with select_word_register).
 */
void select_word_unregister(void);

/**
 * @brief Tap (register and unregister) a selection action.
 * @param action 'W' word selection, 'B' backward word selection, or 'L' line selection.
 * @return True if the action was valid, false otherwise.
 */
bool select_word_tap(char action);

/**
 * @brief Task to check for inactivity timeout.
 *
 * Call this from housekeeping_task_user() when using SELECT_WORD_TIMEOUT.
 */
void select_word_task(void);

/**
 * @brief Optional defined callback to indicate whether host is Mac.
 *
 * Implement this function if you have specific criteria for switching between Mac and
 * Windows/Linux hotkeys and SELECT_WORD_OS_DYNAMIC is defined in config.h.
 * @return True for Mac hotkeys, false for Windows/Linux hotkeys.
 */
bool select_word_host_is_mac(void);

extern uint16_t SELECT_WORD_KEYCODE;