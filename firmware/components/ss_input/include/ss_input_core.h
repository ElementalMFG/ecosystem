// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_input_core.h — pure, host-testable decision cores for the ss_input
// component (S-03-004). Like ss_power_core, this layer holds NO ESP-IDF
// runtime dependency: it reasons only about the frozen contract types
// (ss_hal_types.h) so it can be exercised with host gtest. The IDF glue
// (ss_touch.c / ss_buttons.c) feeds these cores raw samples and forwards the
// derived ss_input_event_t values to a registered callback.
//
// Two independent FSMs live here:
//   * a touch gesture recogniser (raw down/move/up  ->  TAP / LONG_PRESS /
//     SWIPE_{L,R,U,D});
//   * a button debouncer (raw pressed-bool + now_ms  ->  BUTTON_DOWN /
//     LONG_PRESS / BUTTON_UP).

#pragma once

#include "ss_hal_types.h"

// ===========================================================================
// Gesture recogniser
// ===========================================================================

// Named, documented thresholds. Pixels are in the panel's native coordinate
// space (GT911 reports 0..479 x 0..319 on the Lite 3.5" panel).

// A touch that goes down and up again within this many milliseconds, without
// exceeding the tap slop, is reported as SS_INPUT_TAP.
#define SS_GESTURE_TAP_MAX_MS 300u

// A touch held (without exceeding the tap slop) for at least this many
// milliseconds is reported as SS_INPUT_LONG_PRESS. Emitted as soon as the hold
// time is observed (on a move sample or on release), then latched so a given
// press yields at most one long-press.
#define SS_GESTURE_LONGPRESS_MS 500u

// Maximum Chebyshev displacement (max(|dx|,|dy|)) from the down point that
// still counts as "stationary": a jitter within this radius does not cancel a
// tap or long-press. Exceeding it marks the gesture as moved.
#define SS_GESTURE_TAP_SLOP_PX 12

// Minimum net displacement along the dominant axis for a release to be
// classified as a swipe. The dominant axis (larger absolute displacement)
// selects the direction: +x -> SWIPE_R, -x -> SWIPE_L, +y -> SWIPE_D
// (y grows downward), -y -> SWIPE_U.
#define SS_GESTURE_SWIPE_MIN_PX 40

// Recogniser state. Zero-initialise (or call ss_gesture_init) before first use.
typedef struct {
    bool         active;            // a touch is currently down
    bool         moved;             // exceeded SS_GESTURE_TAP_SLOP_PX since down
    bool         longpress_emitted; // long-press already reported for this press
    int16_t      start_x, start_y;  // coordinates at the down sample
    int16_t      last_x, last_y;    // most recent coordinates
    ss_mono_us_t start_us;          // timestamp of the down sample
} ss_gesture_t;

// Raw touch sample phase fed to the recogniser.
typedef enum {
    SS_TOUCH_SAMPLE_DOWN, // finger first detected
    SS_TOUCH_SAMPLE_MOVE, // finger still down, new position
    SS_TOUCH_SAMPLE_UP,   // finger released
} ss_touch_sample_t;

// Reset a recogniser to the idle state.
// Pre:  g non-NULL.
// Post: *g is fully zeroed (active == false).
void ss_gesture_init(ss_gesture_t* g);

// Feed one raw touch sample.
// Pre:  g and out non-NULL. `at` is a monotonic microsecond timestamp that is
//       non-decreasing across the samples of one gesture.
// Post: DOWN always (re)starts tracking and returns false (the raw
//       SS_INPUT_TOUCH_DOWN is emitted by the glue, not here). MOVE updates the
//       tracked position and returns true with SS_INPUT_LONG_PRESS the first
//       time a stationary hold reaches SS_GESTURE_LONGPRESS_MS. UP classifies
//       the completed gesture and returns true with exactly one of:
//         - SS_INPUT_SWIPE_{L,R,U,D} when the dominant-axis net displacement is
//           >= SS_GESTURE_SWIPE_MIN_PX (delta = that displacement's magnitude);
//         - SS_INPUT_TAP when it stayed within slop and lasted < TAP_MAX_MS;
//         - SS_INPUT_LONG_PRESS when it stayed within slop, lasted >=
//           LONGPRESS_MS, and no long-press was already emitted;
//       otherwise (ambiguous release) returns false. Returns false whenever no
//       derived event is produced; *out is only meaningful when true.
bool ss_gesture_step(ss_gesture_t* g, ss_touch_sample_t phase, int16_t x, int16_t y,
                     ss_mono_us_t at, ss_input_event_t* out);

// ===========================================================================
// Button debouncer
// ===========================================================================

// A raw level must persist unchanged for at least this long before the
// debouncer commits the new stable state (bounce shorter than this is ignored).
#define SS_BTN_DEBOUNCE_MS 20u

// A committed press held continuously for at least this long emits one
// SS_INPUT_LONG_PRESS (latched until release).
#define SS_BTN_LONGPRESS_MS 800u

// Debouncer state. Zero-initialise (or call ss_debounce_init) before first use.
typedef struct {
    bool     stable_pressed;    // last committed (debounced) level
    bool     cand;              // raw level currently being timed
    bool     longpress_emitted; // long-press already reported for this press
    bool     have_cand;         // a candidate change is being timed
    uint32_t cand_since_ms;     // timestamp the candidate was first seen
    uint32_t press_since_ms;    // timestamp the current press committed
} ss_debounce_t;

// Reset a debouncer to the released, idle state.
// Pre:  d non-NULL.
// Post: *d is fully zeroed (stable_pressed == false).
void ss_debounce_init(ss_debounce_t* d);

// Feed one raw pressed-bool sample with a monotonic millisecond timestamp.
// Pre:  d and out non-NULL; now_ms non-decreasing across calls.
// Post: returns true with SS_INPUT_BUTTON_DOWN when a new level of `true`
//       stays stable for SS_BTN_DEBOUNCE_MS; SS_INPUT_BUTTON_UP when a new
//       `false` stays stable that long; SS_INPUT_LONG_PRESS once when a
//       committed press has been held SS_BTN_LONGPRESS_MS. Bounce shorter than
//       the debounce interval yields no event (returns false). At most one
//       event per call; *out is only meaningful when true.
bool ss_debounce_step(ss_debounce_t* d, bool pressed, uint32_t now_ms, ss_input_event_t* out);
