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

static bool     key_lock_pending;
static uint16_t locked_keycode = KC_NO;

bool key_lock_is_active(void) {
    return key_lock_pending || locked_keycode != KC_NO;
}

/**
 * Let QK_LOCK release the key it locked, in addition to its default behavior.
 *
 * This hook runs before QMK's Key Lock handler. It only consumes QK_LOCK when
 * a key is already locked; all other events continue through the built-in
 * handler unchanged.
 */
bool pre_process_key_lock(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }

    if (keycode == QK_LOCK) {
        if (locked_keycode != KC_NO) {
            uint16_t keycode_to_unlock = locked_keycode;

            process_key_lock(&keycode_to_unlock, record);
            unregister_code16(keycode_to_unlock);
            locked_keycode = KC_NO;
            return false;
        }

        key_lock_pending = !key_lock_pending;
        return true;
    }

    if (key_lock_pending) {
        key_lock_pending = false;
        if (keycode <= 0xFF) {
            locked_keycode = keycode;
        }
    } else if (keycode == locked_keycode) {
        locked_keycode = KC_NO;
    }

    return true;
}
