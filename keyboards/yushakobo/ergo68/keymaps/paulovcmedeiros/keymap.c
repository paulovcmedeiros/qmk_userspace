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

#include "keycodes.h"
#include QMK_KEYBOARD_H

#include "process_quantum.h"
#include "transactions.h"
#include "version.h"

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

enum combo_events {
    THUMBS_ENTER,
};

#define HOLD_ACTION_TERM 2000
#define HOLD_ACTION_CONFIRMED_TERM 250
#define HOLD_ACTION_BLINK_TERM 250
#define HOLD_ACTION_SYNC_RETRY_TERM 50
#define MOUSE_DOUBLE_TAP_TERM 100
#define THUMBS_ENTER_COMBO_TERM 30

typedef enum {
    SYSTEM_ACTION_NONE,
    SYSTEM_ACTION_MAKE,
    SYSTEM_ACTION_EE_CLEAR,
    SYSTEM_ACTION_SHOW_VERSION,
} system_action_t;

typedef enum {
    SYSTEM_ACTION_FEEDBACK_IDLE,
    SYSTEM_ACTION_FEEDBACK_ARMING,
    SYSTEM_ACTION_FEEDBACK_CONFIRMED,
} system_action_feedback_t;

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
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,            KC_B,               QK_LOCK,                  S(KC_4),        KC_N,                KC_M,           KC_COMM,     KC_DOT,      KC_SLSH, KC_RSFT,
        LCTL_T(KC_BSLS),  AS_TOGG, KC_LGUI, KC_LSFT, LALT_T(KC_SPC),  LT(MOUSE, KC_SPC),  LT(NUMPAD, KC_SPC),       RSFT_T(KC_SPC), LT(SYMBOLS, KC_SPC), RALT_T(KC_SPC), KC_RSFT,     KC_MUTE,     UG_TOGG, RCTL_T(KC_ENT)
    ),
    // Layer 1: Mouse
    [MOUSE] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                           XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX, MS_WHLU, MS_WHLD,
        KC_LSFT, XXXXXXX, MS_LEFT, MS_UP,   MS_RGHT, MS_BTN1, XXXXXXX, KC_HOME, XXXXXXX,          XXXXXXX, KC_UP,   XXXXXXX, KC_PGUP, KC_PGDN,
        MS_BTN1, MS_BTN2, MS_LEFT, MS_DOWN, MS_RGHT, MS_BTN1, MS_BTN4, KC_END,  XXXXXXX,          KC_LEFT, KC_DOWN, KC_RGHT, XXXXXXX, KC_ENTER,
        MS_BTN3, MS_BTN2, XXXXXXX, XXXXXXX, MS_BTN3, XXXXXXX, MS_BTN5, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_BSPC, RSFT_T(KC_BSPC),  KC_BSPC, XXXXXXX, UG_PREV, UG_NEXT, XXXXXXX
    ),
    // Layer 2: Numpad
    [NUMPAD] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                                     XXXXXXX, KC_7,    KC_8,    KC_9,    XXXXXXX,     XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, HOLD_MAKE,         XXXXXXX, KC_PSLS, KC_4,    KC_5,        KC_6,    KC_KP_ASTERISK,  XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, HOLD_EE_CLEAR,     XXXXXXX, KC_PMNS, KC_1,    KC_2,        KC_3,    KC_PPLS,         KC_ENTER,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, HOLD_SHOW_VERSION, XXXXXXX, S(KC_5), KC_DOT,  KC_0,        XXXXXXX, XXXXXXX,         XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,           KC_BSPC, KC_BSPC, KC_BSPC, XXXXXXX,     XXXXXXX, XXXXXXX,         XXXXXXX
    ),
    // Layer 3: Symbols
    [SYMBOLS] = LAYOUT(
        XXXXXXX,    XXXXXXX, XXXXXXX, S(KC_COMM), S(KC_DOT),  XXXXXXX,                                            S(KC_6),    S(KC_7),   S(KC_8),    S(KC_BSLS), KC_PLUS,  XXXXXXX,
        XXXXXXX,    XXXXXXX, S(KC_4), S(KC_LBRC), S(KC_RBRC), KC_EQL,      PYTHON_SHEBANG,       XXXXXXX, XXXXXXX,    XXXXXXX,   DOT_SP,      SP_MINS,        XXXXXXX,  XXXXXXX,
        S(KC_GRV),  XXXXXXX, S(KC_3), KC_LBRC,    KC_RBRC,    SP_MINS,     BASH_SHEBANG,         XXXXXXX, S(KC_SCLN), SCLN_SP,   COMM_SP,     S(KC_SLSH),     S(KC_1),  XXXXXXX,
        XXXXXXX,    XXXXXXX, S(KC_5), S(KC_9),    S(KC_0),    S(KC_MINS),  XXXXXXX,              XXXXXXX, XXXXXXX,    XXXXXXX,   XXXXXXX,     XXXXXXX,        XXXXXXX,  XXXXXXX,
        XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX,    KC_BSLS,    HOME_SLASH,  XXXXXXX,              XXXXXXX, XXXXXXX,    XXXXXXX,   XXXXXXX,     XXXXXXX,        XXXXXXX,  XXXXXXX
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

static matrix_row_t             intercepted_space_taps[MATRIX_ROWS];
static system_action_t          system_action;
static system_action_feedback_t system_action_feedback;
static uint16_t                 system_action_timer;
static uint8_t                  system_action_mods;
static bool                     system_action_feedback_sync_pending = true;
static uint16_t                 system_action_feedback_sync_timer;
static bool                     show_version_pending;
static bool                     key_lock_pending;
static uint16_t                 locked_keycode = KC_NO;

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

typedef struct {
    bool     pressed;
    bool     hold_sent;
    uint16_t timer;
} tap_hold_state_t;

// Tap sends keypad slash; hold or interruption sends "~/".
static tap_hold_state_t home_slash;

// Tap sends minus; hold or interruption sends " -".
static tap_hold_state_t space_minus;

typedef struct {
    uint16_t custom_keycode;
    uint16_t tap_keycode;
    uint16_t timer;
    bool     pending;
} punctuation_space_t;

static punctuation_space_t punctuation_spaces[] = {
    {COMM_SP, KC_COMM},
    {SCLN_SP, KC_SCLN},
    {DOT_SP, KC_DOT},
};

typedef struct {
    char build_date[sizeof(QMK_BUILDDATE)];
} build_date_t;

typedef struct {
    uint8_t feedback;
} system_action_feedback_sync_t;

/** Return this half's build date to the primary half. */
static void build_date_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (out_buflen == sizeof(build_date_t)) {
        build_date_t *response = out_data;

        memcpy(response->build_date, QMK_BUILDDATE, sizeof(response->build_date));
    }
}

