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

#pragma once

/* Select Word機能の設定 */
#define SELECT_WORD_TIMEOUT 2000  // 2秒のアイドルタイムアウト

/* タップホールド設定 */
#define TAPPING_TERM 200
#define PERMISSIVE_HOLD
#define HOLD_ON_OTHER_KEY_PRESS

/* Orbital Mouse設定 */
#define ORBITAL_MOUSE_RADIUS 36
#define ORBITAL_MOUSE_SPEED_CURVE \
      {24, 24, 24, 32, 58, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66}
#define ORBITAL_MOUSE_SLOW_MOVE_FACTOR 0.333
#define ORBITAL_MOUSE_SLOW_TURN_FACTOR 0.5
#define ORBITAL_MOUSE_WHEEL_SPEED 0.2