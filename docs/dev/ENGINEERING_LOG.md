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
- S-02-017 (2026-07-07): settings-class persistence goes through the ss_nvs wrapper (firmware/main/ss_nvs.h), never raw nvs_flash. Each namespace carries a uint32 schema version under reserved key "__ver"; the "__" prefix is reserved module-wide (validators reject it) so ns/key names are printable-non-space ASCII, 1..15 bytes, no "__". Every get/set declares the caller's code version: stored<code runs the registered migration hook (idempotent; register before first access; UPGRADE with no hook is REFUSED); stored>code is REFUSED (SS_NVS_ERR_DOWNGRADE), never auto-migrated. NOT for keys/ratchet/anti-rollback counter (EPIC-08/09 T1). Pure planner+validators host-tested; nvs_flash glue on target.
- infra (2026-07-07): headless story-run rules — push is part of the commit
  step (never held for permission); scratch/probe files go in /tmp only
  (sandbox rm may be denied); cmake/ctest now allowlisted so headless
  workers can run host tests locally instead of deferring to CI.
- infra (2026-07-07): NEVER `git add -A` — stage explicit paths. A root-level
  personal note file was swept into a public commit; root `*.txt` is now
  gitignored and tip-removed (history retains it; no secrets were present).
- S-01-018 (2026-07-07): scope-lock RFC drafted (rfcs/DRAFT-scope-lock.md) —
  capability audit verified drift-not-gaps; wake-word "gap" was false
  (S-14-013 exists); compatibility guards bind partition headroom, versioned
  surfaces, HAL caps, and the pin-sharing ledger.
- S-01-018 (2026-07-08): RFC-0004 scope lock RATIFIED (D-0019) — identity is
  "sovereign multi-band mesh communicator + universal node"; browsing is
  application-layer only (guard 6 binds the EPIC-15/18 plugin platform to
  the smartphone-app capability floor: networked + UI-rendering + storage-
  scoped plugins); video = signed plugins post-v1.0; ss_ai scaffold-only
  until v2.x; v1.0 ships Lite+Alpha; SDKs C/Rust/Python (TS/Dart v1.1).
  All additive — no story deleted or narrowed.
- S-01-018 (2026-07-08): RFC-0004 follow-through audit — guard 6 needed NO
  new stories (EPIC-18 S-18-004/005/006 + reference plugins already cover
  the smartphone-app capability floor; S-18-018 is the video-plugin story);
  added S-05-020 (Omega v1.x spec-lock, promised by SL-5) and recorded the
  SL-6 re-train on S-20-007/008/009/010 (Prio P3 + D-0019 Deps note —
  deferred, not descoped).
- S-02-019 (2026-07-07): SBOM tooling landed — `tools/gen-sbom.py` emits
  deterministic CycloneDX 1.6 per board; first-party components enumerated at
  runtime (dirs `firmware/components/ss_*` WITH a `CMakeLists.txt`, 2 today),
  never hardcoded. IDF v5.3.5 `project_description.json` exposes the IDF
  version under key `git_revision` (no `idf_version` key); toolchain parsed
  from the `c_compiler` path. Keyless attest/publish is tag-only in
  `release-sbom.yml` → story parks IN_REVIEW until a `v*` tag exercises it.
- S-02-020 (2026-07-07): firmware version provenance = `firmware/main/ss_version.h`
  accessors (`ss_fw_git_sha/tag/build_id`), values injected at configure time by
  `firmware/main/version.cmake` (compile defs on the `main` component only) and
  printed in `main.cpp` banner(). Single source of truth with the SBOM: both use
  `git describe --tags --always --dirty` (tag) + `git rev-parse HEAD` (sha), so
  values are byte-identical to `gen-sbom.py` — asserted by
  `tools/tests/test_fw_version_matches_sbom.py`. build_id = `<board>-<sha12>`.
  Future OTA/diag/version-query stories: reuse the three accessors, never re-derive.
- infra (2026-07-08): STATUS-GUARD (exit 5) — allocation.py --eligible now
  refuses stories that are not DRAFT/READY (DONE=already executed,
  IN_PROGRESS/IN_REVIEW=in flight, BLOCKED=parked) and run-queue.sh aborts
  on it; re-executing a completed story is structurally impossible.
