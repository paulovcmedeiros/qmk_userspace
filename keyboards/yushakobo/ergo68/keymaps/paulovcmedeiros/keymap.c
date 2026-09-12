/* Copyright 2023 yushakobo
 * Copyright 2026 Paulo V. C. Medeiros <paulo@medeiros.se>
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
#include "keycodes.h"

/*
 * Host layout assumption
 *
 * This keymap targets EurKEY. Its base and shifted symbol positions are
 * compatible with US QWERTY, so expressions such as S(KC_3), KC_LBRC, and
 * QMK's default SEND_STRING translation intentionally use US key positions.
 *
 * Right Alt serves as EurKEY's AltGr modifier, so its combinations rely on
 * that layout. Changing the host layout requires auditing both keymap entries
 * and all SEND_STRING macros.
 */

enum layer_names {
    BASE,
    MOUSE,
    NUMPAD,
    SYMBOLS,
};

enum combo_events {
    THUMBS_ENTER,
};

/* Set the thumb Enter combo’s recognition window independently. */
#define THUMBS_ENTER_COMBO_TERM 40

// Physical RGB matrix indexes reserved for status indicators.
enum indicator_leds {
    CAPS_LOCK_LED   = 0,
    NUM_LOCK_LED    = 1,
    SCROLL_LOCK_LED = 2,
    MOUSE_LAYER_LED = 37,
    NUMPAD_LAYER_LED,
    SYMBOLS_LAYER_LED,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Layer 0: Base
    [BASE] = LAYOUT(
        QK_GESC,          KC_1,    KC_2,    KC_3,    KC_4,            KC_5,                                                                 KC_6,                KC_7,           KC_8,    KC_9,    KC_0,    KC_BSPC,
        KC_TAB,           KC_Q,    KC_W,    KC_E,    KC_R,            KC_T,               KC_DEL,                   KC_DEL,         KC_Y,                KC_U,           KC_I,        KC_O,        KC_P,    KC_RCTL,
        KC_GRV,           KC_A,    KC_S,    KC_D,    KC_F,            KC_G,               KC_PSCR,                  KC_INS,         KC_H,                KC_J,           KC_K,        KC_L,        KC_SCLN, KC_QUOT,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,            KC_B,               XXXXXXX,                  XXXXXXX,        KC_N,                KC_M,           KC_COMM,     KC_DOT,      XXXXXXX, KC_RSFT,
        KC_LCTL,  XXXXXXX, KC_LGUI, KC_LSFT, LALT_T(KC_SPC),          LT(MOUSE, KC_SPC),  LT(NUMPAD, KC_SPC),       RSFT_T(KC_SPC), LT(SYMBOLS, KC_SPC), RALT_T(KC_SPC), KC_RSFT,     KC_MUTE,     XXXXXXX, RCTL_T(KC_ENT)
    ),
    // Layer 1: Mouse
    [MOUSE] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                           XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX, MS_WHLU, MS_WHLD,
        KC_LSFT, XXXXXXX, MS_LEFT, MS_UP,   MS_RGHT, MS_BTN1, XXXXXXX, KC_HOME, XXXXXXX,          XXXXXXX, KC_UP,   XXXXXXX, KC_PGUP, KC_PGDN,
        MS_BTN1, MS_BTN2, MS_LEFT, MS_DOWN, MS_RGHT, MS_BTN1, MS_BTN4, KC_END,  XXXXXXX,          KC_LEFT, KC_DOWN, KC_RGHT, XXXXXXX, KC_ENTER,
        MS_BTN3, MS_BTN2, XXXXXXX, XXXXXXX, MS_BTN3, XXXXXXX, MS_BTN5, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_BSPC, RSFT_T(KC_BSPC),  KC_BSPC, XXXXXXX, XXXXXXX, XXXXXXX, QK_LLCK
    ),
    // Layer 2: Numpad
    [NUMPAD] = LAYOUT(
        SCREEN_LOCK, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                                     XXXXXXX, KC_7,    KC_8, KC_9,    XXXXXXX,     XXXXXXX,
        XXXXXXX,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, HOLD_MAKE,         XXXXXXX, KC_PSLS, KC_4,    KC_5,     KC_6,    KC_KP_ASTERISK,  XXXXXXX,
        XXXXXXX,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, HOLD_EE_CLEAR,     XXXXXXX, KC_PMNS, KC_1,    KC_2,     KC_3,    KC_PPLS,         KC_ENTER,
        XXXXXXX,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, HOLD_SHOW_VERSION, XXXXXXX, XXXXXXX, KC_DOT,  KC_0,     XXXXXXX, XXXXXXX,         XXXXXXX,
        RM_NEXT,     RM_TOGG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,           KC_RSFT, KC_BSPC, KC_0, KC_RSFT,     XXXXXXX, XXXXXXX,         QK_LLCK
    ),
    // Layer 3: Symbols
    [SYMBOLS] = LAYOUT(
        XXXXXXX,    XXXXXXX, XXXXXXX, S(KC_COMM), S(KC_DOT),  XXXXXXX,                                            S(KC_6),    S(KC_7),   S(KC_8),    S(KC_BSLS), KC_PLUS,  XXXXXXX,
        XXXXXXX,    XXXXXXX, S(KC_4), S(KC_LBRC), S(KC_RBRC), KC_EQL,      PYTHON_SHEBANG,       XXXXXXX, XXXXXXX,    XXXXXXX,   DOT_SP,      SP_MINS,        XXXXXXX,  XXXXXXX,
        S(KC_GRV),  XXXXXXX, S(KC_3), KC_LBRC,    KC_RBRC,    SP_MINS,     BASH_SHEBANG,         XXXXXXX, S(KC_SCLN), SCLN_SP,   COMM_SP,     S(KC_SLSH),     S(KC_1),  XXXXXXX,
        XXXXXXX,    XXXXXXX, S(KC_5), S(KC_9),    S(KC_0),    S(KC_MINS),  XXXXXXX,              XXXXXXX, XXXXXXX,    XXXXXXX,   XXXXXXX,     XXXXXXX,        XXXXXXX,  XXXXXXX,
        XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX,    KC_BSLS,    DOT_SLASH,   HOME_SLASH,           XXXXXXX, XXXXXXX,    XXXXXXX,   XXXXXXX,     XXXXXXX,        XXXXXXX,  QK_LLCK
    )
};
// clang-format on

