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

/** Send the host's conventional screen-lock shortcut(s). */
bool process_screen_lock(uint16_t keycode, keyrecord_t *record) {
    if (keycode != SCREEN_LOCK) {
        return true;
    }

    if (record->event.pressed) {
        switch (detected_host_os()) {
            case OS_MACOS:
                tap_code16(LCTL(LGUI(KC_Q)));
                break;
            case OS_LINUX:
                tap_code16(LGUI(KC_L));
                tap_code16(LCTL(LALT(KC_L)));
                break;
            default:
                tap_code16(LGUI(KC_L));
                break;
        }
    }

    return false;
}
