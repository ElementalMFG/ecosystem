# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_display_core.c" "src/ss_display.cpp"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal
    PRIV_REQUIRES driver esp_lcd esp_timer freertos
)
