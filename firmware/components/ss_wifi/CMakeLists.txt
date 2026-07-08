# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_wifi_sta_core.c" "src/ss_wifi.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal esp_wifi esp_event esp_netif nvs_flash
)
