# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# pytest_ss_target.py — CI harness for the on-target Unity test app (S-02-015).
#
# Runs on a host connected to a flashed board via pytest-embedded
# (pytest-embedded-idf + pytest-embedded-serial-esp). It flashes the built app,
# opens the console serial, and parses the Unity summary the runner prints
# (see main/test_app_main.c). This is the "results reported over serial to CI"
# acceptance criterion — it requires an attached Lite board (or QEMU), so it
# runs on a self-hosted runner, not on the container build-only CI job.
#
# Invoke (from this directory, after `idf.py -DSS_BOARD=lite build`):
#   pytest --embedded-services esp,idf --target esp32s3
import pytest


@pytest.mark.esp32s3
def test_ss_target_unity_smoke(dut):
    """Flash, run every on-target Unity case, and require a clean summary.

    dut.expect_unity_test_output() consumes the standard Unity report and
    raises if any case fails; asserting on the parsed record makes the
    zero-failure requirement explicit rather than implicit in the timeout.
    """
    dut.expect_unity_test_output(timeout=90)
    assert len(dut.testsuite.testcases) > 0, "no Unity cases ran on target"
    assert dut.testsuite.attrs["failures"] == 0
    assert dut.testsuite.attrs["errors"] == 0
