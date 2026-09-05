/* Copyright 2026 Paulo V. C. Medeiros <paulo@medeiros.se>
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

#include QMK_KEYBOARD_H

enum custom_keycodes {
    PYTHON_SHEBANG = SAFE_RANGE,
    BASH_SHEBANG,
    HOME_SLASH,
    HOLD_MAKE,
    HOLD_EE_CLEAR,
    HOLD_SHOW_VERSION,
    COMM_SP,
    SCLN_SP,
    DOT_SP,
    SP_MINS,
};

// Called in order by process_record_user; false consumes the event.
void process_fast_mouse_double_tap(uint16_t keycode, keyrecord_t *record);
bool process_typing_macros(uint16_t keycode, keyrecord_t *record);
bool process_system_actions(uint16_t keycode, keyrecord_t *record);
bool process_modified_space(uint16_t keycode, keyrecord_t *record);

bool pre_process_key_lock(uint16_t keycode, keyrecord_t *record);
void typing_macros_task(void);
void system_actions_task(void);
void system_actions_init(void);
void system_actions_housekeeping_task(void);

// Returns true while system-action feedback overrides the normal indicators.
bool system_actions_indicators(uint8_t indicator_led, uint8_t led_min, uint8_t led_max);
