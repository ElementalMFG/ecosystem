# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_gnss.c" "src/ss_gnss_core.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal
    PRIV_REQUIRES driver esp_hw_support esp_system freertos
)
