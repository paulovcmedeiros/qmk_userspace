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

#define THUMB_AUTO_SHIFT_TOGGLE_TERM 1000
// Matrix position of the left thumb-cluster Shift in LAYOUT.
#define THUMB_AUTO_SHIFT_TOGGLE_ROW 4
#define THUMB_AUTO_SHIFT_TOGGLE_COL 3

static bool     thumb_auto_shift_toggle_pending;
static uint16_t thumb_auto_shift_toggle_timer;

bool is_thumb_auto_shift_toggle_key(keypos_t key) {
    return key.row == THUMB_AUTO_SHIFT_TOGGLE_ROW && key.col == THUMB_AUTO_SHIFT_TOGGLE_COL;
}

bool thumb_auto_shift_toggle_ready(void) {
    return thumb_auto_shift_toggle_pending && timer_elapsed(thumb_auto_shift_toggle_timer) >= THUMB_AUTO_SHIFT_TOGGLE_TERM;
}

static bool no_other_keys_pressed(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if ((row != THUMB_AUTO_SHIFT_TOGGLE_ROW || col != THUMB_AUTO_SHIFT_TOGGLE_COL) && matrix_is_on(row, col)) {
                return false;
            }
        }
    }

    return true;
}

void process_thumb_auto_shift_toggle(uint16_t keycode, keyrecord_t *record, uint8_t base_layer) {
    bool is_toggle_key = keycode == KC_LSFT && is_thumb_auto_shift_toggle_key(record->event.key);

    if (record->event.pressed) {
        if (is_toggle_key && get_highest_layer(layer_state) == base_layer && no_other_keys_pressed()) {
            thumb_auto_shift_toggle_pending = true;
            thumb_auto_shift_toggle_timer   = timer_read();
        } else {
            // Any other keypress cancels the pending toggle.
            thumb_auto_shift_toggle_pending = false;
        }
    } else if (is_toggle_key && thumb_auto_shift_toggle_pending) {
        if (thumb_auto_shift_toggle_ready()) {
            autoshift_toggle();
        }
        thumb_auto_shift_toggle_pending = false;
    }
}
