# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_mic.c" "src/ss_audio_core.c" "src/ss_spk.c" "src/ss_spk_core.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal ss_muxctl
    PRIV_REQUIRES driver esp_driver_i2s
)
