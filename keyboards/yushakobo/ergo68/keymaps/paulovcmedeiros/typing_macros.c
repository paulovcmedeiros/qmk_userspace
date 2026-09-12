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

#define BRACKET_PAIR_TERM 300
#define BRACKET_PAIR_SHORTCUT_MODS (MOD_MASK_CTRL | MOD_MASK_ALT | MOD_MASK_GUI)

static matrix_row_t intercepted_space_taps[MATRIX_ROWS];

typedef struct {
    bool     pressed;
    bool     hold_sent;
    uint16_t timer;
} tap_hold_state_t;

// Tap sends keypad slash; hold or interruption sends "./".
static tap_hold_state_t dot_slash;

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
    uint16_t expected_closer;
    uint16_t pending_closer;
    uint16_t timer;
    keypos_t closing_key;
    bool     cursor_left_pending;
    bool     cursor_left_ready;
} bracket_pair_state_t;

static bracket_pair_state_t bracket_pair;

static bool key_positions_match(keypos_t first, keypos_t second) {
    return first.row == second.row && first.col == second.col;
}

/** Resolve pending Auto Shift input before consuming a custom keypress. */
static void resolve_pending_auto_shift(uint16_t keycode, keyrecord_t *record) {
    process_auto_shift(keycode, record);
}

/** Return the closer paired with a Symbols-layer opener. */
static uint16_t matching_bracket_closer(uint16_t keycode) {
    switch (keycode) {
        case S(KC_LBRC):
            return S(KC_RBRC);
        case KC_LBRC:
            return KC_RBRC;
        case S(KC_9):
            return S(KC_0);
        default:
            return KC_NO;
    }
}

/** Track a matching closer pressed immediately after its opener. */
static void process_bracket_pair(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        bool matching_release = keycode == bracket_pair.pending_closer && key_positions_match(record->event.key, bracket_pair.closing_key);

        if (bracket_pair.cursor_left_pending && matching_release) {
            bracket_pair.cursor_left_pending = false;
            bracket_pair.cursor_left_ready   = true;
        }
        return;
    }

    // A key pressed before the pending closer is released cancels the move.
    bracket_pair.cursor_left_pending = false;
    bracket_pair.cursor_left_ready   = false;

    if ((get_mods() | get_weak_mods()) & BRACKET_PAIR_SHORTCUT_MODS) {
        bracket_pair.expected_closer = KC_NO;
        return;
    }

    uint16_t closer = matching_bracket_closer(keycode);
    if (closer != KC_NO) {
        bracket_pair.expected_closer = closer;
        bracket_pair.timer           = timer_read();
        return;
    }

    if (keycode == bracket_pair.expected_closer && timer_elapsed(bracket_pair.timer) <= BRACKET_PAIR_TERM) {
        bracket_pair.pending_closer      = keycode;
        bracket_pair.closing_key         = record->event.key;
        bracket_pair.cursor_left_pending = true;
    }

    bracket_pair.expected_closer = KC_NO;
}

/** Tap Left without applying modifiers that remain physically held. */
static void tap_unmodified_left(void) {
    uint8_t saved_mods      = get_mods();
    uint8_t saved_weak_mods = get_weak_mods();

    clear_mods();
    clear_weak_mods();
    send_keyboard_report();
    tap_code(KC_LEFT);
    set_mods(saved_mods);
    set_weak_mods(saved_weak_mods);
    send_keyboard_report();
}

/** Send the hold action for DOT_SLASH and mark it as resolved. */
static void send_dot_slash_hold(void) {
    SEND_STRING("./");
    dot_slash.hold_sent = true;
}

/** Send the hold action for SP_MINS and mark it as resolved. */
static void send_space_minus_hold(void) {
    SEND_STRING(" -");
    space_minus.hold_sent = true;
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
bool process_modified_space(uint16_t keycode, keyrecord_t *record) {
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

bool process_typing_macros(uint16_t keycode, keyrecord_t *record) {
    process_bracket_pair(keycode, record);

    // Resolve DOT_SLASH as a hold before processing an interrupting key.
    if (record->event.pressed && dot_slash.pressed && !dot_slash.hold_sent && keycode != DOT_SLASH) {
        send_dot_slash_hold();
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
                resolve_pending_auto_shift(keycode, record);
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
                resolve_pending_auto_shift(keycode, record);
                SEND_STRING("#!/usr/bin/env python3\n");
            }
            return false;

        case BASH_SHEBANG:
            if (record->event.pressed) {
                resolve_pending_auto_shift(keycode, record);
                SEND_STRING("#!/usr/bin/env bash\n");
            }
            return false;

        case DOT_SLASH:
            if (record->event.pressed) {
                resolve_pending_auto_shift(keycode, record);
                dot_slash.pressed   = true;
                dot_slash.hold_sent = false;
                dot_slash.timer     = timer_read();
            } else {
                if (!dot_slash.hold_sent) {
                    tap_code(KC_PSLS);
                }
                dot_slash.pressed = false;
            }
            return false;

        case HOME_SLASH:
            if (record->event.pressed) {
                resolve_pending_auto_shift(keycode, record);
                SEND_STRING("~/");
            }
            return false;

        case SP_MINS:
            if (record->event.pressed) {
                resolve_pending_auto_shift(keycode, record);
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
    }

    return true;
}

void typing_macros_task(void) {
    if (bracket_pair.cursor_left_ready) {
        bracket_pair.cursor_left_ready = false;
        tap_unmodified_left();
    }

    for (uint8_t i = 0; i < ARRAY_SIZE(punctuation_spaces); i++) {
        punctuation_space_t *punctuation = &punctuation_spaces[i];

        if (punctuation->pending && timer_elapsed(punctuation->timer) >= TAPPING_TERM) {
            punctuation->pending = false;
            tap_code(KC_SPC);
        }
    }

    if (dot_slash.pressed && !dot_slash.hold_sent && timer_elapsed(dot_slash.timer) >= TAPPING_TERM) {
        send_dot_slash_hold();
    }

    if (space_minus.pressed && !space_minus.hold_sent && timer_elapsed(space_minus.timer) >= TAPPING_TERM) {
        send_space_minus_hold();
    }
}
