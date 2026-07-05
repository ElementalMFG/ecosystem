<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-12 — LXMF Messaging

**Primary WG:** wg-protocol · **Contributing:** wg-firmware, wg-ui-ux
**Priority:** P0 · **SKU:** ★ · **Milestone:** M2

## Outcome
LXMF is the messaging layer atop Reticulum. Users send/receive text (and voice notes) with store-and-forward semantics, delivery receipts, priority classes, and optional propagation nodes. Home Gateways can act as propagation nodes.

## Constitution
C-02 `02_PROTOCOL_STACK.md` §LXMF; C-08 `08_UNIVERSAL_CONNECTIVITY.md` §Home Gateway; C-05 `05_SECURITY_MODEL.md` §group messaging.

## Dependencies
EPIC-11.

## Shards
- **S-12.A LXMF message struct + serialisation.**
- **S-12.B Sender + receiver state machines.**
- **S-12.C Delivery receipts + retry policy.**
- **S-12.D Priority classes** — SOS, high, normal, bulk.
- **S-12.E Propagation-node role** — HGW acts as store.
- **S-12.F Attachment support** — small files + voice notes.
- **S-12.G Message storage on device** — encrypted with device key.
- **S-12.H Read/unread + threading.**
- **S-12.I Group message adapter** — sender-key crypto plugged into group threads.

## Exit criteria
1. Lite → Lite text delivery over LoRa RNS end-to-end works.
2. Store-and-forward through an HGW-acting propagation node works.
3. Voice-note attachment (Codec2) delivers.
4. Delivery receipts round-trip.
5. Priority preemption confirmed under load.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R12-01 | LXMF spec drift | Track LXMF releases, version negotiation |
| R12-02 | Storage growth | Retention policy + user quota |
| R12-03 | Group protocol misuse | Formal review of sender-key layer |