const uint16_t PROGMEM thumbs_enter_combo[] = {LT(MOUSE, KC_SPC), LT(SYMBOLS, KC_SPC), COMBO_END};

combo_t key_combos[] = {
    [THUMBS_ENTER] = COMBO(thumbs_enter_combo, KC_ENT),
};

uint16_t get_combo_term(uint16_t combo_index, combo_t *combo) {
    return combo_index == THUMBS_ENTER ? THUMBS_ENTER_COMBO_TERM : COMBO_TERM;
}

bool get_combo_must_tap(uint16_t combo_index, combo_t *combo) {
    return true;
}

bool combo_should_trigger(uint16_t combo_index, combo_t *combo, uint16_t keycode, keyrecord_t *record) {
    if (get_highest_layer(layer_state) != BASE) {
        return false;
    }

    return combo_index != THUMBS_ENTER || !(get_mods() & (MOD_MASK_SHIFT | MOD_MASK_ALT));
}

void keyboard_post_init_user(void) {
    system_actions_init();
    mode_indicators_init();
}

void housekeeping_task_user(void) {
    system_actions_housekeeping_task();
    mode_indicators_housekeeping_task();
}

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    process_thumb_auto_shift_toggle(keycode, record, BASE);
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    process_fast_mouse_double_tap(keycode, record);

    if (!process_typing_macros(keycode, record)) {
        return false;
    }

    if (!process_screen_lock(keycode, record)) {
        return false;
    }

    if (!process_system_actions(keycode, record)) {
        return false;
    }

    return process_modified_space(keycode, record);
}

void matrix_scan_user(void) {
    typing_macros_task();
    system_actions_task();
}

/**
 * Set a lock indicator only when its LED is in QMK's current render range.
 *
 * The parameter names led_min and led_max are required by
 * RGB_MATRIX_INDICATOR_SET_COLOR.
 */
static void set_lock_indicator(uint8_t led, bool active, uint8_t led_min, uint8_t led_max) {
    if (active) {
        RGB_MATRIX_INDICATOR_SET_COLOR(led, 0, 0, 128);
    } else {
        RGB_MATRIX_INDICATOR_SET_COLOR(led, 0, 0, 0);
    }
}

/** Color an LED according to the layer it represents. */
static void set_layer_color(uint8_t led, uint8_t layer, uint8_t led_min, uint8_t led_max) {
    if (led < led_min || led >= led_max) {
        return;
    }

    switch (layer) {
        case MOUSE:
            rgb_matrix_set_color(led, 0, 0, 128);
            break;
        case NUMPAD:
            rgb_matrix_set_color(led, 128, 40, 0);
            break;
        case SYMBOLS:
            rgb_matrix_set_color(led, 0, 128, 0);
            break;
    }
}

/** Turn off unassigned keys and highlight the active layer controls. */
static void render_key_indicators(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];

            if (led >= led_min && led < led_max && led != NO_LED) {
                uint16_t keycode = keymap_key_to_keycode(layer, (keypos_t){col, row});

                if (keycode == KC_NO) {
                    rgb_matrix_set_color(led, RGB_OFF);
                } else if (layer == BASE && IS_QK_LAYER_TAP(keycode)) {
                    set_layer_color(led, QK_LAYER_TAP_GET_LAYER(keycode), led_min, led_max);
                } else if (layer != BASE && keycode == QK_LLCK) {
                    set_layer_color(led, layer, led_min, led_max);
                }
            }
        }
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (system_actions_indicators(NUMPAD_LAYER_LED, led_min, led_max)) {
        return false;
    }

    render_key_indicators(led_min, led_max);

    led_t host_leds = host_keyboard_led_state();

    set_lock_indicator(CAPS_LOCK_LED, host_leds.caps_lock, led_min, led_max);
    set_lock_indicator(NUM_LOCK_LED, host_leds.num_lock, led_min, led_max);
    set_lock_indicator(SCROLL_LOCK_LED, host_leds.scroll_lock, led_min, led_max);
    mode_indicators_render(BASE, led_min, led_max);

    // layer state
    switch (get_highest_layer(layer_state)) {
        case MOUSE:
            set_layer_color(MOUSE_LAYER_LED, MOUSE, led_min, led_max);
            break;
        case NUMPAD:
            set_layer_color(NUMPAD_LAYER_LED, NUMPAD, led_min, led_max);
            break;
        case SYMBOLS:
            set_layer_color(SYMBOLS_LAYER_LED, SYMBOLS, led_min, led_max);
            break;
    }
    return false;
}
