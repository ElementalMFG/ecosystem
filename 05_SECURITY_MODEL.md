<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 05 — SS-SP SECURITY MODEL

> **Scope.** Threat model, cryptographic primitives, key hierarchy, identity, secure boot & OTA chain, factory provisioning, plugin sandbox, privacy posture, incident response, and compliance for the entire SS-SP program (Lite, Alpha, Omega, OEM).
>
> **Guiding principle.** The SS-SP is a **sovereign communications device**: it must remain trustworthy even when the network is hostile, the vendor is offline, and the user is under duress. Every cryptographic decision below is chosen so that **loss of vendor infrastructure does not compromise deployed devices**, and **compromise of one device does not compromise its peers or its group**.
>
> **Non-goals.** This document does not attempt to defend against nation-state-grade side-channel attacks with unlimited physical access. It defends against realistic mission threats: theft, tampering, MITM, replay, jamming, imposter beacons, malware sideloading, supply-chain implants, and mass-scale traffic analysis.

---

## Table of contents

- [0. TL;DR](#0-tldr)
- [1. Threat Model](#1-threat-model)
- [2. Cryptographic Primitives (Approved Profile)](#2-cryptographic-primitives-approved-profile)
- [3. Identity & Key Hierarchy](#3-identity--key-hierarchy)
- [4. Messaging Encryption](#4-messaging-encryption)
- [5. Secure Boot & Flash Encryption](#5-secure-boot--flash-encryption)
- [6. OTA Update Chain](#6-ota-update-chain)
- [7. Factory Provisioning](#7-factory-provisioning)
- [8. User Authentication & Duress](#8-user-authentication--duress)
- [9. Runtime Sandboxing & Plugins](#9-runtime-sandboxing--plugins)
- [10. Network / Transport Security](#10-network--transport-security)
- [11. Storage Security & Data Retention](#11-storage-security--data-retention)
- [12. Privacy Posture](#12-privacy-posture)
- [13. Vulnerability Disclosure & Incident Response](#13-vulnerability-disclosure--incident-response)
- [14. Regulatory & Compliance Alignment](#14-regulatory--compliance-alignment)
- [15. Testing & Assurance](#15-testing--assurance)
- [16. Summary — Non-Negotiable Security Bindings](#16-summary--non-negotiable-security-bindings)

## 0. TL;DR

- **Identity primitives**: Ed25519 (sign) + X25519 (KEX), derived from a single 256-bit seed sealed in ESP32 eFuse (or ATECC608 secure element on Alpha/Omega).
- **1:1 messaging**: Signal-style Double Ratchet over LXMF, forward-secret and post-compromise-secure.
- **Group messaging**: MLS-inspired epoch-based ratchet with cheap sender-keys optimization for low bandwidth.
- **Voice**: SRTP-like AEAD (XChaCha20-Poly1305) with per-session keys and rekey every 60 s or 2^16 packets.
- **Secure boot**: ESP32 Secure Boot v2 (RSA-3072 or ECDSA-P256) + flash encryption (AES-256-XTS).
- **OTA**: dual-partition A/B, image signed by *two* independent keys (dev + release) with rollback protection via eFuse-monotonic counter.
- **Provisioning**: per-device unique key at factory, attested by a signed birth-certificate; no shared master keys ever leave HSM.
- **Duress**: PIN + duress-PIN; duress-PIN silently wipes ratchet state and emits a "compromise" beacon.
- **Plugin sandbox**: WASM or ELF-sandbox with capability tokens; no ambient authority.
- **Privacy**: message padding, cover traffic option, Tor-style onion routing on RNS, no telemetry off-device by default, no user data leaves device without explicit consent.
- **Compliance**: designed to be **GDPR-compliant, CCPA-compliant, HIPAA-adjacent, FIPS-140-3 alignable (with alternate crypto profile), CJIS-adjacent for LE deployments**, RED (EU), FCC Part 15/97, ISED, and **export-controlled to 5D002 mass-market self-classified**.

---

## 1. Threat Model

### 1.1 Assets (what we protect)
| # | Asset | Description | Impact if lost |
|---|---|---|---|
| A1 | Long-term identity key (Ed25519 + X25519 seed) | Device's on-air identity | Impersonation, message decryption |
| A2 | Ratchet / session state | Per-conversation forward-secret state | Retroactive message decryption |
| A3 | Group epoch keys | Group chat keys | Group traffic decryption until next epoch |
| A4 | User contacts & message history | Address book, plaintext archive on device | Privacy exposure, targeted attacks |
| A5 | Location history | GPS trail | Physical safety exposure |
| A6 | Voice recordings & mic access | Live audio surface | Surveillance |
| A7 | Firmware image | Executable code | Rootkit, network pivot |
| A8 | Configuration & fleet enrollment | Which fleet, which server | Command hijack |
| A9 | Vendor signing keys | OTA and provisioning | Fleet-wide compromise |
| A10 | Wire traffic metadata | Who talks to whom, when, from where | Social-graph analysis |

### 1.2 Adversaries (who attacks)
| # | Adversary | Capability | Typical goal |
|---|---|---|---|
| T1 | Casual snooper | Sniff Wi-Fi/BLE/LoRa within range | Read messages |
| T2 | Opportunistic thief | Physical device access, no lab | Read stored data, resell |
| T3 | Targeted attacker | Sniff + jam + spoof + limited chip prying | Impersonate, MITM specific users |
| T4 | Insider (support / OEM) | Access to provisioning tools, some signing power | Cover-hijack a device, plant persistent access |
| T5 | Malicious contributor | PR access to source | Backdoor via upstream |
| T6 | Supply-chain adversary | Component substitution, factory-line access | Persistent implant, mass compromise |
| T7 | Vendor compromise | Steal release-signing key | Push malicious OTA fleet-wide |
| T8 | State-level adversary | Full lab (SEM, glitching), traffic-analysis fleets, court-order | Selective decryption, unmasking |
| T9 | User-under-duress | Coerced legitimate user | Extract keys, produce message |
| T10 | Loss of vendor | Vendor bankruptcy / seizure | Devices must remain useful |

### 1.3 Defenses summary (mapped to threats)
| Defense | Threats mitigated |
|---|---|
| End-to-end AEAD over every bearer | T1, T3, T8 (bulk) |
| Forward secrecy (Double Ratchet, MLS-style) | T3, T8 (retroactive) |
| Post-compromise security (ratchet heals) | T3, T8 (ongoing) |
| Secure Boot v2 + flash encryption | T2, T6 |
| Signed OTA + rollback counter | T7, T5 |
| Dual-signer release key | T7 |
| eFuse-sealed per-device seed | T6, T8 |
| Duress-PIN wipe + compromise beacon | T9 |
| Plugin capability sandbox | T5 |
| No default telemetry, on-device processing | T4, T10 |
| CLA + license scan + code review | T5 |
| Trusted-manufacturing HSM ceremony | T4, T6 |
| RNS anonymous transport + padding | T1, T10 (traffic analysis) |
| Zeroization on tamper detect (Alpha/Omega) | T2, T3 |
| Escrowless design (vendor cannot decrypt) | T4, T7, T10 |

---

## 2. Cryptographic Primitives (Approved Profile)

The SS-SP program uses **exactly** these primitives. Adding another primitive requires a security-RFC. Removing one requires migration RFC.

| Purpose | Primitive | Rationale |
|---|---|---|
| Signature | **Ed25519** | Modern, fast on ESP32-S3/P4, no NIST-curve baggage |
| Signature (alt for FIPS profile) | ECDSA-P256 (SHA-256) | For FIPS-140-3-aligned builds; via mbedTLS |
| Key exchange | **X25519** | Companion to Ed25519 |
| Post-quantum wrapper (future) | **X25519 + ML-KEM-768** hybrid | Deployed as opt-in in v1.5; mandatory in v2 |
| Symmetric AEAD | **XChaCha20-Poly1305** | 24-byte nonces (safe under high volume), fast on ESP32 without AES hardware |
| Symmetric AEAD (alt) | AES-256-GCM (hardware-accelerated on ESP32-S3/P4) | For hardware acceleration path |
| Flash / OTA image encryption | **AES-256-XTS** (ESP32 Flash Encryption) | Vendor-provided |
| Hash | **SHA-256** (interop) + **BLAKE3** (internal) | Fast on RISC-V P4 |
| KDF | **HKDF-SHA-256** | Ubiquitous |
| Password KDF | **Argon2id** (m=64 MiB, t=3, p=1) | Duress PIN, passphrase |
| MAC (rare, prefer AEAD) | HMAC-SHA-256 | Legacy only |
| Random | **ESP32 HW RNG** (health-tested) mixed with **CTR-DRBG** on each boot | Belt-and-braces |
| Ratchet (1:1) | **Signal Double Ratchet** | Industry standard |
| Ratchet (group) | **MLS-simplified with sender keys** | Bandwidth-friendly variant of RFC 9420 |

All symmetric encryption is authenticated. Un-authenticated encryption is forbidden.

### 2.1 Deprecated / forbidden

- **MD5, SHA-1** — hash only for legacy interop verification, never as security primitive.
- **RC4, DES, 3DES** — forbidden.
- **AES-ECB** — forbidden.
- **CBC without HMAC-then-encrypt** — forbidden.
- **Curve25519 in raw form without domain separation** — forbidden; always use HKDF.
- **Home-rolled AEAD constructions** — forbidden.

---

## 3. Identity & Key Hierarchy

### 3.1 Device root secret
Each device holds a **256-bit device seed** `S_dev` provisioned at factory (see §7), stored in ESP32 eFuse block (write-once, read-protected) or in the ATECC608B secure element on Alpha/Omega.

From `S_dev`, we derive by HKDF-Expand-Label:

```
Sign_priv    = HKDF(S_dev, "ss/sign/ed25519/v1")
X25519_priv  = HKDF(S_dev, "ss/kex/x25519/v1")
FS_key       = HKDF(S_dev, "ss/storage/fs/aead/v1")      // filesystem encryption
Prov_seed    = HKDF(S_dev, "ss/prov/enroll/v1")           // enrollment authentication
LocalUI_KDF  = HKDF(S_dev, "ss/ui/duress-pin/v1")         // duress PIN slot
Ratchet_root = HKDF(S_dev, "ss/ratchet/root/v1")          // per-peer ratchet seed
```

The public identity broadcast over the air is:
```
node_id = SHA-256( Sign_pub || X25519_pub )[:16]   // 128-bit device address
```
Same identifier used by RNS-compat layer and by SS-Ext records.

### 3.2 Birth certificate (per-device)
Signed at factory by the **Provisioning CA** (kept in HSM):

```
BirthCert = {
  node_id, Sign_pub, X25519_pub,
  hw_model, hw_rev, mfg_lot, mfg_date,
  fleet_pubkey_prehash (optional, if pre-enrolled),
  regulatory_region,
  capability_set_hash
}
signed by ProvCA_priv
```

The device stores the birth certificate + the ProvCA public key in a read-only partition. Any peer can verify a device is genuine SS-SP hardware by checking this chain.

### 3.3 Fleet enrollment (enterprise)
Optional. A device can enroll with a Fleet Controller, which issues a **Fleet Certificate** authorizing the device to receive fleet configs, roster updates, and enterprise plugins. Fleet enrollment does **not** grant the fleet controller ability to decrypt user messages; it only grants fleet-config authority.

Enrollment ceremony:
1. Device generates a nonce, signs with `Sign_priv`.
2. Fleet controller verifies against BirthCert (proves hardware genuineness).
3. Fleet controller issues FleetCert `{ node_id, fleet_id, roles[], expiry }` signed by fleet CA.
4. Device stores FleetCert in encrypted config partition.
5. Revocation: fleet controller adds device to a signed CRL delivered via RNS; devices refuse to load fleet payloads for revoked nodes.

### 3.4 User identity vs device identity
A user may **rotate** logical identity ("persona") without re-provisioning hardware. The persona has its own Ed25519/X25519 pair derived from `HKDF(S_dev, "ss/persona/" || persona_index)`. Rotation increments persona_index and re-announces. Old persona is retained decryptable-only until user commands wipe.

### 3.5 Contact keys & fingerprints
Contacts stored as `{ user_display_name, persona_id (16B), persona_sign_pub, persona_x25519_pub, verified_at_epoch }`.
UI displays contacts' short fingerprints as **6 words from BIP-39 wordlist** (48-bit prefix of SHA-256 of concatenated pubkeys), enabling out-of-band verification.

---

## 4. Messaging Encryption

### 4.1 Direct message (1:1)
- **Double Ratchet** initialized via X3DH from static X25519 pubkeys + a one-time pre-key advertised in the roster.
- Each message: (nonce, ciphertext, MAC) via XChaCha20-Poly1305.
- Ratchet keys sized to fit in 32 bytes; header encrypted-then-authenticated.
- On loss of a chain-of-keys (e.g. long offline), sender initiates fresh X3DH.

### 4.2 Group message
- **Epoch-keyed sender keys**. Each epoch key `K_epoch` is derived by a Diffie-Hellman contribution from every current member (MLS-style tree, simplified to a flat tree for ≤16 members, full tree for larger).
- Each sender derives a per-sender ratchet chain from `K_epoch || sender_id`.
- Adding a member: current members contribute a new commit; epoch advances.
- Removing a member: same, and the removed member's contribution is replaced with a fresh random.
- Bandwidth: sending only requires a per-message header + AEAD; the DH commits happen only on membership change.

### 4.3 Broadcast (channel)
Two modes:
- **Signed-plaintext** channel: messages signed by sender, encrypted with a channel PSK. All members share PSK; forward secrecy not provided; used for "public" channels only.
- **Encrypted channel**: group scheme above with membership managed by channel admin.

### 4.4 Voice
- **PTT session**: on push, sender generates a fresh session key `K_voice = HKDF(K_epoch_or_pair, "voice" || session_id)`.
- Frames encrypted with XChaCha20-Poly1305, nonce = session_id || packet_counter.
- Session key expires after **60 s of silence** or **65 535 packets**, whichever first; new session negotiated on next PTT.
- Voice frames carry a **replay window** of 128 packets.

### 4.5 File transfer
- Chunked with a Merkle tree; each chunk AEAD-encrypted; root hash signed by sender.
- Receiver verifies each chunk on arrival, allowing resumable partial receipt.

### 4.6 Post-quantum readiness
- v1.0: X25519 only.
- v1.5: **hybrid X25519 + ML-KEM-768** as opt-in, negotiated in the LXMF handshake header. Ciphertext expands ~1 KB; acceptable on HaLow and BLE, negotiated off on LoRa unless enabled by policy.
- v2.0: hybrid mandatory for new sessions; legacy X25519-only sessions permitted only for pre-v2 peers, deprecated after v2 + 24 months.

---

## 5. Secure Boot & Flash Encryption

### 5.1 Chain (ESP32-S3 and ESP32-P4)
1. **ROM bootloader** (immutable, silicon-provided).
2. **Second-stage bootloader** (SS-SP-signed) — verified by ROM against public key hash burned into eFuse.
3. **Application image** — verified by second-stage bootloader; signature over the whole image.
4. **Application** — mounts encrypted filesystem partitions using `FS_key`.

Signatures use **Secure Boot v2 (RSA-3072-PSS)** on ESP32-S3, or **ECDSA-P256** where supported. Bootloader supports **≥3 pinned keys** for future rotation.

### 5.2 Flash encryption
- **AES-256-XTS** on all partitions except OTADATA and reserved.
- Key generated on device at first boot (ESP32 flash-encryption "release" mode); never leaves the chip.
- User-data partition additionally sealed with `FS_key` for defense-in-depth.

### 5.3 JTAG / UART download
- **eFuse `DIS_DOWNLOAD_MODE`**: set in factory for release units.
- **eFuse `HARD_DIS_JTAG`**: set.
- **UART bootloader stub**: disabled on release units.
- Development units carry an "unlocked" eFuse profile and are marked with a red boot banner and a scary "not for field use" splash.

### 5.4 Alpha / Omega hardware secure element (ATECC608B or equivalent)
- Long-term identity keys stored in secure element with `ReadKey` disabled.
- All sign/kex operations executed inside the secure element; ESP32 sees only opaque outputs.
- Fault-injection detection triggers zeroization of I2C-side cache; secure element zeroization by explicit CLI or duress event.

---

## 6. OTA Update Chain

### 6.1 Partition layout
- `bootloader`
- `partition_table`
- `nvs`
- `otadata`
- `app0` (A), `app1` (B) — dual-slot
- `factory` (recovery, minimal)
- `fs_user` (encrypted user data)
- `fs_models` (AI models, optional; separate partition permits large models without bloating app slots)
- `birthcert` (read-only)

### 6.2 Image signing
Every release image is signed by **two independent keys**:
- `RelKey_A`: held by the Release Engineering team (in vendor HSM).
- `RelKey_B`: held by the Security Officer (independent HSM, ceremony required).

Bootloader verifies **both** signatures. This means an attacker who steals only one signing key cannot ship a valid image.

Release process:
1. CI builds image → produces SHA-256 hash.
2. Build engineer initiates signing job → HSM signs with `RelKey_A`.
3. Independent Security Officer reviews change log, tests on canary devices, then triggers second HSM sign with `RelKey_B`.
4. Manifest published: `{ image_hash, signature_A, signature_B, min_version, max_version, changelog_url, published_at }` — itself signed with a **manifest key**.

### 6.3 Delivery
- **Bearers**: HTTPS from vendor CDN (default), local Wi-Fi peer share, USB-CDC, LoRa fragmented delivery for extreme-remote fleets.
- **Trust**: bearer is untrusted; the on-device verifier trusts only the signature chain.
- **Bandwidth**: images delta-compressed; delta size typically <20 % of full.

### 6.4 Rollback protection
- **Monotonic version counter** in eFuse (or in NVS with tamper-evident hash chain on units without eFuse counter).
- Bootloader refuses images with `version < counter`.
- After successful boot into new slot, counter advances.
- Recovery slot always accepts current version to allow field recovery.

### 6.5 A/B fail-safe
- New image installed to inactive slot; watchdog "commit" written only after 10 minutes of stable operation.
- If commit not written before reboot, bootloader reverts to previous slot.
- User can force-revert via 3-button chord at boot.

### 6.6 Chained updates for offline fleets
- Devices can act as **OTA carriers**: node A downloads image, verifies signatures, then re-broadcasts to nearby peers over RNS. Peers verify signatures themselves; the carrier is untrusted.
- Enables mass update from a single connected node in a disconnected fleet.

### 6.7 Model updates (AI)
- On-device models updated as separately-signed artifacts.
- Model manifest includes `model_id`, `training_data_provenance_url`, `hash`, `signature`.
- Model signatures use `RelKey_A` alone (single-key OK because models are not privileged code).

### 6.8 Third-party plugin updates
- Plugins signed by plugin developer key, cross-signed by SS-SP plugin registry key.
- User must explicitly opt-in to any plugin.

---

## 7. Factory Provisioning

Provisioning is the **highest-risk** manufacturing step. It happens **once per device** and defines that device's identity forever. It must be conducted at a trusted assembly site under HSM-mediated procedure.

### 7.1 Provisioning line hardware
- **Line HSM**: FIPS-140-2 Level 3 (e.g. YubiHSM 2, Thales Luna, AWS CloudHSM).
- **Line workstation**: hardened, offline, network-isolated except to HSM and DUTs.
- **Line audit log**: append-only, signed daily.
- **DUT interface**: USB-CDC or JTAG (JTAG disabled after provisioning).

### 7.2 Steps
1. DUT boots factory image with default bootloader keys.
2. DUT generates `S_dev` inside its own hardware RNG (256 bits, health-tested).
3. DUT derives `Sign_pub`, `X25519_pub`, sends them to line workstation over signed session.
4. Line workstation forwards `{node_id, Sign_pub, X25519_pub, hw_model, hw_rev, lot, date, region}` to HSM.
5. HSM signs birth certificate with `ProvCA_priv`.
6. Line workstation delivers BirthCert to DUT.
7. DUT writes:
    - `S_dev` to eFuse block (write-once).
    - Boot-verify key hash to eFuse.
    - Flash encryption enable + key.
    - BirthCert to `birthcert` partition.
    - `DIS_DOWNLOAD_MODE`, `HARD_DIS_JTAG`, etc. eFuses set.
8. DUT self-tests: verify Sign, X25519, RNG output.
9. DUT signs a "provisioning complete" attestation, sent back to line workstation for audit log.
10. Line workstation records device serial + node_id in provisioning ledger.

### 7.3 Anti-cloning
- HSM refuses to sign a birth certificate for a `node_id` it has seen before.
- Provisioning ledger is compared to production yield at end of shift; any mismatch flagged.
- Silicon `MAC` (48-bit factory MAC on WROOM-1) is captured and linked to `node_id` in the ledger for later forensic tracing.

### 7.4 Post-factory identity change
- User can trigger **"identity wipe"**: erase persona derivations, but device seed `S_dev` cannot be re-derived (eFuse). BirthCert remains — device is still recognized as genuine, but presents fresh persona keys.
- Full crypto-erase possible only by physical scrap.

### 7.5 Repair / RMA
- Repaired devices whose seed is still valid retain identity.
- Devices with damaged eFuse/SE are declared "identity-destroyed" and either scrapped or re-issued a new seed under an "RMA" flag recorded in the ledger.

---

## 8. User Authentication & Duress

### 8.1 Access model
- **Locked mode**: screen off; PIN/pattern/biometric (Alpha's optional fingerprint) required.
- **Unlocked mode**: full UI access; auto-lock timeout configurable (default 3 min).
- **Emergency mode**: SOS beacon and dial-out accessible from lock screen; nothing else.

### 8.2 PIN storage
- User PIN passed through **Argon2id** with per-device salt (from `S_dev`) into a **wrap key** that decrypts a **local wrap of the ratchet state and message archive** on the encrypted filesystem partition.
- No PIN stored plaintext or hashed-only anywhere.

### 8.3 Duress PIN
- User configures a second PIN. When entered, device:
  1. **Wipes** the archive and ratchet state (best-effort secure-erase — full erase impossible on NAND without whole-partition rewrite; we rewrite the wrap key immediately, then background-scrub archive blocks).
  2. **Emits a signed "duress" beacon** to configured emergency contacts (SS-Ext record `duress.beacon`, high priority).
  3. **Presents a fake "empty" UI** so coercer sees the device appears wiped.
  4. Optionally opens live-mic streaming to trusted contacts if the fleet configured it.

### 8.4 Rate limiting
- PIN attempts rate-limited (exponential backoff).
- After N failed attempts (default 10), device auto-wipes archive and ratchets.
- SOS remains available regardless.

### 8.5 Biometric (Alpha optional fingerprint)
- Biometric unlocks wrap-key, but PIN is still required at boot.
- Duress PIN always wins over biometric.

---

## 9. Runtime Sandboxing & Plugins

### 9.1 Firmware capability boundaries
Even trusted first-party firmware components run with **explicit capabilities**. The core kernel exposes a minimal HAL API; feature modules must **request** capabilities at load time:

```c
ss_cap_grant_t granted;
esp_err_t err = ss_caps_request(
    (const ss_cap_req_t[]) {
        { .cap = SS_CAP_LORA_TX,     .why = "chat.send"           },
        { .cap = SS_CAP_MIC,         .why = "ptt.record"          },
        { .cap = SS_CAP_STORAGE_R,   .why = "roster.load"         },
        { .cap = SS_CAP_STORAGE_W,   .why = "roster.save"         },
    },
    4, &granted, MODULE_ID_CHAT);
```

Any HAL call checks the caller's capability set.

### 9.2 Plugins
Third-party plugins run in a sandbox:
- **Preferred**: **WASM** via a minimal Wasm3 or WAMR runtime, capability-restricted host API.
- **Fallback**: signed ELF loaded into a MPU-isolated region on ESP32-S3 with a syscall shim.
- Plugins declare required capabilities in a manifest; user must approve on install.
- Plugins cannot access raw radio, cannot open arbitrary files, cannot call into other plugins.

### 9.3 Plugin signing
- Plugin developer signs with their key.
- SS-SP Plugin Registry counter-signs after review.
- User can *sideload* unsigned plugins only after enabling "developer mode," which puts a persistent warning banner on the UI.

### 9.4 Resource limits
- CPU quota per plugin (measured cycles).
- Memory ceiling.
- Storage quota.
- Radio TX budget (msgs / minute).
- Watchdog terminates plugins that hang.

---

## 10. Network / Transport Security

### 10.1 Bearer-agnostic
Every bearer (LoRa, HaLow, BLE, Wi-Fi 4, ESP-NOW, USB-CDC) delivers **already-encrypted** packets. Bearer security modes (WPA3, BLE LE Secure Connections) are additive, not primary.

### 10.2 RNS transport security
- RNS provides its own end-to-end security based on X25519. We **stack our LXMF+SS-Ext AEAD on top** rather than trust RNS's alone — defense in depth.
- RNS anonymous transport is opt-in; when enabled, uses onion-style path selection.

### 10.3 Wi-Fi provisioning
- Companion-app-driven provisioning uses **BLE + Wi-Fi Easy Connect (DPP)** where supported, falling back to a **PSK exchange over encrypted BLE GATT** using X25519 KEX and Argon2id-derived shared secret.
- **Never** stores Wi-Fi PSKs plaintext; PSKs sealed under `FS_key`.

### 10.4 Traffic-analysis defenses
- **Padding**: LXMF envelopes padded to nearest 64 B on HaLow/BLE, 32 B on LoRa (bandwidth-conscious).
- **Cover traffic (optional)**: idle chatter to obscure real send times; user-configurable, off by default (bandwidth cost).
- **Timing jitter**: outbound transmissions randomized within QoS-class latency budget.
- **RNS Announce interval jitter**: 90 s ± 30 s.

### 10.5 Rogue node detection
- Devices publish signed presence beacons.
- Peers track observed `node_id ↔ BirthCert` bindings.
- Duplicate `node_id` triggers an alert.
- Un-attested `node_id` from claimed-SS-SP device triggers an alert (someone claims to be SS-SP without a valid BirthCert).

---

## 11. Storage Security & Data Retention

### 11.1 On-device
- Message archive: encrypted with `FS_key`, wrap-encrypted with PIN-derived key.
- Media (voice memos, images): encrypted per-file with per-file DEKs, DEKs wrapped by `FS_key`.
- Location history: same treatment; user can set retention (default 30 days rolling).
- Contacts: encrypted; contact fingerprints displayed as 6-word BIP-39 phrases.
- Ratchet state: encrypted; overwrite-in-place on rekey to reduce forensic residue.

### 11.2 Companion apps
- On mobile: use platform keychain (iOS Keychain, Android Keystore) to wrap a device-app-pair symmetric key.
- On desktop: use OS credential store; encrypted SQLite for archive.

### 11.3 Cloud (optional enterprise fleet)
- Fleet controller receives only what fleet policy specifies: telemetry, presence, roster.
- Message contents **never** reach fleet controller unless explicit fleet policy enables a fleet-key group (enterprise "records retention" mode), and even then the group is a normal LXMF group with the fleet controller as a member — no privileged decrypt.
- Data at rest: AES-256-GCM with keys in cloud HSM.
- Data in transit: mTLS 1.3.
- Retention: configurable per-tenant, default 90 days.
- Right-to-erasure: implementable within 30 days (GDPR).

---

## 12. Privacy Posture

- **No default telemetry.** First-boot wizard asks the user to opt-in to anonymous crash reports and usage metrics. Off by default.
- **On-device processing** for STT, TTS, and LLM assist wherever the hardware supports it. Cloud AI is opt-in per operation, not a global toggle.
- **No advertising, no third-party trackers, ever.** Written into the license and TOS.
- **User owns keys and data.** Export/import supported. Data portable.
- **Reticulum License compliance**: user data is not used, directly or indirectly, in AI training datasets (see `04_LICENSING_AND_FORK_STRATEGY.md` §4.1–4.3).
- **Metadata minimization**: cloud stores only what is required for feature functionality; per-feature data-inventory published.
- **Location privacy**: precise location shared only within explicitly authorized groups; "fuzzed" location option offered.
- **Right-to-know / right-to-delete** implemented in-app.

---

## 13. Vulnerability Disclosure & Incident Response

### 13.1 Disclosure
- **security@ss-sp.org** (PGP key published).
- 90-day disclosure window (industry standard, aligned with Project Zero).
- CVEs filed via a CNA (either apply for our own or coordinate with a partner).
- Public advisories at `security.ss-sp.org` with signed detail.

### 13.2 Bug bounty
- Public scope: firmware, protocol, companion apps, cloud console.
- Out of scope: physical attacks, social engineering of employees, DoS.
- Reward tiers based on CVSS.

### 13.3 Incident response
- 24-hour internal acknowledgement of report.
- On-call security engineer.
- Triage severity → decide whether to hotfix, ship OTA, publish advisory.
- Post-mortem for every P1/P2 incident; published within 30 days.

### 13.4 Fleet-wide compromise plan
If `RelKey_A` **and** `RelKey_B` are simultaneously compromised (worst case):
- Bootloader **also** trusts a third `RelKey_C` held in cold storage, not used for signing but capable of authorizing a "key rotation" image.
- Cold-storage ceremony rotates release keys.
- New image pushed via RNS carrier mesh even if vendor CDN compromised.

If `ProvCA` compromised:
- All BirthCerts must be re-issued via companion "identity refresh" flow that requires physical device presence.
- Old BirthCerts blacklisted via signed CRL delivered mesh-wide.

---

## 14. Regulatory & Compliance Alignment

### 14.1 Radio
- **FCC Part 15** (Wi-Fi, BLE), **Part 97** where applicable.
- **ISED Canada**: RSS-247, RSS-102 SAR.
- **RED (EU)** and **CE-RED** for HaLow (869/915 MHz variants per region).
- **ETSI EN 300 220** (LoRa sub-GHz).
- **Country-locked TX power** enforced by regulatory region in `birthcert.regulatory_region`.

### 14.2 Cyber
- **EU Cyber Resilience Act (CRA)** — coming into force; our SBOM + signed updates + vulnerability disclosure policy + 5-year support window aligns.
- **UK PSTI** — default-password prohibition; we have no defaults, everything provisioned unique.
- **NIST IR 8259A** (IoT baseline) — device identification, config, data protection, interfaces, update, all covered.
- **FIPS-140-3** — alternate crypto profile (AES-256-GCM, ECDSA-P256, SHA-256, HKDF, RFC 5869) available for gov/enterprise builds; requires CMVP module which we would seek for the ATECC608 secure element already-certified path.

### 14.3 Data
- **GDPR** — DPO designated; data-processing inventory maintained; user rights implemented in-app.
- **CCPA/CPRA** — same posture applies globally.
- **HIPAA** — not intended as HIPAA-CE by default; ready for BAA in enterprise builds.
- **CJIS** — analysis in progress for LE deployments; requires additional audit-log & personnel-clearance controls.

### 14.4 Export
- **US EAR 5D002** — mass-market crypto self-classification with TSU notification.
- **EU dual-use regulation** — reviewed per Annex I 5A002/5D002.
- Distribution to sanctioned regions blocked at fulfillment.

---

## 15. Testing & Assurance

- **Static analysis**: `cppcheck`, `clang-tidy`, `-Werror`, `-Wextra`, `-Wshadow`, `-Wconversion` on firmware; `semgrep` and language-specific SAST on companion apps and cloud.
- **Dynamic**: `libFuzzer`/`AFL++` fuzzing of every wire-format parser (LXMF, SS-Ext CBOR, Meshtastic-compat protobuf, birthcert parser, plugin manifest parser). CI runs a nightly 12-hour fuzz campaign.
- **Coverage-guided fuzzing** on ratchet, KDF, framing.
- **Property-based tests** for state machines (session, mux, OTA).
- **Interop tests** against Meshtastic reference devices in a chamber.
- **HIL rig**: physical devices in a Faraday cage running a nightly regression.
- **Third-party pentest** annually.
- **Firmware reproducible builds** — a public verifier can rebuild the exact bytes we shipped from the tagged source.

---

## 16. Summary — Non-Negotiable Security Bindings

| # | Rule | Where enforced |
|---|---|---|
| 1 | No un-authenticated encryption. | Code review, CI cryptolint |
| 2 | No home-rolled crypto. | Code review |
| 3 | All firmware images dual-signed. | Bootloader |
| 4 | All eFuse security bits set on release units. | Factory line |
| 5 | Secure boot + flash encryption always enabled on release. | Factory line, CI check |
| 6 | Per-device unique keys. | Factory line, HSM audit |
| 7 | Vendor cannot decrypt user messages. | Architecture, no key escrow anywhere |
| 8 | Duress-PIN wipes + beacons. | UI security policy |
| 9 | Plugins run capability-sandboxed. | Kernel API |
| 10 | No telemetry off-device without explicit consent. | UI first-boot + product-marketing policy |
| 11 | 5-year minimum security-fix window on shipped hardware. | Governance |
| 12 | Reticulum "no-harm" clause honoured in AUP. | Sales, TOS |
| 13 | SBOM + reproducible builds every release. | CI |
| 14 | Third-party pentest yearly. | Security team |
| 15 | Coordinated disclosure and CVE filing on every advisory. | Security team |

**This document is the security floor. Anything below it does not ship.**
