# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# Host-side gtest baseline for pure-C firmware code (S-02-014).
#
# Pure, ESP-IDF-free cores (e.g. the ss_log redaction formatter) are compiled
# straight into a small `ss_units` static library and exercised with googletest
# on a plain host toolchain — no firmware container required. HAL-header
# consumers are covered via a host mock of board_config.h (see mocks/).
#
# The pre-existing Makefile harness under
# firmware/components/ss_log/test/host/ stays as the quick ASan/UBSan dev loop;
# this project adds the framework + coverage path that CI publishes.

cmake_minimum_required(VERSION 3.16)
project(ss_host_tests CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Coverage is ON by default (CI publishes coverage.xml); developers can turn it
# off for a faster local loop with -DSS_COVERAGE=OFF. It instruments ONLY the
# tested firmware sources (ss_units), never googletest itself.
option(SS_COVERAGE "Instrument tested firmware sources for gcov coverage" ON)

# --- googletest, digest-pinned in the spirit of RFC-0002 ---------------------
# Pinned by FULL commit SHA. The tag is recorded for humans; the SHA is what git
# resolves (identical to the Dockerfile "tag for humans, digest for machines"
# comment style). Bump procedure: update BOTH the tag comment and the SHA below.
#   tag: v1.15.2
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        b514bdc898e2951020cbdca1304b75f5950d1f59 # v1.15.2
)
# Keep googletest's own CRT settings self-consistent on all platforms.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- tested firmware sources -> instrumented static lib ----------------------
set(SS_COMPONENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../components)
set(SS_MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../main)

add_library(ss_units STATIC
    ${SS_COMPONENTS_DIR}/ss_log/src/ss_log_format.c
    ${SS_MAIN_DIR}/ss_panic_guard_core.c
)
target_include_directories(ss_units PUBLIC
    ${SS_COMPONENTS_DIR}/ss_log/include
    ${SS_MAIN_DIR}
)
if(SS_COVERAGE)
    # Compile-time instrumentation on the tested code only; PUBLIC link option so
    # the test executable pulls in the gcov runtime when it links ss_units.
    target_compile_options(ss_units PRIVATE --coverage -O0 -g)
    target_link_options(ss_units PUBLIC --coverage)
endif()

# --- test executable ---------------------------------------------------------
enable_testing()
include(GoogleTest)

add_executable(ss_host_tests
    tests/test_ss_log_format.cpp
    tests/test_hal_mock.cpp
    tests/test_ss_panic_guard.cpp
)
target_include_directories(ss_host_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks   # host board_config.h mock (first on path)
    ${SS_COMPONENTS_DIR}/ss_hal/include # real, host-clean ss_hal_caps.h/types.h
)
target_link_libraries(ss_host_tests PRIVATE ss_units GTest::gtest_main)

gtest_discover_tests(ss_host_tests)