/** Mirror the primary half's system-action feedback on the secondary half. */
static void system_action_feedback_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (in_buflen == sizeof(system_action_feedback_sync_t)) {
        const system_action_feedback_sync_t *sync = in_data;

        if (sync->feedback <= SYSTEM_ACTION_FEEDBACK_CONFIRMED) {
            system_action_feedback = sync->feedback;
            system_action_timer    = timer_read();
        }
    }
}

/** Type the build dates in physical left-to-right order. */
static void send_build_dates(void) {
    build_date_t local     = {.build_date = QMK_BUILDDATE};
    build_date_t secondary = {0};
    bool         received  = transaction_rpc_recv(BUILD_DATE_SYNC, sizeof(secondary), &secondary);

    SEND_STRING("*** Yushakobo Ergo68 QMK firmware - keymap: paulovcmedeiros - build timestamps: < L: ");
    if (is_keyboard_left()) {
        send_string(local.build_date);
    } else if (received) {
        send_string(secondary.build_date);
    } else {
        SEND_STRING("unavailable");
    }

    SEND_STRING(" | R: ");
    if (!is_keyboard_left()) {
        send_string(local.build_date);
    } else if (received) {
        send_string(secondary.build_date);
    } else {
        SEND_STRING("unavailable");
    }
    SEND_STRING(" >  ***");
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(BUILD_DATE_SYNC, build_date_sync_handler);
    transaction_register_rpc(SYSTEM_ACTION_FEEDBACK_SYNC, system_action_feedback_sync_handler);
}

void housekeeping_task_user(void) {
    if (!is_keyboard_master()) {
        return;
    }

    if (system_action_feedback_sync_pending && timer_elapsed(system_action_feedback_sync_timer) >= HOLD_ACTION_SYNC_RETRY_TERM) {
        system_action_feedback_sync_t sync = {.feedback = system_action_feedback};

        system_action_feedback_sync_pending = !transaction_rpc_send(SYSTEM_ACTION_FEEDBACK_SYNC, sizeof(sync), &sync);
        system_action_feedback_sync_timer   = timer_read();
    }

    if (show_version_pending) {
        show_version_pending = false;
        send_build_dates();
    }
}

/**
 * Let QK_LOCK release the key it locked, in addition to its default behavior.
 *
 * This hook runs before QMK's Key Lock handler. It only consumes QK_LOCK when
 * a key is already locked; all other events continue through the built-in
 * handler unchanged.
 */
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
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

/** Send the hold action for HOME_SLASH and mark it as resolved. */
static void send_home_slash_hold(void) {
    SEND_STRING("~/");
    home_slash.hold_sent = true;
}

/** Send the hold action for SP_MINS and mark it as resolved. */
static void send_space_minus_hold(void) {
    SEND_STRING(" -");
    space_minus.hold_sent = true;
}

