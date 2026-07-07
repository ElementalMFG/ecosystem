# SPDX-License-Identifier: Apache-2.0
# ss_log — leveled, colorized console logging with %k key-material redaction.
#
# ss_log_format.c is the pure, ESP-IDF-free redaction core (host-tested under
# ASan/UBSan in test/host/); ss_log.c is the IDF sink layer. esp_timer supplies
# the millisecond console timestamp.

idf_component_register(
    SRCS "src/ss_log.c" "src/ss_log_format.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer
)
