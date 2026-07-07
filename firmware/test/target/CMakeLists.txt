# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# On-target Unity test app (S-02-015). A standalone ESP-IDF project that boots
# on a real board, runs the registered Unity TEST_CASEs, and prints the standard
# Unity summary over the console serial for pytest-embedded to parse in CI.
#
# It is a SEPARATE project from the firmware image (firmware/CMakeLists.txt) on
# purpose: test cases (and the panic-by-design hang tests written by S-02-006 /
# S-02-009) must never be linked into a shipping binary. Board selection mirrors
# the main firmware — the HAL is board-agnostic and boards/<name>/board_config.h
# supplies the port:
#   idf.py -DSS_BOARD=lite build        (cache var; wins over env)
#   BOARD=lite idf.py build             (environment variable)
# Default: lite (ESP32-S3); the AC targets the Lite board.
#
cmake_minimum_required(VERSION 3.16)

# Precedence: -DSS_BOARD cache var > BOARD env var > "lite" default. Guarded so
# set(CACHE) can never clobber the env-derived value (same idiom as the root).
if(NOT DEFINED CACHE{SS_BOARD})
    if(DEFINED ENV{BOARD})
        set(SS_BOARD "$ENV{BOARD}" CACHE STRING "Board port under firmware/boards/<name>/")
    else()
        set(SS_BOARD "lite" CACHE STRING "Board port under firmware/boards/<name>/")
    endif()
endif()

# Validate the board port exists — fail early with a clear error.
set(SS_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}/../../boards/${SS_BOARD}")
if(NOT EXISTS "${SS_BOARD_DIR}/board_config.h")
    message(FATAL_ERROR
        "SS-SP target-tests: unknown board '${SS_BOARD}' — no boards/${SS_BOARD}/board_config.h.\n"
        "Select with BOARD=<name> idf.py build or idf.py -DSS_BOARD=<name> build.")
endif()

# Expose only the known-good repo components the tests need (each entry is a
# single component dir with its own CMakeLists — this deliberately avoids the
# empty epic-gated scaffolds under components/ that have no CMakeLists yet).
# ss_hal exports the selected board's board_config.h on the include path.
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../../components/ss_hal"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/ss_log"
)

message(STATUS "SS-SP target-tests: building for board port '${SS_BOARD}'")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ss_target_tests)
