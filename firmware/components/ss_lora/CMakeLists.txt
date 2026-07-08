# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_lora_lbt_core.c" "src/ss_lora_pa_core.c" "src/ss_lora.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal
)
