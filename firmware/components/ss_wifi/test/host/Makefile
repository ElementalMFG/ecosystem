# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# Host harness for the ss_wifi pure cores (no firmware toolchain needed).
#
# Only the pure cores (ss_wifi_sta_core.c, ss_wifi_prov_core.c) are compiled
# here; the IDF glue (ss_wifi.c, ss_wifi_prov.c) is exercised on target. The
# core headers include only <stdint.h>/<stdbool.h>/<stddef.h>, so they build
# with a plain gcc under ASan/UBSan with no include hacks.

CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Wshadow -Wconversion -Werror -fsanitize=address,undefined -I../../include

STA_BIN := test_ss_wifi_sta_core
PROV_BIN := test_ss_wifi_prov_core

.PHONY: test clean

test: $(STA_BIN) $(PROV_BIN)
	./$(STA_BIN)
	./$(PROV_BIN)

$(STA_BIN): test_ss_wifi_sta_core.c ../../src/ss_wifi_sta_core.c
	$(CC) $(CFLAGS) -o $@ $^

$(PROV_BIN): test_ss_wifi_prov_core.c ../../src/ss_wifi_prov_core.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(STA_BIN) $(PROV_BIN)