- EPIC-03 (2026-07-08): hardware-reality triage — early EPIC-03 stories were
  template-written before the Lite lock. Retargeted to real Lite hardware:
  S-03-001 (no-gauge power contract), S-03-004 (GT911 touch + BOOT button),
  S-03-006 (ILI9488, from board_config, never hardcoded), S-03-018
  (reconcile with the FROZEN partitions.csv, don't reinvent). DROPPED-with-
  pointer (capability moved intact to Alpha): S-03-002→S-04-026 (charger),
  S-03-005→S-04-028 (haptic), S-03-020→S-04-029 (LEDs). Caught by a T3
  worker at opus@medium refusing to implement a MAX17048 on a board whose
  frozen contracts all say it has none (doc 10 §8 escalation working).
  Remaining EPIC-03 titles pass the hardware audit at title level; AC-level
  checks continue per story at elaboration.
- EPIC-03 (2026-07-08): owner confirmed the physical Lite dev-fleet build
  (CrowPanel 3.5" S3 + Elecrow HaLow on the wireless header + Elecrow GPS on
  UART1 + Elecrow 3-axis compass on I2C0 + speaker; C6 on UART2 optional).
  Doc 01 already matches, but NO driver stories existed for the plugged
  modules — added S-03-026 (HaLow-on-SPI, distinct from Alpha's SDIO path),
  S-03-027 (hal_gnss), S-03-028 (compass, with an explicit HMC@0x1E-vs-
  QMC@0x0D part-verify AC), S-03-029 (C6 link). First-hardware-session
  verify list: compass part/address, GNSS part number, C6 port assignment.
- S-03-001 (2026-07-07): `ss_power` is the first real firmware/components/ss_power
  impl — pattern = pure host-testable decision core (`ss_power_core.[ch]`, no
  ESP-IDF calls) + thin IDF glue (`ss_power.c`). Host tests reach it via a new
  `firmware/test/host/mocks/esp_err.h` stub, which unblocks host-testing ANY
  core that includes an `ss_hal_*.h` (they pull `esp_err.h`) — reuse for the
  other EPIC-03 HAL impls. Lite has no battery sense (C-01 §Meshtastic-#7993):
  `ss_power_status` is fixed no-gauge (`v_mv=0`, `battery_sense_valid=false`).
- S-03-001 (2026-07-08): ESP32-S3 sleep APIs — esp_deep_sleep_enable_gpio_wakeup
  and ESP_GPIO_WAKEUP_GPIO_* DO NOT EXIST on Xtensa (RISC-V-only API). S3 deep
  wake = esp_sleep_enable_ext1_wakeup (ANY_HIGH mask) / ext0 (one pin, either
  level), RTC-capable GPIO 0..21 only; light-sleep wake = gpio_wakeup_enable +
  esp_sleep_enable_gpio_wakeup (any GPIO — this is how touch INT 47 wakes).
  Registration must be sleep-mode-specific.
- infra (2026-07-08): the .claude/rules files are VERIFIED-AT-A-DATE snapshots —
  a stale rule ("alpha/omega fail at configure by design") misled a worker into
  believing a correct AC was unsatisfiable. When repo reality changes a rule's
  claim, update the rule in the SAME commit (now done for firmware.md).
- S-03-003 (2026-07-08): wake-set triage — "button" wake was drift; canonical
  Lite trio = touch INT (GPIO47, LIGHT-sleep wake only), LoRa DIO1 (GPIO1,
  light+deep), RTC timer. Timer wake had NO surface in the frozen
  ss_hal_power.h → filed S-03-030 (T1, allocation override) rather than
  absorbing a T1-path edit into a T3 run; C-01 §4.3 reconciliation rides
  S-03-030. Worker escalation per doc 10 §8 worked exactly as designed.
- S-03-004 (2026-07-08): input events land in a NEW `ss_input` component (not
  ss_hal — the touch/buttons contracts were already frozen header-only in the
  ss_hal T1 path; implementing them in a driver component keeps the run T3 and
  out of the T1 path). GT911 @ 0x5D on I2C0 (SDA15/SCL16), INT GPIO47
  (neg-edge, INT-driven for the <25 ms PTT path), RST GPIO48; address-select
  reset = INT held low across RST rising edge. Uses the ESP-IDF 5.3
  `i2c_master.h` API (legacy `i2c.h` is `-Wdeprecated`).
- S-03-004 (2026-07-08): GPIO0 now has THREE ordered consumers on Lite —
  (1) recovery watcher owns it for the first CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS
  (S-02-016), (2) ss_input BOOT-button claims it as input+pull-up AFTER that
  window, (3) LoRa/HaLow CS (SS_LORA_PIN_CS = GPIO0, EPIC-05). (2) and (3)
  genuinely CONFLICT at runtime: a debounced button-input line cannot also
  drive SPI CS. S-03-004's AC only required deferral to the recovery window
  (satisfied); the button-vs-CS arbitration (mux, or drop runtime BOOT input
  when LoRa is active, or a strap-only BOOT role) is unresolved and belongs to
  EPIC-05 radio bring-up. Filed as follow-up DRAFT — do NOT let ss_input and
  the LoRa driver both configure GPIO0 without arbitration.
- infra (2026-07-08): headless workers were sandbox-denied on git write ops
  (S-03-004 finished all engineering but could not commit/push — earlier
  runs' git approvals were nondeterministic). git add/commit/push-to-main/
  fetch are now project-allowlisted: the queue DESIGN requires workers to
  push (post-checks + CI depend on it); branch protection + gates remain the
  safety net.
- S-03-004 (2026-07-08): GPIO0 has THREE ordered consumers on Lite (recovery
  watcher during entry window -> ss_input BOOT button -> SX1262 CS when LoRa
  active); BOOT-input-vs-LoRa-CS runtime conflict is real and unresolved —
  arbitration story S-03-031 must land before/with S-03-011 (LoRa driver).
- S-03-006 (2026-07-08): display HAL landed in a NEW `ss_display` component
  (core/glue split like ss_power); `main/ss_display_boot.*` retired and folded
  behind `ss_hal_display.h`. Tear-free flush = single DMA scratch reused per
  chunk, gated by an `on_color_trans_done` binary semaphore (block before
  refill) — reuse this pattern for S-03-008 (framebuffer). Backlight PWM (LEDC)
  now ships inside ss_display, so S-03-007 (auto-dim) builds on
  `ss_display_backlight`, not a fresh LEDC bring-up. ILI9488 SPI is RGB666-only
  (COLMOD 0x66); upper layers stay RGB565 and convert in flush.
- infra (2026-07-08): workers also need git rm/mv (file retirement is part of
  refactor stories) — now allowlisted alongside add/commit/push. Full worker
  git surface: add, rm, mv, commit, push origin main, fetch.
- S-03-032 (2026-07-08): ss_hal ABI baseline — ALL HAL headers now carry
  extern "C" guards (a C++ TU calling an unguarded C prototype makes a
  mangled unresolvable symbol); ss_hal_has_cap takes uint64_t (caps reach
  bit 38 — uint32_t truncated and MIS-EVALUATED high flags); accessors live
  in ss_hal/src/ss_hal.c (header-driven, no state); ss_hal_init/shutdown
  deliberately NOT stubbed so missing drivers fail loudly. Lesson: an
  undefined symbol can hide until a caller makes the object load-bearing —
  the S-03-004 ss_input reference only surfaced when S-03-006 rewired main.
- S-03-030 (2026-07-08): ss_hal_power.h timer-wake surface (T1, double-
  reviewed). Frozen semantics: countdown from each sleep entry (IDF stores a
  duration and arms at sleep start, so set-time arming is faithful), sticky
  until clear, 30-day cap as a unit-arithmetic guard, SHUTDOWN disables all
  wake sources. VERIFIED against IDF v5.3.5 sleep_modes.c: CHECK_SOURCE
  requires the trigger bit already set, so esp_sleep_disable_wakeup_source
  (TIMER) while not armed ESP_LOGEs "Incorrect wakeup source" and returns
  ESP_ERR_INVALID_STATE — glue must gate the disable on its record or every
  clean boot / idempotent clear logs an error; divergence is prevented at
  set() (commit record only after platform accepts) and init() (disarm a
  live arming BEFORE dropping the record). Glue has no host coverage by
  design (core/glue split) — on-target checks wait on S-02-015 Unity.
  HIBERNATE forward-note: today it maps to esp_deep_sleep_start so the
  header's "whichever wake source fires first" holds; revisit if HIBERNATE
  ever powers down RTC peripherals.
- S-03-030 (2026-07-08): RFC-required determination — the timer-wake HAL
  extension is ADDITIVE to an INTERNAL contract: ss_hal headers are not on
  the RFC-0003 versioned-surface list (wire/LXMF/crypto/pairing/plugin-ABI/
  node-profile/cloud), and CONTRIBUTING §10 requires RFCs for BREAKING API
  changes; additive is always allowed (06 §4.4). T1 double review is the
  correct governance for internal HAL contract changes; no RFC needed.
  Also: t1-pipeline gate list now includes allocation --generate/--check
  (same local-vs-CI parity lesson as the first queue run).
- S-03-003 (2026-07-08): canonical Lite wake set wired via a NEW
  component-local `ss_power_lite.h` / `ss_power_wake_lite_defaults()` — NOT in
  the frozen `ss_hal_power.h` ABI (kept the run T3, out of the ss_hal T1 path).
  It arms touch INT (GPIO47, level 0 — GT911 idles HIGH / active-LOW) and LoRa
  DIO1 (GPIO1, level 1 — SX1262 IRQ active-HIGH); RTC timer is NOT armed here
  (duty-cycle owner arms it via ss_power_wake_timer_set). DOWNSTREAM WIRING
  POINT: nothing in main/ calls it yet (no power-owner task exists); a future
  duty-cycle/power-owner story must call it once after ss_power_init(). The
  light-vs-deep partition is now a pure host-tested predicate
  ss_power_core_wake_is_deep_capable() (S3 RTC range 0..21); glue keeps
  rtc_gpio_is_valid_gpio() as defense-in-depth. GPIO polarities are documented
  assumptions pending confirmation at the D-0013 bench session
  (docs/dev/BENCH_POWER_LITE.md; sleep <= 0.5 mA, wake latency — parks the
  story at IN_REVIEW).
- S-03-009 (2026-07-08): second orphaned-arbiter catch — ss_hal_muxctl.h
  (GPIO45 mic/radio mux) was declared with NO implementation and NO owning
  story; mic (9/3/10) and LoRa share the pin set behind it. Filed S-03-033
  (ss_muxctl component) and dep-wired S-03-009 + S-03-011 onto it (plus
  S-03-031 for LoRa's GPIO0). Pattern to watch: every ss_hal_*.h arbiter/
  contract needs an implementing story before its consumers queue.
- S-03-033 (2026-07-08): ss_muxctl implements the GPIO45 mux — pure decision
  core (ss_muxctl_core, host-tested) + FreeRTOS glue (binary-semaphore
  ownership token for blocking/busy-reject + portMUX-guarded state; GPIO
  writes kept OUTSIDE the spinlock). No-op on non-mux boards via runtime
  ss_hal_has_cap(SS_CAP_MUX_MIC_RADIO). PORTABILITY GOTCHA: parity board
  macros for absent pins are -1 (SS_MUX_MIC_RADIO_PIN=-1 on alpha/omega), and
  the glue compiles on ALL boards — a constant `1ULL << -1` / negative pin is
  fatal under -Werror, so any board-pin GPIO code must sit behind
  `#if <PIN> >= 0` even when it is runtime-dead. Applies to every future
  component that touches a parity-placeholder pin macro.
- audit (2026-07-08): orphaned-artifact sweep (4 classes) — 6 HAL-surface
  GAPs + 2 PARTIALs, 1 real Kconfig orphan (SS_LITE_MOD_SX1262: consumer
  lands with S-03-011; COPROC_NONE benign choice-default), 1 phantom "pin-map
  CI test" reference, 0 dangling Deps. All gaps now story-owned: S-03-034..040
  (sequencer, buzzer, IMU, storage, USB CDC, watchdog-reconcile[T1], RNG
  base), S-08-017 (Lite ATECC), S-02-024 (pinmap checker), S-02-025
  (contract-audit gate — makes this defect class machine-caught). Watchdog
  finding is notable: S-02-009 shipped a DIVERGENT surface (ss_task_wdt_*)
  beside the frozen ss_hal_watchdog.h (ss_wdt_*) — reconciliation is T1.
- S-03-033 (2026-07-08): glue calling ss_hal accessors must #include
  "ss_hal.h" (the sub-headers don't declare has_cap) — implicit-declaration
  is -Werror on target. Recurring shape: IDF glue is only adjudicated by the
  3-board CI (host tests cover cores only); expect first-CI-run fixes on
  glue-heavy stories until S-02-025's contract-audit + a future glue
  host-mock land.
- S-03-009 (2026-07-08): ss_audio component (mic half of ss_hal_audio.h) built
  core/glue split like ss_muxctl. INMP441 24-in-32 -> 16-bit PCM = slot_bit_width
  32 + data_bit_width 16 (peripheral keeps the 16 MSBs, no post-processing).
  ss_mic_open acquires SS_MUX_MODE_MIC as SS_MUX_OWNER_AUDIO_MIC (25 ms cap,
  releases on every failure path). Speaker/buzzer symbols in the same contract
  are deliberately unimplemented (S-03-010). On-hardware SNR + capture-start
  latency + jitter pending a bench board -> parks at IN_REVIEW (like S-03-004/006).
- S-03-010 (2026-07-08): ss_spk (speaker half of ss_hal_audio.h) added to the
  ss_audio component, same core/glue split. Class-D amp (MAX98357A-class) SD =
  active-high enable on SS_SPK_PIN_MUTE (GPIO21). Pop-free is a pure decision:
  ss_spk_core open_seq = {I2S_ENABLE, SETTLE(5ms), AMP_ENABLE}, close_seq =
  {AMP_DISABLE, SETTLE, I2S_DISABLE} (clocks before amp / amp before clocks).
  No gain register -> volume is a software Q15 multiply in ss_spk_write. The
  speaker takes NO mux (dedicated I2S pins) — that absence IS the sidetone
  guarantee (concurrent with a mic that holds the mux). Parks at IN_REVIEW.
- audit (2026-07-08): the ss_audio host Makefile harness (ss_audio_core +
  ss_spk_core) is run by NO CI workflow — host-tests.yml paths cover only
  ss_log + firmware/test/host. Follow-up story proposed to wire both audio
  cores into the CI host-test path.
- S-02-025 (2026-07-08): `tools/contract-audit.py` is now a governance gate (in
  `make audit` + `weekly-audit.yml`): every function declared in
  `ss_hal*.h` must be implemented in-tree (column-0 def in `firmware/**/*.c`) or
  listed in `tools/contract-ownership.txt` (`funcname|header = STORY-ID`). New
  HAL contract functions MUST land implemented or with an ownership entry, else
  the audit fails. The scan covers the umbrella `ss_hal.h` too — that caught
  `ss_hal_init`/`ss_hal_shutdown` still unimplemented (owned by S-03-034, DRAFT).
- S-02-025/S-03-010 (2026-07-08): post-batch review caught (a) contract-audit
  was wired weekly-only, not per-push — its own AC required docs-lint; now
  wired + added to the verify skill; (b) the ss_audio make harness ran in NO
  CI workflow and the speaker worker couldn't execute it locally (gcc/make
  sandbox-denied — now allowlisted; harness executed by the advisor: 44/44).
  Standing rule reinforced: every new test harness lands IN a CI workflow in
  the same commit.
- S-03-013 (2026-07-08): LBT + EU868 duty-cycle guard landed as a pure,
  host-tested policy core `ss_lora/ss_lora_lbt_core.[ch]` (no HAL/IDF dep,
  self-contained on stdint/stdbool/stddef). Contract facts the LoRa driver
  (S-03-011) must honour when it consumes this core: (a) fail-safe decision
  enum — `SS_LORA_TX_ERR == 0`, so any NULL/unknown-band/uninitialised path
  means "do not transmit"; (b) the driver feeds a channel RSSI sample +
  threshold into `ss_lora_tx_evaluate`, backs off per `ss_lora_lbt_backoff_ms`
  (10 ms base, ×2, cap 320 ms, give up after 5) on `DEFER_BUSY`, drops+logs on
  `BLOCK_DUTY` (C-08), and calls `ss_lora_duty_record` AFTER a successful TX
  with the real airtime (`ss_lora_lbt_toa_ms`); (c) SOS override is bounded to
  `SS_LORA_SOS_OVERRIDE_MAX_MS` (5 s) of airtime per rolling hour per sub-band
  and LBT still defers SOS (collision avoidance). EU868 sub-band duty table is
  from ETSI EN 300 220. KNOWN LIMITATION → follow-up: duty accounting is
  in-RAM only (a reboot resets the 1 h window); persisting it across reboot is
  deliberately out of scope here (persistence is an escalation trigger) and
  should be filed as its own story if strict cross-reboot compliance is needed.
- S-03-014 (2026-07-08): Wi-Fi STA landed as a new `ss_wifi` component
  (pure host-tested core `ss_wifi_sta_core` + esp_wifi glue `ss_wifi.c`
  implementing the frozen `ss_hal_radio_wifi.h` lifecycle). Auth-mode policy:
  the enum ordinal is NOT the security order — WPA2/WPA3 mixed-transition ranks
  EQUAL to WPA3-SAE (a WPA3-capable STA negotiates SAE against a transition AP),
  so `ss_wifi_ap_acceptable` compares an internal `authmode_rank`, not raw enum
  values; `SS_WIFI_AUTH_UNKNOWN` and any out-of-enum cast always reject
  (fail-safe, never OPEN). KNOWN LIMITATION → follow-up: the glue's disconnect
  handler blocks the default event-loop task on `vTaskDelay(backoff)` — fine for
  the first cut but should move to a timer/reconnect task before the HIL rack;
  on-target WPA2/WPA3 association ACs are unverified until the Wi-Fi HIL rack
  (EPIC-03 exit criterion 2), so the story parks at IN_REVIEW.
- S-03-015 (2026-07-08): T1? flag CONFIRMED T1 — the AC's "credential
  handoff" over an open soft-AP is an onboarding-security surface (doc 05);
  runs interactive t1-pipeline. Pattern: keyword flags resolve on the AC
  text, not the title. Follow-ups filed: S-03-041 (duty persistence
  compliance determination), S-03-042 (reconnect off the event loop).
- S-03-015 (2026-07-08): landed as `ss_wifi_prov_core` (pure session core) +
  `ss_wifi_prov.c` (AP/DNS/HTTP portal glue). The T1 deliverable was the
  missing contract: doc 05 §10.3 only covered BLE/DPP provisioning, so the
  standalone soft-AP path got its own normative bullet (never-open AP,
  per-session TRNG passphrase on the display, bounded session, one-shot
  zeroized handoff, no at-rest persistence until FS_key). Two-phase AP
  bring-up because the S3 TRNG is only guaranteed with RF up: bootstrap
  passphrase -> start AP -> regenerate real secrets -> reconfigure. Credential
  persistence deliberately NOT implemented (doc 05 "never plaintext" vs no
  FS_key yet) — filed S-03-043. On-target ACs park at IN_REVIEW for the
  Wi-Fi HIL rack, same as S-03-014.
- S-03-015 (2026-07-08): T1 double-review earned its cost — both passes
  REWORK on round 1. Cross-review caught the killer: esp_wifi defaults to
  WIFI_STORAGE_FLASH, so every esp_wifi_set_config silently persisted the
  AP passphrase AND the handed-off home PSK to plaintext NVS
  (nvs.net80211) — exactly the leak the story exists to close. Fix:
  ss_wifi_init forces WIFI_STORAGE_RAM before any set_config (also
  de-persists S-03-014 STA creds, aligning with doc 05 until S-03-043).
  Pattern for the gotcha file: IDF "convenience" defaults can violate a
  security contract without any code in the diff being wrong. Round 2:
  both APPROVE-WITH-NITS; nits fixed (DNS task owns its fd; docs).
- S-03-039 (2026-07-08): watchdog HAL reconciliation (T1) — the shipped
  S-02-009 surface (ss_task_wdt_register/feed/unregister) wins and is adopted
  verbatim into the contract of record ss_hal_watchdog.h; the original
  ss_wdt_* sketch (never implemented, zero call sites) is deprecated in place
  as tombstone declarations + a DEPRECATIONS.md row — never silently
  dropped. Gotcha (caught by T1 review pass 1, same class as the S-03-015
  WIFI_STORAGE_FLASH find): IDF's default build spec appends
  -Wno-error=deprecated-declarations, so __attribute__((deprecated)) alone
  only WARNS — the claimed "any use fails the build" was false. Fix:
  tombstones carry GCC __attribute__((error)) too (flag-independent hard
  compile error on any call, pinned toolchain), deprecated kept for
  clang-based tooling. Decisive
  argument: ss_wdt_init(timeout_ms) was a policy-weakening knob; the TWDT
  deadline/panic-on-expiry are build-time constants (sdkconfig.defaults,
  S-02-009) no runtime code may loosen, so the canonical contract has no
  init entry point at all. Mechanical insight: contract-audit.py only scans
  firmware/**/*.c, so the wrappers moved (unchanged) from ss_tasks.cpp to
  ss_wdt.c — the header flips from owned-gap to impl and the ownership
  wildcard became four per-function tombstone entries. Zero call-site churn
  (ss_compass already used the winner).
- S-03-016 (2026-07-08): T1? flag CONFIRMED T1 — AC exposes GATT **pairing +
  provisioning** services; BLE pairing-mode policy (Secure Connections vs
  Just Works, MITM) and the provisioning characteristic are the same
  onboarding-security surface as S-03-015's soft-AP handoff. Interactive
  t1-pipeline; pairs naturally with S-03-017 in one Fable session.
- S-03-017 (2026-07-08): T1? flag CONFIRMED T1 — LTKs at rest (encrypted NVS
  only), bond wipe on re-pair. Keys-at-rest is a doc 10 §2 T1 domain; no
  judgment call needed.
- S-03-012 (2026-07-08): T1? flag resolved DOWN to T3 — 'provisioning' matched
  incidentally ("region selected at provisioning locks the table"); the story
  consumes the region choice, doesn't carry credentials. PA tables are
  well-specified (LoRaWAN regional params) + sweep-verified.
- S-03-022 (2026-07-08): T1? flag resolved DOWN to T2 — HAL conformance
  vectors, not crypto/wire vector design. Vector-set shape via t2-designer,
  build routine. First recorded downward resolutions: the flag system is
  bidirectional — the AC text decides, in both directions.
- S-03-012 (2026-07-08): PA-table policy landed as pure host-tested core
  `ss_lora_pa_core` (no HAL/IDF). `ss_lora_region_t` {UNKNOWN=0, US915, EU868,
  AU915, AS923} is now the SINGLE source of truth for the region enum — the
  SX1262 driver (S-03-011) MUST consume it, not redefine one. Regulatory max
  EIRP is pinned (US915/AU915 30 dBm, EU868/AS923 16 dBm) and clamped further
  to the SX1262 conducted ceiling SS_LORA_PA_HW_MAX_DBM=22; clamp never
  exceeds min(region, hw) and never drops below SS_LORA_PA_SAFE_MIN_DBM=2.
  Region latch is one-shot in-RAM only — persistence to sealed NVS is S-03-043.
  On-hardware RF sweep (spectrum-analyzer) parks with S-03-011 + HIL rack.
- S-03-019 (2026-07-08): LittleFS user FS lives in a NEW `ss_storage` component
  (NOT ss_hal, which is a T1 stop-path) implementing `ss_hal_storage.h`. It
  mounts joltwallet/littlefs (pinned ^1.22.2) on the EXISTING frozen "storage"
  partition BY LABEL at base path "/user" — the data/spiffs subtype is kept
  as-is, so no partitions.csv change / RFC-0003 trigger. Self-heal =
  mount→(on fail)format→remount, decided by the pure host-tested core
  `ss_storage_fs_core` (bounded: format at most once). Wear stats surface via
  `ss_storage_log_stats()` in the ss_diag 30 s health loop.
- S-03-019 (2026-07-08): user FS is PLAINTEXT for now — at-rest encryption is
  deferred to EPIC-08 flash-encryption (TODO markers in ss_storage). The
  power-cut torture AC + real-hardware mount need the S-02-015 on-target Unity
  rig, so S-03-019 lands IN_REVIEW with those as the blocking hardware gate.
- S-03-019 (2026-07-08): FSM driver convention is PERFORM-then-advance —
  perform the first step (always MOUNT), then call ss_fs_heal_next(last,res)
  for the next. Priming the core with (MOUNT, OK) instead makes it report
  DONE_OK without ever mounting; any future consumer of a *_heal_next FSM
  must drive it this way.
- ss_hal_storage.h (2026-07-08): comment-only honesty fix — `SS_STORAGE_INTERNAL_FS`
  line comment changed from "encrypted user FS" to "user data FS (at-rest
  encryption deferred to EPIC-08)". No ABI/wire/signature/type change; this is a
  pure documentation correction removing an aspirational security claim that
  S-03-019 landed the plaintext implementation of. Does NOT constitute a T1
  contract change under doc 10 §8.3 (no declaration modified) — flagged
  explicitly so the record is unambiguous.
- S-03-022 (2026-07-08): HAL conformance vectors live in a NEW pure-C core
  `firmware/components/ss_hal/test/conformance/` (ss_hal_conformance_core.c =
  runner + frozen diff formatter; ss_hal_conformance_vectors.c = builtin
  static-const tables, 19 vectors over power/audio/LoRa/Wi-Fi/BLE). The core
  never calls a HAL fn — it drives an env vtable (`ss_conf_env_t`: reset/exec/
  emit), so the IDENTICAL core+vector objects run under host gtest today (mock
  env, S-02-014 harness) and on-target Unity later (real-driver adapter,
  S-02-015 — deferred, no board). New drivers are graded against these vectors:
  the state/aux word encodings + `SS_CONF_RET_*` numerics are FROZEN in the
  header. Vectors are the tie-breaker of record where a HAL header is silent on
  error behavior (NULL req-ptr → 0x102 INVALID_ARG, use-before-init → 0x103
  INVALID_STATE) — relaxing one is a header change first, vector second.
- S-03-022 (2026-07-08): the diff grammar is byte-for-byte frozen and the host
  test asserts it with LITERAL strings: `CONF-FAIL dom=<d> vec=<n> step=<N>
  op=<OP> field=<ret|state|aux|exec> expected=0x<8hex> actual=0x<8hex>`. Any
  future formatter edit that drifts this fails CI. Placed under ss_hal/** (a T1
  stop-path) but is test-only, consumes frozen headers, modifies NO
  include/*.h, and holds NO keys/pairing/wire bytes (BLE pairing/LTK stays T1,
  S-03-016/017) — consistent with the recorded downward T2 resolution.
- S-03-022 (2026-07-08): GOTCHA — the host `ss_units` lib does NOT apply
  `-Wconversion/-Wshadow/-Werror`, and these conformance .c files are in NO
  component SRCS (S-02-015 owns target wiring), so `make lite` never compiles
  them and ctest-green alone is not sufficient. Strict-`gcc -c` the pure cores
  separately when touching them.
- S-03-023 (2026-07-08): Lite HIL rack test-plan authored at
  `docs/dev/HIL_TEST_PLAN_LITE.md` — topology, fixtures, exit-criteria×domain
  matrix, CI (self-hosted `[hil, lite]` runner, JUnit+artifact reporting). On-
  target conformance reuses `ss_hal_conformance_core` via the S-02-015 adapter;
  actual HIL workflow wiring + execution defer to the D-0013 fleet (VA-03).
- S-03-044 / S-03-045 (2026-07-08): filed the two HIL follow-ups the S-03-023
  worker discovered: S-03-044 wires the `.github/workflows/hil-lite.yml`
  self-hosted runner + JUnit plumbing; S-03-045 executes the matrix on the
  D-0013 fleet and fills `HIL_TEST_PLAN_LITE.md` §4.1 results. Rule of thumb
  reinforced: a plan doc without an execution owner is orphaned — every plan
  story earns at least one execution story before the workers close.
- ss_storage.c (2026-07-08): comment-only TODO owner correction — line 125
  pointed to S-03-023 (HIL rack test-plan) but the correct owner of NVS/SD/
  MODELS storage-kind aggregation is S-03-018 (Flash partition layout
  reconciliation), whose AC explicitly names "logs/FS mapping onto storage
  (LittleFS, S-03-019)" as scope. Worker ID transposition (018↔023); caught
  during the follow-up filing sweep. No code change.
- S-03-024 (2026-07-08): `docs/dev/POWER_BUDGET_LITE.md` is the roll-up Lite
  power budget proving NF-PWR-01 — per-`ss_hal_power.h`-state table + RX-idle
  (≤25 mA) + per-bearer TX peaks + a symbolic duty-cycle standby-hours model
  (≥24 h). Per-bearer TX peaks are DATASHEET TYPICALS (SX1262 ~118 mA @ +22 dBm,
  ESP32-S3 Wi-Fi ~355 mA 802.11b, ESP32-S3 BLE ~30 mA @ 0 dBm), NOT measured —
  actual draw is set by firmware TX-power / the region PA table (S-03-012). The
  sleep ≤0.5 mA procedure is NOT duplicated (lives in BENCH_POWER_LITE.md,
  S-03-003) and the PPK2 series fixture is reused from HIL_TEST_PLAN_LITE.md
  (S-03-023). Measured columns + numeric standby fill park at IN_REVIEW on the
  same D-0013 hardware session as S-03-003/BENCH_POWER_LITE.
- S-03-025 (2026-07-08): `docs/dev/COEX_STRESS_LITE.md` is the Wi-Fi/BLE
  coexistence stress plan. Lite's ESP32-S3 shares ONE 2.4 GHz antenna, so the
  two bearers time-share the RF front end via ESP-IDF software coex
  (`CONFIG_ESP_COEX_SW_COEXIST_ENABLE=y` + coex preference: balance /
  Wi-Fi-priority / BLE-priority). The soak reuses the HIL rack's Wi-Fi AP +
  BLE central peer (S-03-023) — no second rack. Thresholds are DEFINED targets
  (BLE disconnects 0, notif loss ≤0.5%, Wi-Fi TCP floor ≥10 Mbps, UDP loss
  ≤1%, bidir degradation ≤20%); measured/pass columns fill at D-0013, so the
  story parks at IN_REVIEW. Closes EPIC-03 risk R03-02.
- S-03-026 (2026-07-09): stop-and-escalate CONFIRMED VALID (doc 10 §8 —
  surprise contract contradiction). Frozen `ss_hal_radio_halow.h` prose says
  "via Morse Micro MM8108 … absent on Lite", but ratified Lite hardware
  (C-01 §3) carries an Elecrow HaLow module on the wireless-header SPI.
  The API surface (init/config/start/stop/tx/rx) is largely silicon-neutral;
  the stale parts are the prose + MM8108-shaped comments (MCS 0..10,
  BW ≤16 MHz). Resolution: filed S-03-046 (T1, t1-pipeline) to reconcile
  the header; S-03-026 → BLOCKED on it. Elecrow part/stack stays TBD-D-0013 —
  the reconciliation must not guess silicon ranges. Pattern: headers written
  before the hardware lock are contract-drift candidates; the T3 escalation
  path caught this exactly as designed.
- S-03-027 (2026-07-09): GNSS driven behind `hal_gnss` (`firmware/components/ss_gnss/`):
  pure host-tested `ss_gnss_core.[ch]` (NMEA-0183 checksum/coord/RMC-GGA parse,
  byte-identical to the old `ss_uart_engine` logic) + IDF glue `ss_gnss.c` owning
  UART1 + pump + fix. `ss_uart_engine` GNSS API now forwards to `ss_gnss` (main.cpp
  unchanged). Future GNSS consumers use `ss_hal_gnss.h`/`ss_gnss_*`, NOT the engine.
- firmware/components (2026-07-09): component-owned pump tasks use a RAW
  `xTaskCreate` with a documented numeric priority because `ss_tasks.h`
  (`ss_task_create`/`SS_PRIO_*`) is main-only and unreachable from a component
  (precedent: `ss_input/ss_touch.c` prio 10; now `ss_gnss` prio 12 == SS_PRIO_COMMS).
  This sidesteps the ss_tasks priority-ceiling convention — a follow-up could expose
  a component-safe task shim. S-03-028/S-03-029 will hit the same when they land.
- S-03-028 (2026-07-09): compass/mag driven behind the `ss_hal_imu.h` mag path
  (`firmware/components/ss_compass/`): pure host-tested `ss_compass_core.[ch]`
  (HMC5883L X→Z→Y decode + `-4096` overflow reject, tilt-comp/flat-hold heading,
  `ss_compass_src_t` now owned here) + IDF glue `ss_compass.c` implementing
  `ss_imu_read/heading_deg/sleep` (I²C0 @0x1E mag + @0x68 accel, gated on
  `SS_CAP_MAGNETOMETER`/`SS_CAP_IMU`, on-demand reads — no pump task, unlike
  `ss_gnss`). `main/ss_compass.cpp` now delegates its math to the shared core.
  Two open items for whoever wires this into boot: (1) both the new `ss_imu_init`
  glue and the legacy `main/ss_compass_start()` thread call `i2c_new_master_bus`
  on I²C0 (shared with GT911 touch) — bus ownership must be reconciled before
  enabling both; (2) exact Elecrow part is TBD-D-0013: doc 01 documents HMC5883L
  @0x1E but a QMC5883L variant sits @0x0D and changes the decode — verify at
  attachment and update doc 01 + pin map FIRST on any deviation.
- IN_REVIEW audit (2026-07-09): 22 stories parked. Semantics clarified and
  now enforced: IN_REVIEW = merged + tier-review-complete, AC *evidence*
  pending — it is NOT a pending-code-review status. Audit found 5 stories
  with no recorded park reason (S-02-019, S-03-009, S-03-013, S-03-019,
  S-03-025) — all fixed; gen-stories-index now REJECTS IN_REVIEW/BLOCKED
  stories without a park/blocker reason in Deps. New doc of record:
  docs/dev/EVIDENCE_RUNSHEET.md — every parked clause grouped by the
  fixture that discharges it (A: software-now, B: any Lite board,
  C: D-0013 fleet/rack, D: later-epic-gated) + the binding exit rule
  (flip only on linked evidence; never "probably fine"). Verified
  S-03-015's T1 double review ran (commit d13cf6e body carries verdicts).
- D-0020 (2026-07-09): Omega/Alpha hardware-truth alignment. Omega v1.0 =
  PCB release v69 (signed off 2026-07-08, TVF 69/69, SHA 054eaa8b) — P4 SoM
  + C6 bridge + MM8108 HaLow; NO LoRa/cellular/satellite/baro/SE/supercap
  and NO expansion interface (respin required for any of them). Alpha v152
  is NOT release-verified (v14 "DO NOT FABRICATE"; v15 unverified) — do not
  elaborate EPIC-04 against it yet. Actions: doc of record
  docs/dev/OMEGA_HW_BASELINE.md; 16 EPIC-05 ghostware stories BLOCKED
  (capability preserved for rev-2); S-05-011 retargeted MMC5983MA→BMM350;
  S-05-012 part confirmed (12× SK6805); additive D-0020 callouts in doc 00
  §1.2/§11-Ph4, doc 08 bearer table, PRD F-BR-01/06; omega board_config
  narration fixed. Covenant note: docs 00/08 anchors are mutable
  (existence+SPDX only) — content callouts are sanctioned. Pattern repeats
  EPIC-03: plans written before a hardware lock are drift candidates; the
  PCB release package is the only alignment source.
- S-21-030 (2026-07-13): TPI (telephony provider interface) contract frozen at
  `cloud/telephony-bridge/TPI_CONTRACT.md` — the single abstraction all
  telephony tiers target (D-0025, doc 12). Sequencing fact for future stories:
  `cloud/` service dirs and `sdk/` are unscaffolded (empty, no build/CI), so
  every telephony *implementation* story (S-21-027/031/032, S-17-022,
  S-14-020/021, S-18-019) is blocked until a cloud-services bootstrap + a cloud
  CI lane + a CPaaS sandbox exist. Contract/spec work can proceed regardless.
- S-05-042 (2026-07-18): `elecrow5` schematic validated (D-0027) — display is
  **16-bit parallel RGB565 (esp_lcd RGB path), NOT MIPI-DSI**; on-panel touch on
  I²C1, controller UNVERIFIED (likely GT911). Corrects the D-0026 §(2) web-spec
  assumption before it reached driver work.
- S-05-044 (2026-07-18): C6↔P4 bus **CONFIRMED = SDIO 4-bit** (GPIO53/54/52/51/
  50/49; esp_hosted/esp_wifi_remote) — resolves the D-0026 §7 SDIO-vs-SPI open item.
- S-05-048 (2026-07-18): **STC8H1K08 co-MCU (U14) on I²C1** gates backlight-EN,
  SPI/UART mux select, touch/camera reset, audio-shutdown, charge LEDs, and VBAT/
  charge sense — new bring-up story. HaLow/LoRa module slot = SPI2 behind an
  SGM3005 mux (S1/STC8).
- S-05-045 (2026-07-18): **no on-board fuel gauge** (battery via STC8 ADC), **no
  on-board GNSS/mag**; camera is **external-only** (MIPI-CSI header, no sensor);
  on-board audio (NS4168 ×2 + PDM mic) always present. Provenance: `vendor/elecrow/`.
- S-05-042 (2026-07-18): touch controller **CONFIRMED = GT911 (Goodix) @ 0x5D/0x14**
  (strap-selected) from the Elecrow `Lesson05` example driver + `gt911_for_crowpanel`
  — upgrades the D-0027 "likely GT911, UNVERIFIED"; residual = which strap on a
  physical unit (I²C scan at bring-up). The Eagle schematic (`.sch/.brd/.pdf` V1.0)
  is bundled inside `repo-crowpanel-5in-esp32p4.zip` under `Eagle_SCH&PCB/1.0/`.
- S-05-041 (2026-07-18): **PSRAM CONFIRMED = 32 MB in-package 16-line HEX/OPI,
  1.8 V ~200 MHz** (part ESP32-P4NRW32 + repo `sdkconfig` `SPIRAM_MODE_HEX`/`SPEED_200M`);
  flash = 16 MB W25Q128JVSIQ confirmed.
- S-05-047 (2026-07-18): module-slot **LoRa sub-variant CONFIRMED = SX1262** —
  pinout NSS30/BUSY29/IRQ31/NRST32/SCK26/MISO47/MOSI48, 3.3 V (product page +
  wiki BSP `RADIO_GPIO_*`); camera sensor **LIKELY SC2336** (2 MP, DS gated).
