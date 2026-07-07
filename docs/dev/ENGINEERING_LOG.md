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
- S-02-015 (2026-07-07): on-target Unity tests live in a SEPARATE IDF project
  (`firmware/test/target/`), NOT reached by `make lite` — add on-target suites
  by listing the source in its `main/CMakeLists.txt` and REQUIRE-ing the
  component. The `.gitignore` `target/` (Rust build) pattern swallows the dir;
  a `!firmware/test/target/` negation keeps the source tracked. The pinned IDF
  digest is now in FOUR files (Dockerfile + devcontainer + firmware-build.yml +
  target-tests.yml) — bump all four together (RFC-0002).
- S-02-015 (2026-07-07): `ss_hal_has_cap`/`ss_hal_board_id`/`ss_hal_caps_mask`
  are still unimplemented (ss_hal is header-only until EPIC-03), so the on-target
  HAL smoke test asserts compile-time `SS_BOARD_CAPS`/identity macros only; a
  runtime-accessor smoke test waits on the impl (EPIC-03 HAL bring-up).
- infra (2026-07-07): a NEW CI gate must land in the same commit as its
  addition to the `verify`/`story-run` skill gate lists — the first headless
  queue run failed docs-lint because ALLOCATION_MAP.md (which carries story
  Status) wasn't regenerated locally; local gates and CI gates must stay
  identical sets.
- S-02-015 (2026-07-07): CI jobs that run pytest need an explicit
  `pip install pytest pytest-embedded[-idf,-serial-esp]` step — GitHub's
  setup-python ships no packages.
- S-02-016 (2026-07-07): recovery BOOT-hold watcher owns GPIO0 (input+pull-up) for the first CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS (default 10 s) of every boot — EPIC-05 radio bring-up must init LoRa SPI (GPIO0 = SS_LORA_PIN_CS) only after the window closes, or shrink/disable the window; holding BOOT THROUGH reset is ROM download mode, so the recovery gesture is press-RESET-then-hold-BOOT (contract: firmware/main/ss_recovery.h).
- infra (2026-07-07): tier recipes v2 (doc 11 §6f) — T2 = t2-designer
  (fable@medium) contract + t2-builder (opus@high) build, orchestrated on
  opus@medium; t3-standard pinned opus@medium with two-failure escalation to
  t2-builder; fable@low is dominated everywhere (2x price without the
  deliberation edge) — never use it.
