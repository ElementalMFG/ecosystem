# SPDX-License-Identifier: Apache-2.0
# SS-SP firmware entry component — boot scaffolding (display + dual-UART first).

idf_component_register(
    SRCS
        "main.cpp"
        "ss_display_boot.cpp"
        "ss_uart_engine.cpp"
        "ss_compass.cpp"
        "ss_diag.cpp"
    INCLUDE_DIRS "."
    REQUIRES hal driver esp_lcd esp_timer nvs_flash
)
