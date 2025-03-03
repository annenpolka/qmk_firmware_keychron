/* Copyright 2023 Pascal Kolkman (@pascalkolkman)
 * Copyright 2023 Guillem Moreso (@guillem-ms)
 * Copyright 2023 Hugh Lang-Bulley (@hughlb)
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "orbital_mouse.h"
#include <stdlib.h>

// Orbital Mouse configuration options.
#ifndef ORBITAL_MOUSE_RADIUS
#    define ORBITAL_MOUSE_RADIUS 36 // Must be ≤ 63.
#endif
#ifndef ORBITAL_MOUSE_SPEED_CURVE
#    define ORBITAL_MOUSE_SPEED_CURVE                                          \
        { 24, 24, 24, 32, 58, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66 }
#endif
#ifndef ORBITAL_MOUSE_SLOW_MOVE_FACTOR
#    define ORBITAL_MOUSE_SLOW_MOVE_FACTOR 0.333
#endif
#ifndef ORBITAL_MOUSE_SLOW_TURN_FACTOR
#    define ORBITAL_MOUSE_SLOW_TURN_FACTOR 0.5
#endif
#ifndef ORBITAL_MOUSE_WHEEL_SPEED
#    define ORBITAL_MOUSE_WHEEL_SPEED 0.2
#endif
#ifndef ORBITAL_MOUSE_DBL_DELAY_MS
#    define ORBITAL_MOUSE_DBL_DELAY_MS 50
#endif

// Clamp "val" to [min, max].
#define CLAMP(val, min, max) (val < min ? min : val > max ? max : val)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"

// Speed curve as 16 values in [0, 255] for speeds at times 0, 0.25, 0.5, ..., 3.75 seconds.
static const uint8_t default_speed_curve[16] = ORBITAL_MOUSE_SPEED_CURVE;
static const uint8_t *speed_curve = default_speed_curve;

// Slow move and turn factors as Q8.8 fixed point values in [0, 1].
static const int16_t slow_move_factor = (int16_t)(ORBITAL_MOUSE_SLOW_MOVE_FACTOR * 256);
static const int16_t slow_turn_factor = (int16_t)(ORBITAL_MOUSE_SLOW_TURN_FACTOR * 256);

// Mouse wheel turn speed as a Q4.4 fixed point value in [0, 3.99].
static const int8_t wheel_speed = (int8_t)(ORBITAL_MOUSE_WHEEL_SPEED * 16);

// Time in ms between clicks with `OM_DBLS` double clicking.
static const uint8_t double_click_delay = ORBITAL_MOUSE_DBL_DELAY_MS;

#pragma GCC diagnostic pop

static struct {
    bool active;       // Currently moving the mouse.
    bool slow;         // True iff slow mode is active.
    uint8_t angle;     // Heading angle [0, 63], where 0 = Up, 16 = Left, 32 = Down, 48 = Right.
    int8_t fwd;        // Move forward (> 0) or backward (< 0).
    int8_t turn;       // Turn counterclockwise (> 0) or clockwise (< 0).
    uint16_t elapsed_time_ms; // Elapsed time since beginning a move.
    uint8_t selected_btn;     // Selected mouse button in [1, 8].
    uint8_t double_click_btn; // Non-zero if in progress double clicking the button.
    bool btn_held;     // True if a button is being held.
    bool wheel_up;     // Wheel up pressed.
    bool wheel_down;   // Wheel down pressed.
    bool wheel_left;   // Wheel left pressed.
    bool wheel_right;  // Wheel right pressed.
} state = {
    .selected_btn = 1,
};

// Sine function for unsigned phase in range [0, 63] and amplitude in [0, 255].
// Accuracy is within ±0.1%.
static int16_t scaled_sin(uint8_t phase, uint16_t amplitude) {
    // Converts phase to [0, 3) and amplitude to a Q8.8 value.
    phase &= 63;
    uint8_t quad = phase >> 4;
    int8_t x = (phase & 15) - 8;
    if (x < 0) { x = -x; }
    
    int16_t y;
    // y = A * (1 - x * x / 64) is a parabola approximation of sin(π/2 * x / 8) for x in [0, 8].
    if (amplitude < 256) {
        amplitude <<= 8;
        y = amplitude - (((amplitude >> 7) * (int16_t)x * (int16_t)x + 128) >> 8);
    } else {
        y = (amplitude - (((amplitude >> 8) * (int16_t)x * (int16_t)x + 128) >> 7)) << 1;
    }
    
    return (quad & 1) ? -y : y;
}

// Cosine function for unsigned phase in range [0, 63] and amplitude in [0, 255].
static inline int16_t scaled_cos(uint8_t phase, uint16_t amplitude) {
    return scaled_sin(phase + 16, amplitude);
}

uint8_t get_orbital_mouse_angle(void) {
    return state.angle;
}

void set_orbital_mouse_angle(uint8_t angle) {
    state.angle = angle & 63;
}

void set_orbital_mouse_speed_curve(const uint8_t *curve) {
    speed_curve = curve ? curve : default_speed_curve;
}

bool process_orbital_mouse(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // Direction:
        case OM_U:
            if (record->event.pressed) {
                state.fwd = 1;
                state.active = true;
                state.elapsed_time_ms = 0;
            } else if (state.fwd > 0) {
                // Only reset if this key caused Forward movement.
                state.fwd = 0;
            }
            break;
        case OM_D:
            if (record->event.pressed) {
                state.fwd = -1;
                state.active = true;
                state.elapsed_time_ms = 0;
            } else if (state.fwd < 0) {
                // Only reset if this key caused Backward movement.
                state.fwd = 0;
            }
            break;
        case OM_L:
            if (record->event.pressed) {
                state.turn = 1;
                state.active = true;
                state.elapsed_time_ms = 0;
            } else if (state.turn > 0) {
                // Only reset if this key caused CCW turning.
                state.turn = 0;
            }
            break;
        case OM_R:
            if (record->event.pressed) {
                state.turn = -1;
                state.active = true;
                state.elapsed_time_ms = 0;
            } else if (state.turn < 0) {
                // Only reset if this key caused CW turning.
                state.turn = 0;
            }
            break;
            
        case OM_SLOW:
            state.slow = record->event.pressed;
            break;
            
        // Mouse buttons:
        case OM_BTN1:
        case OM_BTN2:
        case OM_BTN3:
        case OM_BTN4:
        case OM_BTN5:
        case OM_BTN6:
        case OM_BTN7:
        case OM_BTN8: {
            uint8_t button = keycode - OM_BTN1 + 1;
            report_mouse_t report = pointing_device_get_report();
            if (record->event.pressed) {
                // Press the button.
                switch (button) {
                    case 1: report.buttons |= MOUSE_BTN1; break;
                    case 2: report.buttons |= MOUSE_BTN2; break;
                    case 3: report.buttons |= MOUSE_BTN3; break;
                    case 4: report.buttons |= MOUSE_BTN4; break;
                    case 5: report.buttons |= MOUSE_BTN5; break;
                    case 6: report.buttons |= MOUSE_BTN6; break;
                    case 7: report.buttons |= MOUSE_BTN7; break;
                    case 8: report.buttons |= MOUSE_BTN8; break;
                }
            } else {
                // Release the button.
                switch (button) {
                    case 1: report.buttons &= ~MOUSE_BTN1; break;
                    case 2: report.buttons &= ~MOUSE_BTN2; break;
                    case 3: report.buttons &= ~MOUSE_BTN3; break;
                    case 4: report.buttons &= ~MOUSE_BTN4; break;
                    case 5: report.buttons &= ~MOUSE_BTN5; break;
                    case 6: report.buttons &= ~MOUSE_BTN6; break;
                    case 7: report.buttons &= ~MOUSE_BTN7; break;
                    case 8: report.buttons &= ~MOUSE_BTN8; break;
                }
            }
            pointing_device_set_report(report);
            pointing_device_send();
            break;
        }
            
        // Mouse wheel:
        case OM_W_U:
            state.wheel_up = record->event.pressed;
            if (record->event.pressed) {
                state.active = true;
            }
            break;
        case OM_W_D:
            state.wheel_down = record->event.pressed;
            if (record->event.pressed) {
                state.active = true;
            }
            break;
        case OM_W_L:
            state.wheel_left = record->event.pressed;
            if (record->event.pressed) {
                state.active = true;
            }
            break;
        case OM_W_R:
            state.wheel_right = record->event.pressed;
            if (record->event.pressed) {
                state.active = true;
            }
            break;
            
        // Selected-mouse-button:
        case OM_SEL1:
        case OM_SEL2:
        case OM_SEL3:
        case OM_SEL4:
        case OM_SEL5:
        case OM_SEL6:
        case OM_SEL7:
        case OM_SEL8:
            if (record->event.pressed) {
                state.selected_btn = keycode - OM_SEL1 + 1;
            }
            break;
            
        case OM_BTNS: {
            report_mouse_t report = pointing_device_get_report();
            if (record->event.pressed) {
                // Press the button.
                switch (state.selected_btn) {
                    case 1: report.buttons |= MOUSE_BTN1; break;
                    case 2: report.buttons |= MOUSE_BTN2; break;
                    case 3: report.buttons |= MOUSE_BTN3; break;
                    case 4: report.buttons |= MOUSE_BTN4; break;
                    case 5: report.buttons |= MOUSE_BTN5; break;
                    case 6: report.buttons |= MOUSE_BTN6; break;
                    case 7: report.buttons |= MOUSE_BTN7; break;
                    case 8: report.buttons |= MOUSE_BTN8; break;
                }
            } else {
                // Release the button.
                switch (state.selected_btn) {
                    case 1: report.buttons &= ~MOUSE_BTN1; break;
                    case 2: report.buttons &= ~MOUSE_BTN2; break;
                    case 3: report.buttons &= ~MOUSE_BTN3; break;
                    case 4: report.buttons &= ~MOUSE_BTN4; break;
                    case 5: report.buttons &= ~MOUSE_BTN5; break;
                    case 6: report.buttons &= ~MOUSE_BTN6; break;
                    case 7: report.buttons &= ~MOUSE_BTN7; break;
                    case 8: report.buttons &= ~MOUSE_BTN8; break;
                }
            }
            pointing_device_set_report(report);
            pointing_device_send();
            break;
        }
            
        case OM_DBLS:
            if (record->event.pressed) {
                // Double-click the selected button. The first click is issued now in the
                // key press handler, while the second click is done by the task.
                report_mouse_t report = pointing_device_get_report();
                
                // Press and release the button.
                switch (state.selected_btn) {
                    case 1: report.buttons |= MOUSE_BTN1; break;
                    case 2: report.buttons |= MOUSE_BTN2; break;
                    case 3: report.buttons |= MOUSE_BTN3; break;
                    case 4: report.buttons |= MOUSE_BTN4; break;
                    case 5: report.buttons |= MOUSE_BTN5; break;
                    case 6: report.buttons |= MOUSE_BTN6; break;
                    case 7: report.buttons |= MOUSE_BTN7; break;
                    case 8: report.buttons |= MOUSE_BTN8; break;
                }
                
                pointing_device_set_report(report);
                pointing_device_send();
                
                switch (state.selected_btn) {
                    case 1: report.buttons &= ~MOUSE_BTN1; break;
                    case 2: report.buttons &= ~MOUSE_BTN2; break;
                    case 3: report.buttons &= ~MOUSE_BTN3; break;
                    case 4: report.buttons &= ~MOUSE_BTN4; break;
                    case 5: report.buttons &= ~MOUSE_BTN5; break;
                    case 6: report.buttons &= ~MOUSE_BTN6; break;
                    case 7: report.buttons &= ~MOUSE_BTN7; break;
                    case 8: report.buttons &= ~MOUSE_BTN8; break;
                }
                
                pointing_device_set_report(report);
                pointing_device_send();
                
                // Schedule the second click in double_click_delay ms, to be done in the task.
                state.double_click_btn = state.selected_btn;
                state.active = true;
                state.elapsed_time_ms = 0;
            }
            break;
            
        case OM_HLDS:
            if (record->event.pressed) {
                // Note that the button is being held.
                state.btn_held = true;
                
                // Press the button.
                report_mouse_t report = pointing_device_get_report();
                switch (state.selected_btn) {
                    case 1: report.buttons |= MOUSE_BTN1; break;
                    case 2: report.buttons |= MOUSE_BTN2; break;
                    case 3: report.buttons |= MOUSE_BTN3; break;
                    case 4: report.buttons |= MOUSE_BTN4; break;
                    case 5: report.buttons |= MOUSE_BTN5; break;
                    case 6: report.buttons |= MOUSE_BTN6; break;
                    case 7: report.buttons |= MOUSE_BTN7; break;
                    case 8: report.buttons |= MOUSE_BTN8; break;
                }
                pointing_device_set_report(report);
                pointing_device_send();
            }
            break;
            
        case OM_RELS:
            if (record->event.pressed && state.btn_held) {
                // Release button that was being held.
                state.btn_held = false;
                
                report_mouse_t report = pointing_device_get_report();
                switch (state.selected_btn) {
                    case 1: report.buttons &= ~MOUSE_BTN1; break;
                    case 2: report.buttons &= ~MOUSE_BTN2; break;
                    case 3: report.buttons &= ~MOUSE_BTN3; break;
                    case 4: report.buttons &= ~MOUSE_BTN4; break;
                    case 5: report.buttons &= ~MOUSE_BTN5; break;
                    case 6: report.buttons &= ~MOUSE_BTN6; break;
                    case 7: report.buttons &= ~MOUSE_BTN7; break;
                    case 8: report.buttons &= ~MOUSE_BTN8; break;
                }
                pointing_device_set_report(report);
                pointing_device_send();
            }
            break;
            
        default:
            return true;
    }
    
    return false;
}

void orbital_mouse_task(void) {
    if (!state.active) { return; }
    
    // Only set active = false if we did nothing.
    bool did_something = false;
    
    if (state.double_click_btn != 0) {
        // If double-clicking, issue the second click after double_click_delay ms.
        state.elapsed_time_ms += 16;
        if (state.elapsed_time_ms >= double_click_delay) {
            // Press and release the button.
            report_mouse_t report = pointing_device_get_report();
            
            switch (state.double_click_btn) {
                case 1: report.buttons |= MOUSE_BTN1; break;
                case 2: report.buttons |= MOUSE_BTN2; break;
                case 3: report.buttons |= MOUSE_BTN3; break;
                case 4: report.buttons |= MOUSE_BTN4; break;
                case 5: report.buttons |= MOUSE_BTN5; break;
                case 6: report.buttons |= MOUSE_BTN6; break;
                case 7: report.buttons |= MOUSE_BTN7; break;
                case 8: report.buttons |= MOUSE_BTN8; break;
            }
            
            pointing_device_set_report(report);
            pointing_device_send();
            
            switch (state.double_click_btn) {
                case 1: report.buttons &= ~MOUSE_BTN1; break;
                case 2: report.buttons &= ~MOUSE_BTN2; break;
                case 3: report.buttons &= ~MOUSE_BTN3; break;
                case 4: report.buttons &= ~MOUSE_BTN4; break;
                case 5: report.buttons &= ~MOUSE_BTN5; break;
                case 6: report.buttons &= ~MOUSE_BTN6; break;
                case 7: report.buttons &= ~MOUSE_BTN7; break;
                case 8: report.buttons &= ~MOUSE_BTN8; break;
            }
            
            pointing_device_set_report(report);
            pointing_device_send();
            
            state.double_click_btn = 0;
            did_something = true;
        }
    }
    
    if (state.fwd || state.turn) {
        // Calculate speed from the speed curve.
        uint8_t idx = CLAMP(state.elapsed_time_ms / 256, 0, 15);
        uint16_t t_ratio = (state.elapsed_time_ms % 256) << 8;
        uint16_t t_reciprocal = 65536 - t_ratio;
        uint16_t interp_spd = (speed_curve[idx + 1] * t_ratio + speed_curve[idx] * t_reciprocal) >> 10;
        
        // From the current angle, compute direction cosines.
        int16_t dx = scaled_cos(state.angle, interp_spd) >> 8;
        int16_t dy = scaled_sin(state.angle, interp_spd) >> 8;
        
        if (state.fwd) {
            // Move forward or backward.
            dx = dx * state.fwd;
            dy = dy * state.fwd;
            
            if (state.slow) {
                dx = (dx * slow_move_factor) >> 8;
                dy = (dy * slow_move_factor) >> 8;
            }
            
            report_mouse_t report = pointing_device_get_report();
            report.x = dx;
            report.y = -dy; // Mouse Y+ is to the bottom of the screen.
            pointing_device_set_report(report);
            pointing_device_send();
            
            did_something = true;
        }
        
        if (state.turn) {
            // Turn to change the heading direction.
            uint16_t radius = ORBITAL_MOUSE_RADIUS;
            if (state.slow) {
                radius = (radius * slow_turn_factor) >> 8;
            }
            
            // Update the angle.
            if (state.turn > 0) { // CCW.
                state.angle = (state.angle + 1) & 63;
            } else { // CW.
                state.angle = (state.angle - 1) & 63;
            }
            
            // From the new angle, compute dxdy to show rotational motion.
            int16_t new_dx = scaled_cos(state.angle, radius);
            int16_t new_dy = scaled_sin(state.angle, radius);
            
            report_mouse_t report = pointing_device_get_report();
            report.x = (new_dx - dx) >> 8;
            report.y = -(new_dy - dy) >> 8; // Mouse Y+ is to the bottom of the screen.
            pointing_device_set_report(report);
            pointing_device_send();
            
            did_something = true;
        }
        
        // Update elapsed time.
        state.elapsed_time_ms += 16;
    }
    
    if (state.wheel_up || state.wheel_down || state.wheel_left || state.wheel_right) {
        // Update mouse wheel.
        report_mouse_t report = pointing_device_get_report();
        
        if (state.wheel_up) {
            report.v = wheel_speed;
        } else if (state.wheel_down) {
            report.v = -wheel_speed;
        }
        
        if (state.wheel_left) {
            report.h = -wheel_speed;
        } else if (state.wheel_right) {
            report.h = wheel_speed;
        }
        
        pointing_device_set_report(report);
        pointing_device_send();
        
        did_something = true;
    }
    
    state.active = did_something;
}