// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_storage.h — public surface of the ss_storage component that is NOT part
// of the ss_hal_storage.h contract. The mount/stat/present API is declared in
// ss_hal_storage.h (the HAL contract this component implements); this header
// adds the diagnostics helper used to surface wear-levelling stats.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Log the LittleFS user-FS wear/usage stats (total / used / free / fill%) at
// INFO level under the "ss.storage" tag. Safe to call before a successful
// mount: it logs a warning and returns without touching the filesystem.
// Wired into the diagnostics surface (see firmware/main/ss_diag).
void ss_storage_log_stats(void);

#ifdef __cplusplus
} // extern "C"
#endif