/** Change the RGB feedback phase and schedule a matching update for the other half. */
static void set_system_action_feedback(system_action_feedback_t feedback) {
    system_action_feedback              = feedback;
    system_action_timer                 = timer_read();
    system_action_feedback_sync_pending = true;
}

/** Arm or cancel one of the protected system actions. */
static void process_system_action(system_action_t action, keyrecord_t *record) {
    if (record->event.pressed) {
        if (system_action == SYSTEM_ACTION_NONE) {
            system_action = action;
            set_system_action_feedback(SYSTEM_ACTION_FEEDBACK_ARMING);
        }
    } else if (system_action == action && system_action_feedback == SYSTEM_ACTION_FEEDBACK_ARMING) {
        system_action = SYSTEM_ACTION_NONE;
        set_system_action_feedback(SYSTEM_ACTION_FEEDBACK_IDLE);
    }
}

/** Execute a confirmed system action after its visual acknowledgement. */
static void perform_system_action(void) {
    switch (system_action) {
        case SYSTEM_ACTION_MAKE:
            if ((system_action_mods & MOD_MASK_SHIFT) && (system_action_mods & MOD_MASK_CTRL)) {
                reset_keyboard();
            } else if (system_action_mods & MOD_MASK_SHIFT) {
                clear_mods();
                SEND_STRING_DELAY("f=\"$(qmk userspace-path)/yushakobo_ergo68_paulovcmedeiros.hex\"; "
                                  "if test -f \"$f\"; then qmk flash \"$f\"; "
                                  "else qmk flash -kb yushakobo/ergo68 -km paulovcmedeiros; fi" SS_TAP(X_ENTER),
                                  TAP_CODE_DELAY);
            } else {
                keyrecord_t make_record = {.event.pressed = true};

                process_quantum(QK_MAKE, &make_record);
            }
            break;

        case SYSTEM_ACTION_EE_CLEAR:
            eeconfig_disable();
            soft_reset_keyboard();
            break;

        case SYSTEM_ACTION_SHOW_VERSION:
            show_version_pending = true;
            break;

        case SYSTEM_ACTION_NONE:
            break;
    }

    system_action = SYSTEM_ACTION_NONE;
    set_system_action_feedback(SYSTEM_ACTION_FEEDBACK_IDLE);
}

/** Return whether this event represents a plain or dual-role Space tap. */
static bool is_space_tap(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_SPC) {
        return true;
    }

    if (record->tap.count == 0) {
        return false;
    }

    if (IS_QK_MOD_TAP(keycode)) {
        return QK_MOD_TAP_GET_TAP_KEYCODE(keycode) == KC_SPC;
    }

    if (IS_QK_LAYER_TAP(keycode)) {
        return QK_LAYER_TAP_GET_TAP_KEYCODE(keycode) == KC_SPC;
    }

    return false;
}

/**
 * Replace a Space tap with Backspace while Shift or Alt is held.
 *
 * The intercepted key position is remembered so its release is consumed too.
 * Returns false when the event has been handled.
 */
static bool process_modified_space(uint16_t keycode, keyrecord_t *record) {
    matrix_row_t key_mask = (matrix_row_t)1 << record->event.key.col;
    uint8_t      key_row  = record->event.key.row;

    if (!record->event.pressed && (intercepted_space_taps[key_row] & key_mask)) {
        intercepted_space_taps[key_row] &= ~key_mask;
        return false;
    }

    if (record->event.pressed && is_space_tap(keycode, record) && (get_mods() & (MOD_MASK_SHIFT | MOD_MASK_ALT))) {
        uint8_t saved_mods = get_mods();

        intercepted_space_taps[key_row] |= key_mask;
        del_mods(MOD_MASK_SHIFT | MOD_MASK_ALT);
        send_keyboard_report();
        tap_code(KC_BSPC);
        set_mods(saved_mods);
        send_keyboard_report();
        return false;
    }

    return true;
}

