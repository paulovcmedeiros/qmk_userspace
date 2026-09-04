#pragma once

#define BOTH_SHIFTS_TURNS_ON_CAPS_WORD
#define ENABLE_COMPILE_KEYCODE

/* Disable some unused settings */
#define NO_ACTION_ONESHOT
#define LAYER_STATE_8BIT
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

/* Kinetic Mouse Keys: precise initial movement with a gradual quadratic ramp. */
#define MK_KINETIC_SPEED
#define MOUSEKEY_INTERVAL 8
#define MOUSEKEY_INITIAL_SPEED 150
#define MOUSEKEY_BASE_SPEED 1000
#define MOUSEKEY_MOVE_DELTA 10

/* Tap-hold behavior. */
#define TAPPING_TERM 150

/* Require a deliberate, brief chord for the inner-thumb Enter combo. */
#define COMBO_TERM 40
#define COMBO_MUST_TAP_PER_COMBO
#define COMBO_SHOULD_TRIGGER

/* Allow the primary half to request the secondary half's build date. */
#define SPLIT_TRANSACTION_IDS_USER BUILD_DATE_SYNC

/* Keep Solid Color and Alphas/Mods; omit unused RGB animations. */
#undef ENABLE_RGB_MATRIX_BREATHING
#undef ENABLE_RGB_MATRIX_CYCLE_ALL
#undef ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
#undef ENABLE_RGB_MATRIX_CYCLE_SPIRAL
#undef ENABLE_RGB_MATRIX_DUAL_BEACON
#undef ENABLE_RGB_MATRIX_RAINBOW_BEACON
#undef ENABLE_RGB_MATRIX_RAINDROPS
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE
#undef ENABLE_RGB_MATRIX_SPLASH
#undef ENABLE_RGB_MATRIX_SOLID_SPLASH
