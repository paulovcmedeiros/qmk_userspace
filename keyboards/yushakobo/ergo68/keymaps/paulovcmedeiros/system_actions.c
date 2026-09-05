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

#include "process_quantum.h"
#include "transactions.h"
#include "version.h"

#define HOLD_ACTION_TERM 2000
#define HOLD_ACTION_CONFIRMED_TERM 250
#define HOLD_ACTION_BLINK_TERM 250
#define HOLD_ACTION_SYNC_RETRY_TERM 50

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

static system_action_t          system_action;
static system_action_feedback_t system_action_feedback;
static uint16_t                 system_action_timer;
static uint8_t                  system_action_mods;
static bool                     system_action_feedback_sync_pending = true;
static uint16_t                 system_action_feedback_sync_timer;
static bool                     show_version_pending;

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

void system_actions_init(void) {
    transaction_register_rpc(BUILD_DATE_SYNC, build_date_sync_handler);
    transaction_register_rpc(SYSTEM_ACTION_FEEDBACK_SYNC, system_action_feedback_sync_handler);
}

void system_actions_housekeeping_task(void) {
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

bool process_system_actions(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
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

    return true;
}

void system_actions_task(void) {
    if (system_action_feedback == SYSTEM_ACTION_FEEDBACK_ARMING && timer_elapsed(system_action_timer) >= HOLD_ACTION_TERM) {
        if (system_action == SYSTEM_ACTION_MAKE) {
            system_action_mods = mod_config(get_mods());
        }
        set_system_action_feedback(SYSTEM_ACTION_FEEDBACK_CONFIRMED);
    } else if (system_action_feedback == SYSTEM_ACTION_FEEDBACK_CONFIRMED && timer_elapsed(system_action_timer) >= HOLD_ACTION_CONFIRMED_TERM) {
        perform_system_action();
    }
}

bool system_actions_indicators(uint8_t indicator_led, uint8_t led_min, uint8_t led_max) {
    if (system_action_feedback != SYSTEM_ACTION_FEEDBACK_IDLE) {
        for (uint8_t i = led_min; i < led_max; i++) {
            rgb_matrix_set_color(i, 0, 0, 0);
        }

        if (system_action_feedback == SYSTEM_ACTION_FEEDBACK_CONFIRMED) {
            RGB_MATRIX_INDICATOR_SET_COLOR(indicator_led, 128, 128, 0);
        } else if (timer_elapsed(system_action_timer) % (HOLD_ACTION_BLINK_TERM * 2) < HOLD_ACTION_BLINK_TERM) {
            RGB_MATRIX_INDICATOR_SET_COLOR(indicator_led, 128, 0, 0);
        }
        return true;
    }

    return false;
}
