// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SS_STORAGE_INTERNAL_NVS,
    SS_STORAGE_INTERNAL_FS,     // encrypted user FS
    SS_STORAGE_SD,              // microSD (Lite: soft-SPI on GPIO 6/4/5/7)
    SS_STORAGE_MODELS,          // separate models partition
} ss_storage_kind_t;

esp_err_t ss_storage_init(void);
esp_err_t ss_storage_mount(ss_storage_kind_t kind);
esp_err_t ss_storage_unmount(ss_storage_kind_t kind);
esp_err_t ss_storage_stat(ss_storage_kind_t kind,
                          uint64_t* total_bytes, uint64_t* free_bytes);
bool      ss_storage_present(ss_storage_kind_t kind);

#ifdef __cplusplus
} // extern "C"
#endif
