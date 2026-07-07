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
#   make lint-docs                lint markdown docs (links, anchors, SPDX, TOCs)
#   make story S=S-NN-MMM         launch a tier-correct Claude session for a story
#   make queue Q="S-NN-MMM ..."   run a headless story queue (tools/claude/run-queue.sh)
#   make help                     this text

FIRMWARE_DIR := firmware
BOARDS       := lite alpha omega

.PHONY: help lint-docs story queue $(BOARDS) $(addprefix flash-,$(BOARDS)) $(addprefix clean-,$(BOARDS))

help:
	@sed -n 's/^#   //p' Makefile

lint-docs:
	python3 tools/lint-docs.py

story:
	tools/claude/work.sh $(S)

queue:
	tools/claude/run-queue.sh $(Q)

$(BOARDS):
	cd $(FIRMWARE_DIR) && idf.py -B build/$@ -DSS_BOARD=$@ build

$(addprefix flash-,$(BOARDS)): flash-%:
	cd $(FIRMWARE_DIR) && idf.py -B build/$* -DSS_BOARD=$* flash monitor

$(addprefix clean-,$(BOARDS)): clean-%:
	rm -rf $(FIRMWARE_DIR)/build/$*
