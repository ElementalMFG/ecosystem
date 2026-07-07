#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# xtensa-decode-crash.py — decode an SS-SP panic core dump (S-02-008).
#
# Thin, target-aware wrapper over ESP-IDF's espcoredump.py. The firmware
# writes ELF-format dumps (CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF) to the
# `coredump` partition (partitions.csv); this tool decodes either a dump file
# read out of that partition or the dump on a live, attached chip.
#
# Run it where ESP-IDF exists — most simply inside the pinned build container
# (docs/dev/BUILDING.md):
#
#   # live chip:
#   tools/xtensa-decode-crash.py firmware/build/lite/ss_firmware.elf --port /dev/ttyACM0
#   # dump file previously read out of the coredump partition (raw image with
#   # the IDF core_dump header — format auto-detected):
#   tools/xtensa-decode-crash.py firmware/build/lite/ss_firmware.elf --core dump.bin
#
# The name honours the S-02-008 AC; the tool itself is architecture-aware:
# Lite is Xtensa (esp32s3), Alpha will decode with --target esp32p4 (RISC-V)
# once that port is real — espcoredump handles both.
#
# RELEASE UNITS (flash encryption, EPIC-08): raw serial/esptool partition
# reads — including espcoredump's --port path — return CIPHERTEXT and decode
# to garbage. On encrypted units, export the dump through the device-side
# path (S-02-016 recovery UX), then decode the exported file with --core.

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys


def find_espcoredump(idf_path: str | None) -> list[str] | None:
    """Return the command prefix for espcoredump, or None if unavailable."""
    exe = shutil.which("espcoredump.py")
    if exe:
        return [exe]
    if idf_path:
        candidate = os.path.join(idf_path, "components", "espcoredump", "espcoredump.py")
        if os.path.isfile(candidate):
            return [sys.executable, candidate]
    return None


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Decode an SS-SP panic core dump (wraps ESP-IDF espcoredump.py)."
    )
    parser.add_argument("elf", help="app ELF matching the crashed firmware (build/<board>/*.elf)")
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--core", help="core dump file read from the coredump partition")
    source.add_argument("--port", help="serial port of a live chip to read the dump from")
    parser.add_argument(
        "--target",
        default="esp32s3",
        help="chip target (default esp32s3 / Lite; Alpha: esp32p4 when real)",
    )
    parser.add_argument(
        "--core-format",
        default="auto",
        choices=["auto", "elf", "raw", "b64"],
        help="format of --core file. A file read straight out of the coredump "
        "partition is 'raw' (IDF core_dump header) even though the firmware "
        "encodes ELF inside; an already-extracted dump is 'elf'. Default "
        "'auto' detects either (espcoredump's own default).",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.elf):
        print(f"error: ELF not found: {args.elf}", file=sys.stderr)
        return 2
    if args.core and not os.path.isfile(args.core):
        print(f"error: core dump file not found: {args.core}", file=sys.stderr)
        return 2

    prefix = find_espcoredump(os.environ.get("IDF_PATH"))
    if prefix is None:
        print(
            "error: espcoredump.py not found. Run inside the pinned ESP-IDF "
            "container (docs/dev/BUILDING.md) or export IDF_PATH.",
            file=sys.stderr,
        )
        return 2

    cmd = prefix + ["--chip", args.target]
    if args.port:
        cmd += ["--port", args.port]
    cmd += ["info_corefile"]
    if args.core:
        cmd += ["--core", args.core, "--core-format", args.core_format]
    cmd += [args.elf]

    print("+ " + " ".join(cmd), file=sys.stderr)
    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())
