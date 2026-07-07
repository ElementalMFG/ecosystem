# SPDX-License-Identifier: Apache-2.0
# On-target Unity test app entry component (S-02-015).
#
# REQUIRES `unity` (the ESP-IDF-bundled Unity framework + IDF test runner) and
# the components under test. Add new on-target suites by listing their source
# here and REQUIRE-ing the component they exercise.

idf_component_register(
    SRCS
        "test_app_main.c"
        "test_ss_hal_smoke.c"
    INCLUDE_DIRS "."
    REQUIRES unity ss_hal ss_log
)
