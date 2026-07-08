# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_muxctl.c" "src/ss_muxctl_core.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal
    PRIV_REQUIRES driver
)
