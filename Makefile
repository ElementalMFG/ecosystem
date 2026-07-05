# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# Monorepo build wrapper (S-02-002): one command per board target.
# Requires ESP-IDF exported (`. $IDF_PATH/export.sh`) or run inside the
# reproducible container (ci/containers/firmware/Dockerfile, VA-01).
#
#   make lite | alpha | omega     build firmware for that board
#   make flash-<board>            build + flash (e.g. make flash-lite)
#   make clean-<board>            remove that board's build dir
#   make help                     this text

FIRMWARE_DIR := firmware
BOARDS       := lite alpha omega

.PHONY: help $(BOARDS) $(addprefix flash-,$(BOARDS)) $(addprefix clean-,$(BOARDS))

help:
	@sed -n 's/^#   //p' Makefile

$(BOARDS):
	cd $(FIRMWARE_DIR) && idf.py -B build/$@ -DSS_BOARD=$@ build

$(addprefix flash-,$(BOARDS)): flash-%:
	cd $(FIRMWARE_DIR) && idf.py -B build/$* -DSS_BOARD=$* flash monitor

$(addprefix clean-,$(BOARDS)): clean-%:
	rm -rf $(FIRMWARE_DIR)/build/$*