/** Accelerate mouse movement in any direction while a second tap is held. */
static void process_fast_mouse_double_tap(uint16_t keycode, keyrecord_t *record) {
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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    process_fast_mouse_double_tap(keycode, record);

    // Resolve HOME_SLASH as a hold before processing an interrupting key.
    if (record->event.pressed && home_slash.pressed && !home_slash.hold_sent && keycode != HOME_SLASH) {
        send_home_slash_hold();
    }

    // Resolve SP_MINS as a hold before processing an interrupting key.
    if (record->event.pressed && space_minus.pressed && !space_minus.hold_sent && keycode != SP_MINS) {
        send_space_minus_hold();
    }

    // Punctuation is immediate; Space is appended only after TAPPING_TERM.
    for (uint8_t i = 0; i < ARRAY_SIZE(punctuation_spaces); i++) {
        punctuation_space_t *punctuation = &punctuation_spaces[i];

        if (keycode == punctuation->custom_keycode) {
            if (record->event.pressed) {
                tap_code(punctuation->tap_keycode);
                punctuation->pending = true;
                punctuation->timer   = timer_read();
            } else {
                punctuation->pending = false;
            }
            return false;
        }
    }

    switch (keycode) {
        case PYTHON_SHEBANG:
            if (record->event.pressed) {
                SEND_STRING("#!/usr/bin/env python3\n");
            }
            return false;

        case BASH_SHEBANG:
            if (record->event.pressed) {
                SEND_STRING("#!/usr/bin/env bash\n");
            }
            return false;

        case HOME_SLASH:
            if (record->event.pressed) {
                home_slash.pressed   = true;
                home_slash.hold_sent = false;
                home_slash.timer     = timer_read();
            } else {
                if (!home_slash.hold_sent) {
                    tap_code(KC_PSLS);
                }
                home_slash.pressed = false;
            }
            return false;

        case SP_MINS:
            if (record->event.pressed) {
                space_minus.pressed   = true;
                space_minus.hold_sent = false;
                space_minus.timer     = timer_read();
            } else {
                if (!space_minus.hold_sent) {
                    tap_code(KC_MINS);
                }
                space_minus.pressed = false;
            }
            return false;

        case HOLD_MAKE:
            process_system_action(SYSTEM_ACTION_MAKE, record);
            return false;

        case HOLD_EE_CLEAR:
            process_system_action(SYSTEM_ACTION_EE_CLEAR, record);
            return false;

        case HOLD_SHOW_VERSION:
            process_system_action(SYSTEM_ACTION_SHOW_VERSION, record);
            return false;
    }

    return process_modified_space(keycode, record);
}

void matrix_scan_user(void) {
    for (uint8_t i = 0; i < ARRAY_SIZE(punctuation_spaces); i++) {
        punctuation_space_t *punctuation = &punctuation_spaces[i];

        if (punctuation->pending && timer_elapsed(punctuation->timer) >= TAPPING_TERM) {
            punctuation->pending = false;
            tap_code(KC_SPC);
        }
    }

    if (home_slash.pressed && !home_slash.hold_sent && timer_elapsed(home_slash.timer) >= TAPPING_TERM) {
        send_home_slash_hold();
    }

    if (space_minus.pressed && !space_minus.hold_sent && timer_elapsed(space_minus.timer) >= TAPPING_TERM) {
        send_space_minus_hold();
    }

    if (system_action_feedback == SYSTEM_ACTION_FEEDBACK_ARMING && timer_elapsed(system_action_timer) >= HOLD_ACTION_TERM) {
        if (system_action == SYSTEM_ACTION_MAKE) {
            system_action_mods = mod_config(get_mods());
        }
        set_system_action_feedback(SYSTEM_ACTION_FEEDBACK_CONFIRMED);
    } else if (system_action_feedback == SYSTEM_ACTION_FEEDBACK_CONFIRMED && timer_elapsed(system_action_timer) >= HOLD_ACTION_CONFIRMED_TERM) {
        perform_system_action();
    }
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

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (system_action_feedback != SYSTEM_ACTION_FEEDBACK_IDLE) {
        for (uint8_t i = led_min; i < led_max; i++) {
            rgb_matrix_set_color(i, 0, 0, 0);
        }

        if (system_action_feedback == SYSTEM_ACTION_FEEDBACK_CONFIRMED) {
            RGB_MATRIX_INDICATOR_SET_COLOR(NUMPAD_LAYER_LED, 128, 128, 0);
        } else if (timer_elapsed(system_action_timer) % (HOLD_ACTION_BLINK_TERM * 2) < HOLD_ACTION_BLINK_TERM) {
            RGB_MATRIX_INDICATOR_SET_COLOR(NUMPAD_LAYER_LED, 128, 0, 0);
        }
        return false;
    }

    led_t host_leds = host_keyboard_led_state();

    set_lock_indicator(CAPS_LOCK_LED, host_leds.caps_lock, led_min, led_max);
    set_lock_indicator(NUM_LOCK_LED, host_leds.num_lock, led_min, led_max);
    set_lock_indicator(SCROLL_LOCK_LED, host_leds.scroll_lock, led_min, led_max);

    // layer state
    switch (get_highest_layer(layer_state)) {
        case MOUSE:
            RGB_MATRIX_INDICATOR_SET_COLOR(MOUSE_LAYER_LED, 0, 0, 128);
            break;
        case NUMPAD:
            RGB_MATRIX_INDICATOR_SET_COLOR(NUMPAD_LAYER_LED, 128, 40, 0);
            break;
        case SYMBOLS:
            RGB_MATRIX_INDICATOR_SET_COLOR(SYMBOLS_LAYER_LED, 0, 128, 0);
            break;
    }
    return false;
}
