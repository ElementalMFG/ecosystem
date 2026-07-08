# SPDX-License-Identifier: Apache-2.0
# Host harness for the ss_audio decision cores (no firmware toolchain needed).
#
# Only the pure cores (ss_audio_core.c / ss_spk_core.c) are compiled here; the
# IDF glue (ss_mic.c / ss_spk.c) needs the ESP-IDF I2S driver and is exercised
# on target. The core headers pull ss_hal_audio.h which includes <esp_err.h>,
# resolved via the shared host mock.

CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
	-I../../include -I../../../ss_hal/include -I../../../../test/host/mocks

MIC_SRCS := test_ss_audio_core.c ../../src/ss_audio_core.c
MIC_BIN := test_ss_audio_core

SPK_SRCS := test_ss_spk_core.c ../../src/ss_spk_core.c
SPK_BIN := test_ss_spk_core

.PHONY: test clean

test: $(MIC_BIN) $(SPK_BIN)
	./$(MIC_BIN)
	./$(SPK_BIN)

$(MIC_BIN): $(MIC_SRCS)
	$(CC) $(CFLAGS) -o $@ $(MIC_SRCS)

$(SPK_BIN): $(SPK_SRCS)
	$(CC) $(CFLAGS) -o $@ $(SPK_SRCS)

clean:
	rm -f $(MIC_BIN) $(SPK_BIN)
