# SPDX-License-Identifier: Apache-2.0
# Host harness for the ss_log redaction core (no firmware toolchain required).

CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -I../../include

SRCS := test_ss_log_format.c ../../src/ss_log_format.c
BIN := test_ss_log_format

.PHONY: test clean

test: $(BIN)
	./$(BIN)

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(BIN)
