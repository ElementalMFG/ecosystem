// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_lora.c — ESP-IDF glue for the LoRa radio path. Today this is a stub: the
// pure policy core (ss_lora_lbt_core.c) is complete and host-tested under
// S-03-013, but the on-target SX1262 driver integration is a separate story.
//
// TODO(S-03-011): wire ss_lora_lbt_core into the SX1262 driver — feed RSSI/CAD to
// ss_lora_tx_evaluate and gate ss_lora_tx().

#include "ss_lora_lbt_core.h"
