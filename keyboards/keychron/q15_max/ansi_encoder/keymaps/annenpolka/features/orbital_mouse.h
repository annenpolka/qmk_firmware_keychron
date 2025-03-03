/* Copyright 2023 Pascal Kolkman (@pascalkolkman)
 * Copyright 2023 Guillem Moreso (@guillem-ms)
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "quantum.h"
#include "pointing_device.h"

// Mouse Keys replacement.
enum orbital_mouse_keycodes {
    OM_U = KC_MS_UP,
    OM_D = KC_MS_DOWN,
    OM_L = KC_MS_LEFT,
    OM_R = KC_MS_RIGHT,
    OM_SLOW = KC_MS_BTN4, // Mouse BTN 4.

    OM_BTN1 = KC_MS_BTN1,
    OM_BTN2 = KC_MS_BTN2,
    OM_BTN3 = KC_MS_BTN3,
    OM_BTN4 = KC_MS_BTN5, // Mouse BTN 5.
    OM_BTN5 = UP('A', 'A'), // UC 0x41 / "A".
    OM_BTN6 = UP('B', 'B'), // UC 0x42 / "B".
    OM_BTN7 = UP('C', 'C'), // UC 0x43 / "C".
    OM_BTN8 = UP('D', 'D'), // UC 0x44 / "D".

    OM_W_U = KC_MS_WH_UP,
    OM_W_D = KC_MS_WH_DOWN,
    OM_W_L = KC_MS_WH_LEFT,
    OM_W_R = KC_MS_WH_RIGHT,

    OM_SEL1 = UP('E', 'E'), // UC 0x45 / "E".
    OM_SEL2 = UP('F', 'F'), // UC 0x46 / "F".
    OM_SEL3 = UP('G', 'G'), // UC 0x47 / "G".
    OM_SEL4 = UP('H', 'H'), // UC 0x48 / "H".
    OM_SEL5 = UP('I', 'I'), // UC 0x49 / "I".
    OM_SEL6 = UP('J', 'J'), // UC 0x4a / "J".
    OM_SEL7 = UP('K', 'K'), // UC 0x4b / "K".
    OM_SEL8 = UP('L', 'L'), // UC 0x4c / "L".
    OM_BTNS = UP('M', 'M'), // UC 0x4d / "M".
    OM_DBLS = UP('N', 'N'), // UC 0x4e / "N".
    OM_HLDS = UP('O', 'O'), // UC 0x4f / "O".
    OM_RELS = UP('P', 'P'), // UC 0x50 / "P".
};

/**
 * @brief Process mouse movement.
 * @return false to indicate the key was consumed.
 */
bool process_orbital_mouse(uint16_t keycode, keyrecord_t *record);

/**
 * @brief Mouse task function.
 *
 * Include this in your housekeeping_task_user() function.
 */
void orbital_mouse_task(void);

/**
 * @brief Gets the mouse heading direction.
 * @return Heading angle in [0, 63], where 0 = up, increasing counterclockwise.
 */
uint8_t get_orbital_mouse_angle(void);

/**
 * @brief Sets the mouse heading direction.
 * @param angle Heading angle, which is automatically wrapped to [0, 63].
 */
void set_orbital_mouse_angle(uint8_t angle);

/**
 * @brief Set the mouse speed curve at run time.
 *
 * This function is useful to make dynamic changes to the speed curve during use.
 * If speed_curve is NULL, the speed curve defined by `ORBITAL_MOUSE_SPEED_CURVE`
 * is used. Otherwise this is a table of 16 `uint8_t` values.
 *
 * The `n`th curve value represents speed at time 0.256 * n seconds,
 * interpolating piecewise linearly between these sampling points. Curve values
 * are interpreted as speed in pixels per 16 ms interval.
 */
void set_orbital_mouse_speed_curve(const uint8_t *speed_curve);