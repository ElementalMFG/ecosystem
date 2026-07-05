# SPDX-License-Identifier: Apache-2.0
# ss_hal — capability-flagged hardware abstraction layer (headers + board port).
#
# The active board port directory (firmware/boards/${SS_BOARD}/) is exported on
# the include path so `#include "board_config.h"` resolves to the selected
# board. HAL implementations (hal_*.c) land per the EPIC-03 bring-up stories;
# until then this is a header-only component.

idf_component_register(
    INCLUDE_DIRS "include" "${CMAKE_CURRENT_LIST_DIR}/../../boards/${SS_BOARD}"
    REQUIRES driver esp_timer
)
