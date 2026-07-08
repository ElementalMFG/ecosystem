# SPDX-License-Identifier: Apache-2.0
# SS-SP firmware entry component — boot scaffolding (display + dual-UART first).

idf_component_register(
    SRCS
        "main.cpp"
        "ss_uart_engine.cpp"
        "ss_compass.cpp"
        "ss_diag.cpp"
        "ss_bootmark.cpp"
        "ss_tasks.cpp"
        "ss_panic_guard.cpp"
        "ss_panic_guard_core.c"
        "ss_recovery.cpp"
        "ss_recovery_core.c"
        "ss_memwatch.cpp"
        "ss_memwatch_core.c"
        "ss_nvs.cpp"
        "ss_nvs_core.c"
        "ss_version.cpp"
    INCLUDE_DIRS "."
    REQUIRES ss_hal ss_log ss_power ss_display driver esp_timer nvs_flash espcoredump esp_partition
             app_update esp_system
)

# Capture git provenance (SHA + tag + build id) into the main component.
# MUST follow idf_component_register (needs ${COMPONENT_LIB}).
include("${CMAKE_CURRENT_LIST_DIR}/version.cmake")
