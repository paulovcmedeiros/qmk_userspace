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

#include "transactions.h"

#define MODE_INDICATOR_SYNC_RETRY_TERM 50
#define MODE_INDICATOR_COLOR 32, 0, 32
#define MODE_INDICATOR_READY_COLOR 64, 32, 0

enum mode_indicator_flags {
    MODE_INDICATOR_AUTO_SHIFT = 1 << 0,
    MODE_INDICATOR_CAPS_WORD  = 1 << 1,
    MODE_INDICATOR_KEY_LOCK   = 1 << 2,
    MODE_INDICATOR_READY      = 1 << 3,
};

typedef struct {
    uint8_t active_modes;
} mode_indicator_sync_t;

static uint8_t  secondary_active_modes;
static uint8_t  last_synced_active_modes;
static bool     mode_indicator_sync_pending = true;
static uint16_t mode_indicator_sync_timer;

static uint8_t get_active_modes(void) {
    uint8_t active_modes = 0;

    if (get_autoshift_state()) {
        active_modes |= MODE_INDICATOR_AUTO_SHIFT;
    }
    if (is_caps_word_on()) {
        active_modes |= MODE_INDICATOR_CAPS_WORD;
    }
    if (key_lock_is_active()) {
        active_modes |= MODE_INDICATOR_KEY_LOCK;
    }
    if (thumb_auto_shift_toggle_ready()) {
        active_modes |= MODE_INDICATOR_READY;
    }

    return active_modes;
}

/** Mirror the primary half's active typing modes on the secondary half. */
static void mode_indicator_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (in_buflen == sizeof(mode_indicator_sync_t)) {
        const mode_indicator_sync_t *sync = in_data;

        secondary_active_modes = sync->active_modes;
    }
}

void mode_indicators_init(void) {
    transaction_register_rpc(MODE_INDICATOR_SYNC, mode_indicator_sync_handler);
}

void mode_indicators_housekeeping_task(void) {
    if (!is_keyboard_master()) {
        return;
    }

    uint8_t active_modes = get_active_modes();

    if (active_modes != last_synced_active_modes) {
        mode_indicator_sync_pending = true;
    }

    if (mode_indicator_sync_pending && timer_elapsed(mode_indicator_sync_timer) >= MODE_INDICATOR_SYNC_RETRY_TERM) {
        mode_indicator_sync_t sync = {.active_modes = active_modes};

        if (transaction_rpc_send(MODE_INDICATOR_SYNC, sizeof(sync), &sync)) {
            last_synced_active_modes    = active_modes;
            mode_indicator_sync_pending = false;
        }
        mode_indicator_sync_timer = timer_read();
    }
}

static bool is_shift_keycode(uint16_t keycode) {
    if (keycode == KC_LSFT || keycode == KC_RSFT) {
        return true;
    }

    return IS_QK_MOD_TAP(keycode) && (QK_MOD_TAP_GET_MODS(keycode) & MOD_LSFT);
}

static bool mode_is_active_for_key(uint16_t keycode, keypos_t key, uint8_t active_modes) {
    if (is_thumb_auto_shift_toggle_key(key) && (active_modes & MODE_INDICATOR_AUTO_SHIFT)) {
        return true;
    }
    if (keycode == QK_LOCK) {
        return active_modes & MODE_INDICATOR_KEY_LOCK;
    }

    return is_shift_keycode(keycode) && (active_modes & MODE_INDICATOR_CAPS_WORD);
}

void mode_indicators_render(uint8_t base_layer, uint8_t led_min, uint8_t led_max) {
    uint8_t active_modes = is_keyboard_master() ? get_active_modes() : secondary_active_modes;

    if (active_modes == 0) {
        return;
    }

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];

            if (led >= led_min && led < led_max && led != NO_LED) {
                keypos_t key = {col, row};

                if ((active_modes & MODE_INDICATOR_READY) && is_thumb_auto_shift_toggle_key(key)) {
                    rgb_matrix_set_color(led, MODE_INDICATOR_READY_COLOR);
                } else if (mode_is_active_for_key(keymap_key_to_keycode(base_layer, key), key, active_modes)) {
                    rgb_matrix_set_color(led, MODE_INDICATOR_COLOR);
                }
            }
        }
    }
}
