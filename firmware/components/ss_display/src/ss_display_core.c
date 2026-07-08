// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_display_core.c — pure geometry/timing/format logic for the ss_display
// HAL driver. No ESP-IDF runtime calls and no board_config reads: this file
// must stay host-linkable for gtest.

#include "ss_display_core.h"

// ILI9488 MADCTL bit field (datasheet §5 "Memory Data Access Control").
#define ILI_MADCTL_MY 0x80u  // row address order
#define ILI_MADCTL_MX 0x40u  // column address order
#define ILI_MADCTL_MV 0x20u  // row/column exchange (landscape when set)
#define ILI_MADCTL_BGR 0x08u // BGR panel colour order

bool ss_display_core_clip(ss_rect_t in, uint16_t w_px, uint16_t h_px, ss_rect_t* out)
{
    if (out == NULL) { return false; }

    int32_t x1 = in.x;
    int32_t y1 = in.y;
    int32_t x2 = (int32_t)in.x + in.w; // exclusive right edge
    int32_t y2 = (int32_t)in.y + in.h; // exclusive bottom edge

    if (x1 < 0) { x1 = 0; }
    if (y1 < 0) { y1 = 0; }
    if (x2 > (int32_t)w_px) { x2 = (int32_t)w_px; }
    if (y2 > (int32_t)h_px) { y2 = (int32_t)h_px; }

    if (x2 <= x1 || y2 <= y1) {
        out->x = 0;
        out->y = 0;
        out->w = 0;
        out->h = 0;
        return false;
    }

    out->x = (int16_t)x1;
    out->y = (int16_t)y1;
    out->w = (int16_t)(x2 - x1);
    out->h = (int16_t)(y2 - y1);
    return true;
}

void ss_display_core_px565_to_666(uint16_t c, uint8_t out[3])
{
    // Match the verified bring-up shifts exactly (ss_display_boot px565_to_666):
    // R5 -> high 5 bits, G6 -> high 6 bits, B5 -> high 5 bits of each wire byte.
    out[0] = (uint8_t)((c >> 8) & 0xF8u); // R
    out[1] = (uint8_t)((c >> 3) & 0xFCu); // G
    out[2] = (uint8_t)((c << 3) & 0xF8u); // B
}

uint32_t ss_display_core_tile_xfer_us(uint16_t w, uint16_t h, uint8_t wire_bpp, uint32_t freq_hz)
{
    if (freq_hz == 0u) { return UINT32_MAX; }
    const uint64_t bits = (uint64_t)w * (uint64_t)h * (uint64_t)wire_bpp * 8u;
    const uint64_t num = bits * 1000000u; // bit-seconds -> bit-microseconds
    const uint64_t us = (num + (uint64_t)freq_hz - 1u) / (uint64_t)freq_hz; // ceil
    return (us > UINT32_MAX) ? UINT32_MAX : (uint32_t)us;
}

uint8_t ss_display_core_madctl(ss_orient_t o)
{
    switch (o) {
    case SS_ORIENT_0:                                     // portrait
        return (uint8_t)(ILI_MADCTL_MX | ILI_MADCTL_BGR); // 0x48
    case SS_ORIENT_90:                                    // native landscape
        return (uint8_t)(ILI_MADCTL_MV | ILI_MADCTL_MX | ILI_MADCTL_MY | ILI_MADCTL_BGR); // 0xE8
    case SS_ORIENT_180: // portrait, 180-flipped (both MX/MY toggled vs 0)
        return (uint8_t)(ILI_MADCTL_MY | ILI_MADCTL_BGR); // 0x88
    case SS_ORIENT_270: // landscape, 180-flipped (both MX/MY toggled vs 90)
        return (uint8_t)(ILI_MADCTL_MV | ILI_MADCTL_BGR); // 0x28
    default:
        return (uint8_t)(ILI_MADCTL_MV | ILI_MADCTL_MX | ILI_MADCTL_MY | ILI_MADCTL_BGR);
    }
}
