# SS-SP — Seekie-Speakie Smart Pager

> Sovereign, mesh-native, off-grid smart pager platform.
> Wi-Fi HaLow + LoRa + BLE + Reticulum. Voice, text, presence, and Seekie compass direction-finding.
> Universal UI (rect / square / round / e-ink / headless). Apache-2.0 firmware. Enterprise-ready.

**This repository is the monorepo for the entire SS-SP program**: firmware (Lite / Alpha / Omega), companion apps, cloud, SDKs, protocol specs, and governance.

---

## Product tiers

| Tier | MCU | Radio | Display | Purpose |
|---|---|---|---|---|
| **Lite (v1 — first build)** | ESP32-S3 (CrowPanel Advance 3.5" HMI) | LoRa SX1262, Wi-Fi 4, BLE 5 | 3.5" IPS 480×320 | Dev kit / entry unit |
| **Alpha 1.0** | ESP32-P4 + ESP32-C6 | HaLow (MM8108 + SKY66423 1 W FEM), LoRa optional | 2.4" IPS 320×240 + 12-LED bezel | Flagship production pager |
| **Omega (next)** | TBD (RISC-V + Linux SoM candidate) | HaLow + LoRa + Cellular + SatCom | Larger IPS / OLED | Enterprise / heavy-duty |

## Founding documents

Read these in order:

1. [`00_MASTER_SOFTWARE_PLAN.md`](./00_MASTER_SOFTWARE_PLAN.md) — program-wide software plan and roadmap.
2. [`01_SS-SP_LITE_HARDWARE_REFERENCE.md`](./01_SS-SP_LITE_HARDWARE_REFERENCE.md) — Lite hardware, source-verified pin map, bring-up.
3. [`02_PROTOCOL_STACK.md`](./02_PROTOCOL_STACK.md) — wire format, LXMF↔Meshtastic bridging, RNS-HaLow.
4. [`03_UI_LAYOUT_SPEC.md`](./03_UI_LAYOUT_SPEC.md) — universal, aspect-ratio-agnostic UI framework.
5. [`04_LICENSING_AND_FORK_STRATEGY.md`](./04_LICENSING_AND_FORK_STRATEGY.md) — **binding** commercial-sale licensing decisions.
6. [`05_SECURITY_MODEL.md`](./05_SECURITY_MODEL.md) — threat model, crypto, secure boot, OTA, provisioning, sandbox.
7. [`06_GOVERNANCE.md`](./06_GOVERNANCE.md) — governance, RFC process, foundation transition.

## Repo layout

```
firmware/            ESP-IDF firmware (Lite / Alpha / Omega)
  boards/lite/       Elecrow CrowPanel Advance 3.5" HMI board port
  boards/alpha/      ESP32-P4 + MM8108 board port
  boards/omega/      (reserved)
  components/ss_hal/ HAL headers + per-board implementations
  components/ss_ui/  Universal UI framework on top of LVGL 9.x
  components/ss_net/ Bearer manager, path selection
  components/ss_rns/ Reticulum transport (see licensing notes)
  components/ss_lxmf/ LXMF messaging
  components/ss_meshtastic_compat/  Clean-room Meshtastic wire compat
  components/ss_crypto/  X25519, Ed25519, XChaCha20, ratchets
  components/ss_ota/  OTA manifest, dual-sig verify
  components/ss_provisioning/  Factory & fleet enrollment
  components/ss_ai/  On-device STT/TTS/LLM glue
  components/ss_plugin/  WASM/ELF plugin sandbox
  components/ss_audio/  I2S in/out, codecs
  components/ss_seekie/  Direction-finding compass
  components/ss_power/  Battery/PMIC/thermal state
  components/ss_storage/  Encrypted FS, SD
  main/              app entrypoint
  bootloader/        SS-SP-signed second-stage bootloader
protocol/            Wire specs + our own .proto/CBOR files
  ss/                Native SS-Ext CBOR schemas
  foreign/meshtastic/  Clean-room wire compat spec + our .proto
  foreign/rns/       RNS interoperability notes
  foreign/lxmf/      LXMF envelope
  testvectors/       Hex+JSON test corpora
sdk/                 C / Rust / Python / Dart / TypeScript SDKs
companion/           Mobile (Flutter), desktop (Tauri), web (React)
cloud/               Fleet console, relay, provisioning service, plugin registry
tools/               Simulator, OTA signer, provisioning line, fuzzer
infra/               Docker, k8s, terraform, ansible
ci/                  Reusable workflows, scripts, linters
docs/                User + dev + protocol + wire + security + compliance
rfcs/                Formal RFC documents
governance/          SC decisions, trademarks, open-assurance
models/              On-device model catalog + provenance
vendor/              Vendor SDKs (Morse Micro, etc.) under NDA
assets/              Fonts, icons, sounds, logos
```

## Building — Lite (first target)

Requires ESP-IDF v5.3+ and Python 3.10+. See [`docs/dev/BUILDING.md`](./docs/dev/BUILDING.md) once produced.

```bash
# From repo root
idf.py -B build/lite -DBOARD=lite build
idf.py -B build/lite -p /dev/ttyUSB0 flash monitor
```

## Contributing

- Read [`CONTRIBUTING.md`](./CONTRIBUTING.md).
- Sign the DCO (`git commit -s`) and CLA on first PR.
- Follow the [`CODE_OF_CONDUCT.md`](./CODE_OF_CONDUCT.md).
- For new wire-format changes: open an [`rfcs/`](./rfcs/) RFC.

## Security

- See [`SECURITY.md`](./SECURITY.md) for coordinated disclosure.
- **security@ss-sp.org** (PGP key in `SECURITY.md`).

## Licensing summary

- **Firmware, HAL, SDK, sample apps**: Apache-2.0.
- **Protocol spec documents**: CC-BY 4.0.
- **Fonts**: OFL-1.1.
- **Cloud fleet console**: BSL 1.1 (converts to Apache-2.0) *or* proprietary — see `04_LICENSING_AND_FORK_STRATEGY.md`.
- **We do NOT fork Meshtastic firmware or apps** (GPL-3.0 viral). We interoperate via clean-room wire compat.
- Reticulum and LXMF are used under the Reticulum License; deployments must honor the "no-harm" and "no-AI-training" clauses. See `04_LICENSING_AND_FORK_STRATEGY.md` §4.

## Trademarks

"SS-SP", "Seekie-Speakie", and the shield/dot logo are trademarks of the SS-SP project vendor entity, transferred to the SS-SP foundation on Phase 2 governance transition. See [`docs/TRADEMARK.md`](./docs/TRADEMARK.md).

"Meshtastic" is a registered trademark of Meshtastic LLC. SS-SP is not affiliated with Meshtastic LLC; interoperability references are nominative fair use.

---

*"Sovereign communication, in every hand, on every network, in every language."*
