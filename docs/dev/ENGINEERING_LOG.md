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
