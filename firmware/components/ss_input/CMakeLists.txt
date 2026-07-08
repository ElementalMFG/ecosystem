# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_input_core.c" "src/ss_touch.c" "src/ss_buttons.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal
    PRIV_REQUIRES driver esp_timer esp_hw_support esp_system
)
