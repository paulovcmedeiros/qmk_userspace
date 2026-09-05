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

#include "keymap.h"

#define MOUSE_DOUBLE_TAP_TERM 100

typedef struct {
    uint16_t press_timer;
    uint16_t next_tap_timer;
    bool     waiting_for_next_tap;
    bool     accelerated;
} mouse_double_tap_state_t;

static mouse_double_tap_state_t mouse_up_double_tap;
static mouse_double_tap_state_t mouse_down_double_tap;
static mouse_double_tap_state_t mouse_left_double_tap;
static mouse_double_tap_state_t mouse_right_double_tap;

/** Accelerate mouse movement in any direction while a second tap is held. */
void process_fast_mouse_double_tap(uint16_t keycode, keyrecord_t *record) {
    mouse_double_tap_state_t *state;

    switch (keycode) {
        case MS_UP:
            state = &mouse_up_double_tap;
            break;
        case MS_DOWN:
            state = &mouse_down_double_tap;
            break;
        case MS_LEFT:
            state = &mouse_left_double_tap;
            break;
        case MS_RGHT:
            state = &mouse_right_double_tap;
            break;
        default:
            if (record->event.pressed) {
                mouse_up_double_tap.waiting_for_next_tap    = false;
                mouse_down_double_tap.waiting_for_next_tap  = false;
                mouse_left_double_tap.waiting_for_next_tap  = false;
                mouse_right_double_tap.waiting_for_next_tap = false;
            }
            return;
    }

    if (record->event.pressed) {
        bool is_double_tap = state->waiting_for_next_tap && timer_elapsed(state->next_tap_timer) <= MOUSE_DOUBLE_TAP_TERM;

        mouse_up_double_tap.waiting_for_next_tap    = false;
        mouse_down_double_tap.waiting_for_next_tap  = false;
        mouse_left_double_tap.waiting_for_next_tap  = false;
        mouse_right_double_tap.waiting_for_next_tap = false;

        if (is_double_tap) {
            state->accelerated = true;
            register_code16(MS_ACL2);
            return;
        }

        state->press_timer = timer_read();
        return;
    }

    if (state->accelerated) {
        state->accelerated = false;
        unregister_code16(MS_ACL2);
        return;
    }

    state->waiting_for_next_tap = timer_elapsed(state->press_timer) <= MOUSE_DOUBLE_TAP_TERM;
    state->next_tap_timer       = timer_read();
}
