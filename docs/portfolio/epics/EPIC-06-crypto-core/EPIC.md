<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-06 — Cryptography Core

**Primary WG:** wg-security · **Contributing:** wg-firmware, wg-protocol
**Priority:** P0 · **SKU:** ★ · **Milestone:** M1

## Outcome
`firmware/components/ss_crypto/` provides a single, audited, side-channel-hardened cryptography surface used by every other component. Includes primitives, ratchets, group crypto, PQ hybrid, and a passing Known-Answer-Test (KAT) suite.

## Constitution
C-05 `05_SECURITY_MODEL.md` §crypto (primitives, ratchets, group crypto, PQ hybrid); C-00 `00_MASTER_SOFTWARE_PLAN.md` §firmware stack (`ss_crypto` component); C-SEC `SECURITY.md` §disclosure & audit.

## Dependencies
EPIC-02 (baseline).

## Shards
- **S-06.A Primitives** — Ed25519, X25519, XChaCha20-Poly1305, HKDF-SHA-256, HMAC-SHA-256, Argon2id, BLAKE2s, ChaCha20.
- **S-06.B Random number generation** — TRNG source + reseeded ChaCha20-DRBG, health tests.
- **S-06.C KDF & key-hierarchy helpers** — HKDF wrappers, domain-separated labels.
- **S-06.D Signal Double Ratchet** — session state, out-of-order handling, header encryption.
- **S-06.E Sender-key group protocol** — MLS-simplified, rekey on membership change.
- **S-06.F Hybrid PQ** — X25519 + ML-KEM-768 for handshake; Ed25519 + ML-DSA-65 for signatures.
- **S-06.G Side-channel hardening** — constant-time primitives, memory-zeroization macros, cache-timing review.
- **S-06.H KAT & fuzz suite** — RFC/CAVP vectors, differential fuzz against libsodium & ring.
- **S-06.I Formal review** — external audit engagement plan, threat model doc.

## Exit criteria
1. All primitives pass their RFC KATs (Ed25519 RFC 8032, X25519 RFC 7748, ChaCha20-Poly1305 RFC 8439, Argon2 RFC 9106).
2. Double Ratchet passes Signal test vectors.
3. Sender-key protocol passes MLS test vectors (subset).
4. Hybrid PQ passes NIST FIPS 203 KATs for ML-KEM-768 and FIPS 204 for ML-DSA-65.
5. External audit report published with all HIGH/CRITICAL closed.
6. `wg-security` sign-off recorded.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R06-01 | PQ standard drift | Track NIST FIPS 203/204 releases, versioned crypto suite ID |
| R06-02 | Side-channel leak in primitives | External audit + valgrind-ctgrind |
| R06-03 | ROM crypto-library conflict, per SKU: **ESP32-S3** ROM libs on **Lite**; **ESP32-P4** mbedTLS-in-ROM on **Omega** (and Alpha at lock) — D-0021 | Use library-agnostic wrapper across both ROM variants |
