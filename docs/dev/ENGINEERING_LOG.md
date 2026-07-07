<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Engineering log (append-only)

One-to-three lines per entry, newest LAST, appended by the closing step of
every story run (doc 11 §6d). This is the raw capture layer for cross-story
knowledge: anything a *future* story needs that is not already expressed in
a contract doc-block, a rules file, or a `TODO(S-NN-MMM)` code marker lands
here first. Durable domain facts graduate from here into `.claude/rules/`.
Grep it at story start: `grep -i <component|story-id> docs/dev/ENGINEERING_LOG.md`.

Format: `- S-NN-MMM (YYYY-MM-DD): fact.` Never rewrite old entries.

---

- S-02-004 (2026-07-07): models/CATALOG + doc 00 §1.4 are the only sources
  allowed to "lock" Alpha/Omega hardware facts; everything else is a
  TODO(models/CATALOG) placeholder — never invent pins.
- S-02-006 (2026-07-07): compass ODR macro feeds `1000/SS_MAG_ODR_HZ` at
  compile time — any board setting SS_MAG_ODR_HZ=0 breaks the build.
- S-02-009 (2026-07-07): gnss/coproc pump tasks block on
  `xQueueReceive(portMAX_DELAY)` — they are TWDT-EXEMPT by design; do not
  "fix" them to feed the watchdog.
- S-02-007 (2026-07-07): ss_log_format.c is the redaction guarantee and is
  deliberately IDF-free; new logging features must keep the pure core free
  of ESP-IDF includes so host tests keep covering it.
- S-02-008 (2026-07-07): a coredump read raw from the flash partition is
  `raw` format (IDF header) even though the payload is ELF — decode with
  `--core-format auto`; on flash-encrypted release units serial partition
  reads return ciphertext (device-side export only, S-02-016).
- S-02-008 (2026-07-07): reset-reason classification is three-way
  (CRASH/BENIGN/INDETERMINATE-preserves-count); ESP_RST_EFUSE is
  INDETERMINATE; unknown reasons must never clear the panic counter.
- S-02-010 (2026-07-07): CI workflow steps run under `bash -e` — asserting
  a nonzero exit needs the `rc=0; cmd || rc=$?; test $rc -eq N` idiom, and
  `! cmd` accepts ANY nonzero (assert exact codes instead).
- S-02-014 (2026-07-07): host-testable pure cores compile into
  firmware/test/host `ss_units`; only board_config.h needs mocking — the
  real ss_hal_caps/types headers are host-clean.
- infra (2026-07-07): agent-memory markdown files need SPDX headers or
  lint-docs fails; `github.event.pull_request.base.sha` can be stale
  (PR-creation-time); a job-level-skipped matrix job reports its bare job
  name, which is why required contexts use always-run gate jobs.
