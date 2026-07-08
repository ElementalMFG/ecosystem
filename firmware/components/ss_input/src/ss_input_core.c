// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_input_core.c — pure decision logic for the ss_input HAL glue. No ESP-IDF
// runtime calls: this file must stay host-linkable for gtest.

#include "ss_input_core.h"

#include <string.h>

// Absolute value of an int without pulling in <stdlib.h> semantics that vary
// by width; the arguments here are bounded pixel deltas.
static int iabs(int v)
{
    return v < 0 ? -v : v;
}

// Clamp a pixel magnitude into the int16_t `delta` field of ss_input_event_t.
static int16_t clamp_i16(int v)
{
    if (v > 32767) { return 32767; }
    if (v < -32768) { return -32768; }
    return (int16_t)v;
}

static uint64_t ms_since(ss_mono_us_t from, ss_mono_us_t to)
{
    if (to <= from) { return 0; }
    return (to - from) / 1000u;
}

// ===========================================================================
// Gesture recogniser
// ===========================================================================

void ss_gesture_init(ss_gesture_t* g)
{
    if (g == NULL) { return; }
    memset(g, 0, sizeof(*g));
}

// Mark `moved` if the current point has left the tap-slop radius of the down
// point (Chebyshev distance).
static void update_moved(ss_gesture_t* g, int16_t x, int16_t y)
{
    const int dx = iabs((int)x - (int)g->start_x);
    const int dy = iabs((int)y - (int)g->start_y);
    if (dx > SS_GESTURE_TAP_SLOP_PX || dy > SS_GESTURE_TAP_SLOP_PX) { g->moved = true; }
}

static void fill_event(ss_input_event_t* out, ss_input_kind_t kind, int16_t x, int16_t y,
                       ss_mono_us_t at, int16_t delta)
{
    memset(out, 0, sizeof(*out));
    out->kind = kind;
    out->at = at;
    out->x = x;
    out->y = y;
    out->delta = delta;
}

bool ss_gesture_step(ss_gesture_t* g, ss_touch_sample_t phase, int16_t x, int16_t y,
                     ss_mono_us_t at, ss_input_event_t* out)
{
    if (g == NULL || out == NULL) { return false; }

    switch (phase) {
    case SS_TOUCH_SAMPLE_DOWN:
        g->active = true;
        g->moved = false;
        g->longpress_emitted = false;
        g->start_x = x;
        g->start_y = y;
        g->last_x = x;
        g->last_y = y;
        g->start_us = at;
        return false; // the raw TOUCH_DOWN is emitted by the glue

    case SS_TOUCH_SAMPLE_MOVE:
        if (!g->active) { return false; }
        g->last_x = x;
        g->last_y = y;
        update_moved(g, x, y);
        if (!g->moved && !g->longpress_emitted &&
            ms_since(g->start_us, at) >= SS_GESTURE_LONGPRESS_MS) {
            g->longpress_emitted = true;
            fill_event(out, SS_INPUT_LONG_PRESS, x, y, at, 0);
            return true;
        }
        return false;

    case SS_TOUCH_SAMPLE_UP: {
        if (!g->active) { return false; }
        g->active = false;
        g->last_x = x;
        g->last_y = y;
        update_moved(g, x, y);

        const int dx = (int)x - (int)g->start_x;
        const int dy = (int)y - (int)g->start_y;
        const int adx = iabs(dx);
        const int ady = iabs(dy);

        // Swipe: dominant-axis net displacement clears the minimum.
        if (adx >= SS_GESTURE_SWIPE_MIN_PX || ady >= SS_GESTURE_SWIPE_MIN_PX) {
            if (adx >= ady) {
                fill_event(out, dx > 0 ? SS_INPUT_SWIPE_R : SS_INPUT_SWIPE_L, x, y, at,
                           clamp_i16(adx));
            } else {
                fill_event(out, dy > 0 ? SS_INPUT_SWIPE_D : SS_INPUT_SWIPE_U, x, y, at,
                           clamp_i16(ady));
            }
            return true;
        }

        if (g->moved) { return false; } // moved but not far enough for a swipe

        const uint64_t held = ms_since(g->start_us, at);
        if (held < SS_GESTURE_TAP_MAX_MS) {
            fill_event(out, SS_INPUT_TAP, x, y, at, 0);
            return true;
        }
        if (held >= SS_GESTURE_LONGPRESS_MS && !g->longpress_emitted) {
            g->longpress_emitted = true;
            fill_event(out, SS_INPUT_LONG_PRESS, x, y, at, 0);
            return true;
        }
        return false; // ambiguous (e.g. held between TAP_MAX and LONGPRESS)
    }
    }
    return false;
}

// ===========================================================================
// Button debouncer
// ===========================================================================

void ss_debounce_init(ss_debounce_t* d)
{
    if (d == NULL) { return; }
    memset(d, 0, sizeof(*d));
}

bool ss_debounce_step(ss_debounce_t* d, bool pressed, uint32_t now_ms, ss_input_event_t* out)
{
    if (d == NULL || out == NULL) { return false; }

    if (pressed == d->stable_pressed) {
        // Level matches the committed state: cancel any pending candidate and
        // check for a long-press on a held, committed press.
        d->have_cand = false;
        if (d->stable_pressed && !d->longpress_emitted &&
            (now_ms - d->press_since_ms) >= SS_BTN_LONGPRESS_MS) {
            d->longpress_emitted = true;
            memset(out, 0, sizeof(*out));
            out->kind = SS_INPUT_LONG_PRESS;
            out->at = (ss_mono_us_t)now_ms * 1000u;
            return true;
        }
        return false;
    }

    // Level differs from the committed state: time it.
    if (!d->have_cand || pressed != d->cand) {
        d->have_cand = true;
        d->cand = pressed;
        d->cand_since_ms = now_ms;
        return false;
    }
    if ((now_ms - d->cand_since_ms) < SS_BTN_DEBOUNCE_MS) { return false; }

    // Candidate has been stable long enough: commit it.
    d->stable_pressed = pressed;
    d->have_cand = false;
    memset(out, 0, sizeof(*out));
    out->at = (ss_mono_us_t)now_ms * 1000u;
    if (pressed) {
        d->press_since_ms = now_ms;
        d->longpress_emitted = false;
        out->kind = SS_INPUT_BUTTON_DOWN;
    } else {
        out->kind = SS_INPUT_BUTTON_UP;
    }
    return true;
}
