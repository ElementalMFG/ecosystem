# SPDX-License-Identifier: Apache-2.0
# Host harness for the ss_lora LBT + duty-cycle policy core (no firmware
# toolchain needed).
#
# Only the pure core (ss_lora_lbt_core.c) is compiled here; the IDF glue
# (ss_lora.c) is exercised on target. The core header includes only
# <stdint.h>/<stdbool.h>/<stddef.h>, so it builds with a plain gcc under
# ASan/UBSan with no include hacks and no libm.

CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -I../../include

LBT_SRCS := test_ss_lora_lbt_core.c ../../src/ss_lora_lbt_core.c
LBT_BIN := test_ss_lora_lbt_core

PA_SRCS := test_ss_lora_pa_core.c ../../src/ss_lora_pa_core.c
PA_BIN := test_ss_lora_pa_core

BINS := $(LBT_BIN) $(PA_BIN)

.PHONY: test clean

test: $(BINS)
	./$(LBT_BIN)
	./$(PA_BIN)

$(LBT_BIN): $(LBT_SRCS)
	$(CC) $(CFLAGS) -o $@ $(LBT_SRCS)

$(PA_BIN): $(PA_SRCS)
	$(CC) $(CFLAGS) -o $@ $(PA_SRCS)

clean:
	rm -f $(BINS)
