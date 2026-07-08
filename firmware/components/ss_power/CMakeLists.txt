# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_power.c" "src/ss_power_core.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal
    PRIV_REQUIRES esp_hw_support esp_system esp_timer driver
)
