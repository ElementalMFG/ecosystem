# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# SS-SP universal firmware — ESP-IDF v5.3+ project.
#
# Board selection (S-02-002): the HAL is board-agnostic; a board port supplies
# firmware/boards/<name>/board_config.h. Select with either:
#   BOARD=lite idf.py build            (environment variable)
#   idf.py -DSS_BOARD=lite build       (cache variable; wins over env)
# Default: lite. Or from the repo root: `make lite|alpha|omega`.
#
cmake_minimum_required(VERSION 3.16)

# Precedence: -DSS_BOARD cache var > BOARD env var > "lite" default.
# (Guarded so set(CACHE) can never clobber the env-derived value; each board
#  builds in its own build/<board> dir, so the cached choice never leaks.)
if(NOT DEFINED CACHE{SS_BOARD})
    if(DEFINED ENV{BOARD})
        set(SS_BOARD "$ENV{BOARD}" CACHE STRING "Board port under firmware/boards/<name>/")
    else()
        set(SS_BOARD "lite" CACHE STRING "Board port under firmware/boards/<name>/")
    endif()
endif()

# Validate the board port exists — fail early with a clear error.
set(SS_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}/boards/${SS_BOARD}")
if(NOT EXISTS "${SS_BOARD_DIR}/board_config.h")
    file(GLOB _ss_board_dirs RELATIVE "${CMAKE_CURRENT_LIST_DIR}/boards" "${CMAKE_CURRENT_LIST_DIR}/boards/*")
    list(JOIN _ss_board_dirs ", " _ss_boards_pretty)
    message(FATAL_ERROR
        "SS-SP: unknown board '${SS_BOARD}' — no boards/${SS_BOARD}/board_config.h.\n"
        "Available board ports: ${_ss_boards_pretty}.\n"
        "Select with BOARD=<name> idf.py build, idf.py -DSS_BOARD=<name> build, or make <name>.")
endif()

message(STATUS "SS-SP: building for board port '${SS_BOARD}'")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ss_sp_firmware)
