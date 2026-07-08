# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# Host harness for the ss_wifi STA policy core (no firmware toolchain needed).
#
# Only the pure core (ss_wifi_sta_core.c) is compiled here; the IDF glue
# (ss_wifi.c) is exercised on target. The core header includes only
# <stdint.h>/<stdbool.h>/<stddef.h>, so it builds with a plain gcc under
# ASan/UBSan with no include hacks and no libc qsort.

CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -I../../include

SRCS := test_ss_wifi_sta_core.c ../../src/ss_wifi_sta_core.c
BIN := test_ss_wifi_sta_core

.PHONY: test clean

test: $(BIN)
	./$(BIN)

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(BIN)
