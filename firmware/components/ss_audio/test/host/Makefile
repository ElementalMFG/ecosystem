# SPDX-License-Identifier: Apache-2.0
# Host harness for the ss_audio mic decision core (no firmware toolchain needed).
#
# Only the pure core (ss_audio_core.c) is compiled here; ss_mic.c needs the IDF
# I2S driver and is exercised on target. ss_audio_core.h pulls ss_hal_audio.h
# which includes <esp_err.h>, resolved via the shared host mock.

CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
	-I../../include -I../../../ss_hal/include -I../../../../test/host/mocks

SRCS := test_ss_audio_core.c ../../src/ss_audio_core.c
BIN := test_ss_audio_core

.PHONY: test clean

test: $(BIN)
	./$(BIN)

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(BIN)
