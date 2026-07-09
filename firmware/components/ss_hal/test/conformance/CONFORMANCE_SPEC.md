<!-- SPDX-License-Identifier: Apache-2.0 -->
<!-- Copyright (c) 2026 SS-SP Project Contributors -->

# HAL conformance vectors — frozen spec (S-03-022)

Companion to `ss_hal_conformance.h` (the API contract of record; on any
conflict, the header wins). This spec freezes the authoring rules, the
minimum vector matrix, and the host/target reuse plan that `t2-builder`
implements.

## 1. Purity and file plan

| File | Owner | Rule |
|---|---|---|
| `ss_hal_conformance.h` | frozen (this story) | pure C: `<stdint.h>/<stdbool.h>/<stddef.h>` only |
| `ss_hal_conformance_core.c` | t2-builder | runner + diff formatter; same purity rule; no heap, no `<stdio.h>` |
| `ss_hal_conformance_vectors.c` | t2-builder | builtin `static const` tables; same purity rule |
| `firmware/test/host/mocks/ss_hal_mock_env.h/.c` | t2-builder | pure-C mock environment (models the five domains' documented semantics; claims all five domain caps) |
| `firmware/test/host/tests/test_hal_conformance.cpp` | t2-builder | gtest wrapper: runs builtin vectors against the mock env, asserts return `0`; prints emitted diff lines on failure; unit-tests `ss_conf_diff_format` against literal frozen strings |
| target env adapter + Unity wiring | **S-02-015 (deferred)** | thin adapter calling real `ss_*` HAL; links the SAME core + vectors objects |

The three conformance-dir files are **not** added to `ss_hal`'s
`idf_component_register` SRCS in this story — target wiring belongs to
S-02-015. Nothing in them may preclude that link (no host-isms).

## 2. Vector authoring rules

1. Every vector starts from pristine state: the runner calls `env->reset`
   before each vector; vectors MUST NOT depend on another vector having run.
2. One vector = one behavior claim. Name it kebab-case after the claim
   (`wake-timer-arg-validation`), unique within its domain.
3. Pin only what the claim is about: set `expect.check` bits sparingly.
   A don't-care field is left unchecked, not guessed.
4. Where a domain header documents pre/post-conditions, the vector encodes
   them exactly (e.g. `ss_hal_power.h`: `us == 0` and
   `us > SS_PWR_WAKE_TIMER_MAX_US` → `0x102` with prior arming unchanged;
   `wake_timer_clear` idempotent → `0` either way).
5. Where a header is silent, vectors follow house policy (refuse, never
   force): required-pointer `NULL` → `SS_CONF_RET_ERR_INVALID_ARG`;
   use-before-open/init → `SS_CONF_RET_ERR_INVALID_STATE`. These vectors are
   then **normative** for future drivers; relaxing one requires amending the
   domain header first.
6. No key material, pairing flows, captured traffic, or wire bytes in any
   table (05_SECURITY_MODEL.md scope; T1 per doc 10 §8). Payload args carry
   lengths only — the environment synthesizes buffer contents.
7. Keep vectors ≤ 32 steps (target RAM/log friendliness; advisory).
8. `SS_PWR_WAKE_TIMER_MAX_US` cannot be included (purity); vectors express
   the over-max probe as the literal `30ULL*24*60*60*1000000 + 1` with a
   comment citing the macro. If the cap is ever raised, this vector is
   updated in the same commit (backward-compatible direction only).

## 3. Minimum vector matrix (AC floor — builder may add, never remove)

| Domain | Mandatory vectors |
|---|---|
| power | `lifecycle-status` (init → status ok → enter LIGHT_SLEEP, state observed); `wake-timer-arg-validation` (set valid → armed; set 0 → `0x102`, still armed; set over-max → `0x102`, still armed); `wake-timer-clear-idempotent` (clear twice → `0` both, aux bit0 clear); `shutdown-disarms-wake` (arm timer, enter SHUTDOWN, aux bit0 clear) |
| audio | `mic-lifecycle` (open/read/close, state bit0 tracks); `spk-volume-mute` (open, volume 40, mute on/off, aux+state track); `use-before-open` (mic_read and spk_write before open → `0x103`); `buzzer-beep` (beep, aux high-16 = freq) |
| lora | `init-config-tx` (init → IDLE, config → cfg count 1, tx → tx count 1); `rx-start-stop` (state IDLE→RX→IDLE); `use-before-init` (config/tx before init → `0x103`); `sleep-wake` (sleep on → SLEEP, sleep off → IDLE) |
| wifi | `lifecycle` (init → READY, config, start → STARTED, stop → READY); `config-null` (config `NULL` → `0x102`, aux count unchanged); `sleep-toggle` (sleep on → SLEEPING, off → prior state) |
| ble | `advertise-stop` (init, advertise "ss-sp-conf" → bit1 set + count 1, stop → bit1 clear); `scan-lifecycle` (scan start/stop, bit2 tracks, count 1); `advertise-null-name` (advertise `NULL` → `0x102`); `use-before-init` (advertise before init → `0x103`) |

Every mandatory vector's `requires_caps` carries its domain cap
(`SS_CAP_RADIO_LORA`, `SS_CAP_RADIO_BLE`, `SS_CAP_RADIO_WIFI4`,
`SS_CAP_MIC|SS_CAP_SPEAKER` or `SS_CAP_BUZZER` as applicable; power vectors
use `0` — power is unconditional).

## 4. Diff format (frozen — restated from the header)

```text
CONF-FAIL dom=<domain> vec=<name> step=<N> op=<OPNAME> field=<ret|state|aux|exec> expected=0x<XXXXXXXX> actual=0x<XXXXXXXX>
```

One line per mismatched field; 8 lowercase zero-padded hex digits; decimal
0-based step; no trailing newline; emitted only via `env->emit`. The gtest
wrapper asserts this grammar with literal expected strings, so any formatter
drift fails CI.

## 5. Host wiring (this story) and target plan (S-02-015)

`firmware/test/host/CMakeLists.txt`:

- append `${SS_COMPONENTS_DIR}/ss_hal/test/conformance/ss_hal_conformance_core.c`,
  `.../ss_hal_conformance_vectors.c`, and
  `${CMAKE_CURRENT_SOURCE_DIR}/mocks/ss_hal_mock_env.c` to `ss_units`;
- add `${SS_COMPONENTS_DIR}/ss_hal/test/conformance` to `ss_units` PUBLIC
  include dirs;
- add `tests/test_hal_conformance.cpp` to `ss_host_tests`.

`firmware/test/host/mocks/esp_err.h`: add `ESP_ERR_INVALID_STATE 0x103` and
`ESP_ERR_TIMEOUT 0x107` (numeric mirrors are frozen in the header).

`.github/workflows/host-tests.yml`: add `firmware/components/ss_hal/**` to
BOTH the `pull_request` and `push` path lists (the conformance core lives
under `ss_hal`; today only `firmware/test/host/**` would catch part of it).

Target (S-02-015, no board yet): a Unity runner links the identical
`ss_hal_conformance_core.c` + `ss_hal_conformance_vectors.c`, provides an
`ss_conf_env_t` whose `exec` calls the real HAL and reads back state per the
frozen encodings, `reset` checks `ss_hal_has_cap(vec->requires_caps)` to
skip honestly, and `emit` routes to the console. Nothing in this story may
add a host dependency that breaks that link.
