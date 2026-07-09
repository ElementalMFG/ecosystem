# SPDX-License-Identifier: Apache-2.0
# Host harness for the ss_storage pure FS core (no firmware toolchain needed).
#
# Only the pure core (ss_storage_fs_core.c) is compiled here; the IDF glue
# (ss_storage.c) is exercised on target. The core header includes only
# <stdint.h>/<stdbool.h>/<stddef.h>, so it builds with a plain gcc under
# ASan/UBSan with no include hacks and no libm.

CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -I../../include

CORE_SRCS := test_ss_storage_fs_core.c ../../src/ss_storage_fs_core.c
CORE_BIN := test_ss_storage_fs_core

BINS := $(CORE_BIN)

.PHONY: test clean

test: $(BINS)
	./$(CORE_BIN)

$(CORE_BIN): $(CORE_SRCS)
	$(CC) $(CFLAGS) -o $@ $(CORE_SRCS)

clean:
	rm -f $(BINS)
