<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-06 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

---

### S-06-001 — Ed25519 sign/verify wrapper w/ constant-time impl
As a security engineer I want a constant-time Ed25519 sign/verify wrapper so that device identity signatures are correct and side-channel resistant.
- AC: passes RFC 8032 vectors; constant-time property checked with ctgrind; API matches `ss_crypto` surface conventions
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-01 | Const=C-05

### S-06-002 — X25519 keypair + ECDH wrapper
As a security engineer I want an X25519 keypair and ECDH wrapper so that key exchange has one audited entry point.
- AC: passes RFC 7748 test vectors; low-order point inputs rejected; shared secret zeroized after derivation
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-02 | Const=C-05

### S-06-003 — XChaCha20-Poly1305 AEAD wrapper
As a security engineer I want an XChaCha20-Poly1305 AEAD wrapper so that all payload encryption uses one vetted primitive.
- AC: passes RFC 8439-derived and XChaCha test vectors; random 192-bit nonce policy enforced by API; decrypt failure returns no plaintext and is constant-time
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-04,F-SEC-05 | Const=C-05

### S-06-004 — HKDF-SHA-256 KDF wrapper w/ domain-separation labels
As a firmware engineer I want an HKDF-SHA-256 wrapper with domain-separated labels so that every derived key is bound to its purpose.
- AC: passes RFC 5869 test vectors; label registry enforced at compile time; derivation without a registered label fails the build or asserts
- Meta: Shard=C | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-01 | Const=C-05

### S-06-005 — HMAC-SHA-256 wrapper
As a firmware engineer I want an HMAC-SHA-256 wrapper so that MAC usage is uniform across components.
- AC: passes RFC 4231 test vectors; tag comparison is constant-time; key-length policy enforced
- Meta: Shard=A | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-04 | Const=C-05

### S-06-006 — Argon2id password KDF (backup passphrase)
As a device owner I want Argon2id protecting the backup passphrase so that offline guessing of my backup is expensive.
- AC: passes RFC 9106 test vectors; cost parameters tuned to the device budget and documented; parameter set versioned for future increases
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05

### S-06-007 — BLAKE2s hash wrapper
As a firmware engineer I want a BLAKE2s hash wrapper so that fast hashing is available where SHA-256 is too slow.
- AC: passes RFC 7693 test vectors; keyed-hash mode supported; on-target benchmark vs SHA-256 published
- Meta: Shard=A | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05

### S-06-008 — TRNG source health tests + reseed schedule
As a security engineer I want TRNG health tests and a DRBG reseed schedule so that all randomness is trustworthy for key generation.
- AC: continuous health tests (repetition count, adaptive proportion) run on TRNG output; ChaCha20-DRBG reseeds on schedule and on demand; health failure blocks key generation and raises an alarm
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-01 | Const=C-05

### S-06-009 — Memory-zeroize + secure-alloc helpers
As a security engineer I want memory-zeroize and secure-alloc helpers so that secrets never linger in freed memory.
- AC: zeroize survives compiler optimisation (verified in generated code/test); secure-alloc pools fenced and zeroized on free; usage enforced by lint for key-material types
- Meta: Shard=G | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-02 | Const=C-05

### S-06-010 — Signal Double Ratchet state machine
As a device owner I want the Signal Double Ratchet so that my 1:1 messages have forward secrecy and post-compromise security.
- AC: passes Signal test vectors; out-of-order messages within the window decrypt correctly; state machine covered by model-based tests
- Meta: Shard=D | Type=Feature | Size=XL | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-04 | Const=C-05

### S-06-011 — Ratchet persistence to NVS with anti-rollback counter
As a security engineer I want ratchet persistence to NVS with an anti-rollback counter so that captured old state cannot be replayed.
- AC: session state persists across reboot; anti-rollback counter rejects stale snapshots; persisted state encrypted at rest
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-04,NF-SEC-02 | Const=C-05

### S-06-012 — Header encryption + skipped-message keys
As a security engineer I want header encryption and skipped-message key handling so that metadata is protected and lossy links still decrypt.
- AC: header encryption matches Signal spec behaviour; skipped-key store bounded with an eviction policy; interop verified against a reference implementation
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-04 | Const=C-05

### S-06-013 — Sender-key group protocol (rekey on add/remove)
As a device owner I want the sender-key group protocol with rekey on membership change so that departed members cannot read new messages.
- AC: passes the MLS-subset test vectors; rekey triggered on add/remove within one message round; groups up to 128 members tested
- Meta: Shard=E | Type=Feature | Size=XL | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-05,F-MSG-02 | Const=C-05

### S-06-014 — Hybrid PQ handshake (X25519 + ML-KEM-768)
As a security engineer I want the hybrid PQ handshake (X25519 + ML-KEM-768) so that sessions resist harvest-now-decrypt-later attacks.
- AC: passes NIST FIPS 203 KATs for ML-KEM-768; hybrid secret combines both shares per spec; policy flag enables/disables PQ per PRD Q-03 posture
- Meta: Shard=F | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SEC-03 | Const=C-05

### S-06-015 — Hybrid PQ signatures (Ed25519 + ML-DSA-65)
As a security engineer I want hybrid PQ signatures (Ed25519 + ML-DSA-65) so that long-lived signatures survive a quantum adversary.
- AC: passes NIST FIPS 204 KATs for ML-DSA-65; hybrid verify requires both signatures valid; signature size and performance impact documented
- Meta: Shard=F | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SEC-03 | Const=C-05

### S-06-016 — Crypto suite ID + version negotiation
As a protocol engineer I want a crypto suite ID with version negotiation so that suites can evolve without breaking deployed devices.
- AC: suite ID carried in the handshake; downgrade attempts detected and rejected; negotiation matrix covered by tests
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-03 | Const=C-02,C-05
- Deps: RFC-0003 (compat/deprecation policy — suite windows & sunset are governed there, not restated here)

### S-06-017 — KAT suite runner + CI gate
As a release manager I want a KAT suite runner as a CI gate so that no crypto change merges without passing known-answer tests.
- AC: RFC/CAVP vectors run on host and on target; CI blocks merge on any KAT failure; vector provenance documented
- Meta: Shard=H | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05,C-06

### S-06-018 — Differential fuzz harness (libsodium ref)
As a security engineer I want a differential fuzz harness against libsodium so that our implementations never silently diverge.
- AC: fuzzers compare outputs against libsodium (and ring where applicable); corpus persisted and grown in CI; any divergence fails the build with a reproducer
- Meta: Shard=H | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05

### S-06-019 — Cache-timing review + ctgrind report
As a security engineer I want a cache-timing review with a ctgrind report so that timing side channels are found before external audit.
- AC: all primitives run clean under valgrind-ctgrind; findings triaged with fixes or documented waivers; report archived per release
- Meta: Shard=G | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05

### S-06-020 — External audit engagement (Trail of Bits / Cure53)
As the security WG chair I want an external audit engagement so that the crypto core is independently validated before release.
- AC: engagement scoped and contracted; audit report published; all HIGH/CRITICAL findings closed before release
- Meta: Shard=I | Type=Task | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-06 | Const=C-05,C-SEC

### S-06-021 — Threat model doc `SECURITY.md` deep-dive
As a security engineer I want a deep-dive threat model document so that design decisions trace to explicit adversaries and mitigations.
- AC: threat model covers assets, adversaries, and trust boundaries for `ss_crypto`; mitigations map to stories and exit criteria; wg-security sign-off recorded
- Meta: Shard=I | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05,C-SEC
