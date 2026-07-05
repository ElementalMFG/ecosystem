// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Low-level OTA plumbing (partition switching, rollback counter, image staging).
// Signature verification and manifest handling live in components/ss_ota.

#pragma once
#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

typedef enum { SS_OTA_SLOT_A, SS_OTA_SLOT_B, SS_OTA_SLOT_FACTORY } ss_ota_slot_t;

esp_err_t ss_ota_current_slot(ss_ota_slot_t* out);
esp_err_t ss_ota_stage_begin(ss_ota_slot_t target);
esp_err_t ss_ota_stage_write(const void* buf, size_t n);
esp_err_t ss_ota_stage_finish(void);
esp_err_t ss_ota_activate(ss_ota_slot_t target);   // marks pending, reboots
esp_err_t ss_ota_commit(void);                     // called after healthy boot
esp_err_t ss_ota_rollback(void);
esp_err_t ss_ota_rollback_counter(uint32_t* out);
