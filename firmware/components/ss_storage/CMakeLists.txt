# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors

idf_component_register(
    SRCS "src/ss_storage_fs_core.c" "src/ss_storage.c"
    INCLUDE_DIRS "include"
    REQUIRES ss_hal esp_partition
    PRIV_REQUIRES littlefs
)
