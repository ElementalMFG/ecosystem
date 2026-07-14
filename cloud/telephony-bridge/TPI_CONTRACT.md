<!-- SPDX-License-Identifier: Apache-2.0 -->
<!-- Copyright (c) 2026 SS-SP Project Contributors -->

# Telephony Provider Interface (TPI) — FROZEN contract specification

Status: FROZEN v1.0 (contract only — no reference implementation in this pass). Story: S-21-030. Tier: T3. Owners: EPIC-21 (cloud) + EPIC-10 (bearer) + EPIC-20 (SDK); wg-security + wg-legal sign-off on §7. Substrate: `docs/portfolio/12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md` §2, decision `D-0025` (`governance/decisions.md`). Last updated: 2026-07-13.

This document is the carrier-neutral interface contract that every SS-SP telephony backend implements. It is precise enough that a future builder implements an adapter against it unmodified. It does **not** implement a reference adapter or any cloud service — those are `IN_PROGRESS`/blocked work (see §10). Where the substrate leaves a detail open, this contract fills it with a clearly-labelled **[DEFAULT]** that a reviewer may adjust before the first adapter ships.

Notation is a language-neutral typed pseudo-IDL plus tables; it is **not** a binding to any one language. The SDK (`sdk/`, EPIC-20) will project it into C / Python / TypeScript / Rust / Dart; those projections must not alter the semantics defined here.

## 1. Purpose and placement

### 1.1 Where the TPI sits

The mesh side never touches the PSTN directly. Every real-world telephony path resolves through a **bridge node** that terminates RNS/LXMF on the mesh side and speaks a carrier backend on the other side, via a pluggable **adapter** that implements this interface:

```
  mesh / LXMF side            bridge node (operator trust boundary)          carrier backend
  ────────────────            ──────────────────────────────────            ───────────────
  RNS identity        <--->   LXMF endpoint  ─  TPI core  ─  adapter   <--->  CPaaS REST /
  (Ed25519+X25519,            (queue, map,      (this        (tpi-cpaas,      SIP trunk /
   16-byte dest hash)          state machine)    contract)    tpi-sip, ...)    XMPP-PSTN / eSIM
                              ═══════════════ E2E boundary ═══════════════  ▶ PLAINTEXT edge (§7)
```

- **TPI core** is bridge-side code shared by all deployment locations (hosted cloud EPIC-21, Home Gateway EPIC-17, WASM sidecar EPIC-18). It owns the E.164↔LXMF map (§3), the delivery-state machine (§6), capability negotiation (§4), and retry/idempotency (§6).
- **Adapter** is the only carrier-specific code. It implements the operation set (§2) for exactly one backend family. Reference set: `tpi-cpaas`, `tpi-sip`, `tpi-xmpp`, `tpi-webrtc`, `tpi-esim` (§8).
- The interface is symmetric: the core calls the adapter for **outbound** operations (`send_message`, `originate_call`, `provision_number`, …); the adapter calls the core via **inbound callbacks / event stream** for carrier-originated events (`receive_message`, `answer_call`, `delivery_receipt`, `presence_hint`).

### 1.2 Capability-ready principle (binding)

Per `D-0023`/`D-0024` bearer-readiness and the `D-0025` "build it into the ecosystem even where it may not ship" posture: **the interface and at least one reference adapter (`tpi-cpaas`) are built and CI-maintained regardless of which tier is activated.** An adapter is never gated on a tier being sold. The TPI is `ADD`-only and never-degrades the mesh (`F-CL-07`): removing every adapter leaves native LXMF messaging fully functional.

### 1.3 The one code path

Rungs 0–4 of the Sovereignty Ladder (doc 12 §3) are the **same** bridge code against different backends at different deployment locations. Only the adapter binding and the deployment location differ; the contract below does not change between rungs.

## 2. Operation set

### 2.1 Common types

```idl
// Native mesh identity: the thing the user owns. 16-byte RNS destination hash.
type LxmfIdentity   = bytes[16]        // RNS/LXMF destination hash (hex-encoded in text form)
type E164           = string           // "+" + 1..15 digits, ITU-T E.164, no separators
type TelUri         = string           // "tel:" destination scheme, grammar in §3.3
type AdapterId      = string           // e.g. "tpi-cpaas", "tpi-sip"
type MsgId          = string           // adapter-scoped opaque message id, <= 128 chars
type CallId         = string           // adapter-scoped opaque call/session id, <= 128 chars
type IdemKey        = string           // caller-supplied idempotency key, <= 64 chars (§6.3)
type Millis         = uint64           // unix epoch milliseconds, UTC
type MediaType      = string           // IANA media type, e.g. "image/jpeg", "audio/opus"

enum Direction      { INBOUND, OUTBOUND }
enum MsgKind        { SMS, MMS }
enum DeliveryState  { QUEUED, SENT, DELIVERED, FAILED }   // lifecycle §6.4
enum CallState      { INIT, RINGING, ANSWERED, BRIDGED, ENDED, FAILED }
enum PresenceState  { UNKNOWN, REACHABLE, UNREACHABLE }
```

`Result<T>` is the uniform return envelope. Every operation returns it; no operation throws across the interface boundary:

```idl
type Result<T> = {
  ok:        bool,
  value?:    T,             // present iff ok == true
  error?:    TpiError,      // present iff ok == false
}
type TpiError = {
  code:      ErrorCode,     // taxonomy §2.9
  retryable: bool,          // core uses this for the retry policy §6.2
  message:   string,        // human-readable, non-localised, no secrets/content
  detail?:   map<string,string>,  // adapter diagnostic; MUST NOT contain message body or keys
}
```

### 2.2 Messaging — `send_message` / `receive_message`

```idl
type OutboundMessage = {
  idem_key:   IdemKey,      // §6.3 — dedup key; same key => same logical send
  from:       E164,         // a number this adapter has provisioned/owns
  to:         E164,         // destination MSISDN
  kind:       MsgKind,      // SMS | MMS
  text?:      string,       // UTF-8 body; required for SMS, optional caption for MMS
  media?:     [ MediaRef ], // required non-empty for MMS, MUST be empty for SMS
}
type MediaRef = {
  media_type: MediaType,
  uri?:       string,       // fetchable URI the adapter may pull from (bridge-local)
  bytes?:     bytes,        // inline payload; adapter picks uri XOR bytes per capability §4
  size:       uint32,       // octets, for pre-flight size-limit checks
}

send_message(msg: OutboundMessage) -> Result<SendAccepted>
type SendAccepted = { msg_id: MsgId, state: DeliveryState /* QUEUED|SENT */, at: Millis }
```

- **Pre:** `msg.from` is owned by this adapter (`provision_number` succeeded or `port_in` completed); `to` is a syntactically valid E.164; for `SMS` `text` non-empty and `media` empty; for `MMS` `media` non-empty and each `size` ≤ the adapter's advertised `max_mms_bytes` (§4); adapter advertises `CAP_SEND_SMS`/`CAP_SEND_MMS` respectively.
- **Post:** on `ok`, exactly one delivery-state lifecycle (§6.4) is started for `msg_id`; a later `delivery_receipt` (§2.7) will carry terminal state. Re-invoking with the same `idem_key` returns the same `msg_id` and does **not** send twice (§6.3).
- **Errors:** `UNSUPPORTED` (cap not advertised), `INVALID_ADDRESS`, `PAYLOAD_TOO_LARGE`, `NUMBER_NOT_OWNED`, `RATE_LIMITED`, `AUTH_FAILED`, `CARRIER_REJECTED`, `TRANSIENT`.

```idl
// Inbound: adapter -> core callback when the carrier delivers a message to an owned number.
receive_message(msg: InboundMessage) -> Result<Ack>
type InboundMessage = {
  msg_id:     MsgId,        // adapter-scoped; stable for dedup
  to:         E164,         // owned number that received it -> maps to LxmfIdentity (§3)
  from:       E164,         // sender MSISDN
  kind:       MsgKind,
  text?:      string,
  media?:     [ MediaRef ],
  received_at: Millis,
}
type Ack = { accepted: bool }   // core acks after it has durably enqueued to the LXMF inbox
```

- **Pre:** `to` resolves to a live entry in the E.164↔LXMF map (§3); otherwise the core returns `ok:false, code:NO_ROUTE` and the adapter must not re-deliver beyond its own carrier retry.
- **Post:** on `accepted`, the core has durably queued an LXMF message to the mapped identity; the adapter may tell the carrier the message is consumed. The core is the idempotency authority: a repeated `msg_id` is de-duplicated and re-acked without a second LXMF enqueue.

### 2.3 Voice — `originate_call` / `answer_call` / `bridge_media`

Voice is **IP-bearer-only** (never LoRa) and its media plane is a **separate story S-14-021**; this contract defines only the control-plane signatures and states so voice adapters target it unmodified. Media/codec notes are §5 (reference only).

```idl
originate_call(req: OriginateReq) -> Result<CallHandle>
type OriginateReq = {
  idem_key:   IdemKey,
  from:       E164,         // owned number
  to:         E164,         // callee MSISDN
  media:      MediaOffer,   // §5 — SDP-like offer; opaque to the core
}
type CallHandle = { call_id: CallId, state: CallState /* INIT|RINGING */, at: Millis }

answer_call(req: AnswerReq) -> Result<CallHandle>   // accept an INBOUND call the adapter signalled
type AnswerReq = { call_id: CallId, media: MediaAnswer }

bridge_media(req: BridgeReq) -> Result<BridgeHandle> // connect two legs / attach gateway transcode
type BridgeReq = { call_id: CallId, peer_call_id?: CallId, transcode?: TranscodeSpec /* §5 */ }
type BridgeHandle = { call_id: CallId, state: CallState /* BRIDGED */, at: Millis }
```

- **Pre (`originate_call`):** adapter advertises `CAP_VOICE_ORIGINATE`; `from` owned; deployment is a BYO/self-host rung (Rungs 1–4) — the hosted Rung-0 offering carries **no** user-originated PSTN voice (§7). Bearer is IP (`ip_bearer == true`).
- **Pre (`answer_call`):** a prior inbound-call event referenced `call_id`; adapter advertises `CAP_VOICE_ANSWER`.
- **Pre (`bridge_media`):** both legs exist and are in `RINGING`/`ANSWERED`; if codecs differ, `transcode` is present and the adapter advertises `CAP_TRANSCODE` (§5).
- **Post:** call transitions along the `CallState` machine (§2.1); the core is notified of every transition via `presence_hint`/an internal call-event callback carrying `{call_id, state, at}`. `ENDED`/`FAILED` are terminal.
- **Errors:** `UNSUPPORTED`, `VOICE_NOT_ON_LORA` (bearer is not IP), `POLICY_FORBIDDEN` (hosted-voice attempt on Rung 0), `INVALID_ADDRESS`, `NUMBER_NOT_OWNED`, `MEDIA_NEGOTIATION_FAILED`, `RATE_LIMITED`, `AUTH_FAILED`, `CARRIER_REJECTED`, `TRANSIENT`.

### 2.4 Number lifecycle — `provision_number` / `port_in` / `port_out` / `release_number`

```idl
provision_number(req: ProvisionReq) -> Result<NumberGrant>
type ProvisionReq = {
  idem_key:    IdemKey,
  identity:    LxmfIdentity,   // the mesh identity to bind the new alias to
  country:     string,         // ISO 3166-1 alpha-2; wg-legal availability gate (§3, §7)
  area_hint?:  string,         // desired area/region code, best-effort
  capabilities: [ Capability ],// requested ops (e.g. SMS-only vs SMS+voice)
}
type NumberGrant = { e164: E164, identity: LxmfIdentity, granted_at: Millis, ported: bool }

port_in(req: PortInReq)   -> Result<PortTicket>   // bring an existing E.164 onto this bridge
type PortInReq  = { idem_key: IdemKey, identity: LxmfIdentity, e164: E164, loa: PortAuth }
port_out(req: PortOutReq) -> Result<PortTicket>   // release to another carrier, NO fee, NO active-sub requirement
type PortOutReq = { idem_key: IdemKey, e164: E164, target_carrier: string, loa: PortAuth }
type PortAuth   = { account_ref: string, pin?: string, evidence_uri?: string } // LOA / account proof
type PortTicket = { ticket_id: string, state: PortState, e164: E164, eta?: Millis }
enum PortState  { REQUESTED, IN_PROGRESS, COMPLETED, REJECTED }

release_number(req: ReleaseReq) -> Result<Released>
type ReleaseReq = { idem_key: IdemKey, e164: E164 }
type Released   = { e164: E164, released_at: Millis }
```

- **Pre (`provision_number`):** adapter advertises `CAP_PROVISION`; `country` passes the wg-legal per-country availability gate; `identity` is a valid RNS destination hash.
- **Post (`provision_number`):** a new E.164↔LXMF map entry (§3) binds `e164`→`identity`; inbound to `e164` now routes to that LXMF inbox.
- **Pre (`port_out`):** adapter advertises `CAP_PORT_OUT`; `e164` is currently owned. **`port_out` MUST NOT require an active subscription and MUST NOT charge a fee** — this is the concrete "device outlives the company" guarantee (`07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §10; S-21-033). DID providers are chosen for clean no-fee port-out (VoIP.ms/Telnyx per doc 12 §3).
- **Post (`port_out`/`release_number`):** on `COMPLETED`/`Released` the map entry for `e164` is torn down cleanly (§3.2); inbound to `e164` afterwards returns `NO_ROUTE`.
- **Errors:** `UNSUPPORTED`, `COUNTRY_UNAVAILABLE`, `NUMBER_NOT_OWNED`, `NUMBER_UNAVAILABLE`, `PORT_REJECTED`, `AUTH_FAILED`, `CARRIER_REJECTED`, `TRANSIENT`. Port operations are long-running: they return a `PortTicket` immediately and drive state via later port-event callbacks; they are **never** synchronous.

### 2.5 `send_dtmf`

```idl
send_dtmf(req: DtmfReq) -> Result<Ack>
type DtmfReq = { call_id: CallId, digits: string /* [0-9A-D*#] */, tone_ms?: uint16 /* [DEFAULT] 200 */ }
```

- **Pre:** `call_id` is in `ANSWERED`/`BRIDGED`; adapter advertises `CAP_DTMF`; `digits` matches `[0-9A-D*#]+`.
- **Post:** tones are injected into the active media leg in order; on `ok` the carrier accepted them. **Errors:** `UNSUPPORTED`, `INVALID_STATE`, `INVALID_ARG`, `TRANSIENT`.

### 2.6 `delivery_receipt`

```idl
// Adapter -> core callback: terminal or intermediate delivery state for a prior send_message.
delivery_receipt(rcpt: Receipt) -> Result<Ack>
type Receipt = {
  msg_id:   MsgId,
  state:    DeliveryState,      // SENT | DELIVERED | FAILED
  at:       Millis,
  reason?:  string,            // FAILED only: carrier reason, non-content
}
```

- **Pre:** `msg_id` was returned by a prior `send_message`. Out-of-order/duplicate receipts are tolerated: the core applies only **monotonic forward** transitions along §6.4 and ignores regressions.
- **Post:** the core surfaces the new state back to the originating LXMF identity (§6.4). A terminal `DELIVERED`/`FAILED` closes the lifecycle.

### 2.7 `presence_hint`

```idl
// Adapter -> core, advisory only. Carrier-side reachability signal for a peer number.
presence_hint(hint: Presence) -> Result<Ack>
type Presence = { peer: E164, state: PresenceState, at: Millis, source?: string }
```

- **Advisory, never authoritative.** PSTN gives weak presence at best; the core treats `UNKNOWN` as the safe default and MUST NOT block a send on a non-`REACHABLE` hint. Maps to the LXMF last-seen/announce model (doc 12 §4.1) as a soft indicator only.

### 2.8 Health / lifecycle (adapter management)

```idl
describe()   -> Result<AdapterDescriptor>    // §4 — capability advertisement; MUST be pure/idempotent
health()     -> Result<HealthReport>         // { up: bool, backend_reachable: bool, detail? }
start(cfg: AdapterConfig) -> Result<Ack>     // cfg carries backend credentials by reference only (§7.3)
stop()       -> Result<Ack>
```

### 2.9 Error taxonomy

```idl
enum ErrorCode {
  UNSUPPORTED,            // operation/media not in the adapter's advertised capabilities (§4) — non-retryable
  INVALID_ADDRESS,       // malformed E.164 / tel: URI — non-retryable
  INVALID_ARG,           // other malformed argument — non-retryable
  INVALID_STATE,         // op not valid for current call/message state — non-retryable
  PAYLOAD_TOO_LARGE,     // media exceeds advertised limit — non-retryable
  NO_ROUTE,              // no E.164<->LXMF map entry for the target — non-retryable
  NUMBER_NOT_OWNED,      // from/e164 not provisioned to this adapter — non-retryable
  NUMBER_UNAVAILABLE,    // requested number cannot be provisioned — non-retryable
  COUNTRY_UNAVAILABLE,   // wg-legal per-country gate closed — non-retryable
  PORT_REJECTED,         // carrier rejected a port request — non-retryable
  POLICY_FORBIDDEN,      // forbidden by deployment policy (e.g. hosted voice) — non-retryable
  VOICE_NOT_ON_LORA,     // voice attempted on a non-IP bearer — non-retryable
  MEDIA_NEGOTIATION_FAILED, // codec/SDP negotiation failed — retryable-with-renegotiation
  AUTH_FAILED,           // backend credential rejected — non-retryable (operator must fix creds)
  RATE_LIMITED,          // backend throttle — retryable after backoff (§6.2)
  CARRIER_REJECTED,      // carrier declined for a business/policy reason — non-retryable
  TRANSIENT,             // network/backend hiccup — retryable (§6.2)
  INTERNAL,              // adapter bug / unexpected — retryable with caution, alert
}
```

`retryable` in `TpiError` is authoritative for the core's retry loop; the mapping above is the **[DEFAULT]** and an adapter may only *narrow* retryability (never mark a non-retryable code retryable).

## 3. Addressing model

### 3.1 Identity and the E.164↔LXMF map

- The **root identity is the RNS/LXMF cryptographic identity** (Ed25519+X25519, 16-byte destination hash) — owned, portable, never issued by a carrier. A phone number is a **leased, portable alias**, never the root identity (doc 12 §2.2).
- The bridge core owns an authoritative **E.164 ↔ LxmfIdentity map**. Cardinality **[DEFAULT]**: one live `E164 → LxmfIdentity` binding at a time (an identity MAY hold several numbers; a number binds to exactly one identity while live). The same alias table also carries external addresses (`xmpp:`, `nostr:`npub, `matrix:`, `sip:`) for the "universal contact" model — out of scope for TPI ops but the map schema reserves them.

```idl
type MapEntry = {
  e164:       E164,
  identity:   LxmfIdentity,
  adapter:    AdapterId,        // which adapter owns the carrier side of this number
  state:      MapState,
  created_at: Millis,
  aliases?:   [ string ],       // reserved: xmpp:/nostr:/matrix:/sip: for universal-contact
}
enum MapState { ACTIVE, PORTING_IN, PORTING_OUT, RELEASED }
```

### 3.2 Lifecycle, collision, reassignment

- **Create:** `provision_number` or a `COMPLETED` `port_in` inserts an `ACTIVE` entry. **Collision:** provisioning/porting a number already live in the map fails `NUMBER_UNAVAILABLE` — the core never silently rebinds a live number.
- **Teardown:** `port_out`/`release_number` moves the entry `PORTING_OUT`→`RELEASED` then removes it; inbound afterwards is `NO_ROUTE`.
- **Reassignment quarantine [DEFAULT]:** a released E.164 must not be re-bound to a *different* identity for a cool-down of **90 days** (carrier aging + avoid mis-delivery to a prior owner's identity). The prior owner re-acquiring their own number within the window rebinds to the same identity freely.
- **Ownership:** the phone number and carrier account always belong to the **bridge operator** (`D-0025`); the map binds the operator's number to a subscriber identity, it does not transfer carrier ownership.

### 3.3 The `tel:` destination scheme

Mesh users reach the phone network via a `tel:` destination. Grammar (RFC 3966 subset, [DEFAULT] narrowed to global numbers):

```abnf
tel-uri     = "tel:" global-number [ ";" params ]
global-number = "+" 1*15DIGIT            ; E.164, no visual separators in the routing form
params      = param *( ";" param )
param       = pname [ "=" pvalue ]       ; e.g. "kind=mms", reserved for future use
```

Routing semantics:

- **Outbound (mesh → PSTN):** an LXMF message/call addressed to `tel:+<E.164>` is resolved by the core: the sender identity must own (be bound to) a `from` number on an adapter advertising the needed capability; the core selects that adapter and invokes `send_message`/`originate_call` with `to = <E.164>`. No binding for the sender → the send fails `NUMBER_NOT_OWNED` (the user needs a provisioned number first).
- **Inbound (PSTN → mesh):** a carrier event on an owned `to` number is mapped `E164 → LxmfIdentity` via §3.1 and delivered to that identity's LXMF inbox (`receive_message`) or rung (`answer_call`). No map entry → `NO_ROUTE`.
- `tel:` is normalised to `"+" + digits` before lookup; any visual separators, `-`, spaces, or `%20` are stripped. Non-global (local) numbers are rejected `INVALID_ADDRESS` in this contract version.

## 4. Capability negotiation surface

An adapter advertises exactly what it supports; the core degrades gracefully and never calls an unadvertised operation.

```idl
describe() -> Result<AdapterDescriptor>
type AdapterDescriptor = {
  adapter_id:      AdapterId,
  version:         string,           // adapter semver
  contract:        string,           // TPI contract version it targets, e.g. "1.0"
  capabilities:    [ Capability ],   // the ops it implements
  send_media_types:[ MediaType ],    // MMS/media types it can send ([] => text-only)
  recv_media_types:[ MediaType ],    // media types it can receive
  number_ops:      [ NumberOp ],     // subset of {PROVISION, PORT_IN, PORT_OUT, RELEASE}
  max_sms_len:     uint16,           // GSM-7/UCS-2 concat cap it will accept
  max_mms_bytes:   uint32,
  ip_bearer:       bool,             // true => can carry voice legs (§2.3, §5)
  media_offer_kind?: string,         // e.g. "sdp"; absent => no live media
}
enum Capability {
  CAP_SEND_SMS, CAP_RECV_SMS, CAP_SEND_MMS, CAP_RECV_MMS,
  CAP_VOICE_ORIGINATE, CAP_VOICE_ANSWER, CAP_VOICE_BRIDGE, CAP_TRANSCODE, CAP_DTMF,
  CAP_PROVISION, CAP_PORT_IN, CAP_PORT_OUT, CAP_RELEASE,
  CAP_DELIVERY_RECEIPT, CAP_PRESENCE_HINT
}
enum NumberOp { PROVISION, PORT_IN, PORT_OUT, RELEASE }
```

Graceful degradation rules (binding):

- The core consults `describe()` once at `start` and caches it; it MUST NOT invoke an operation whose `Capability` is absent — doing so is a core bug, and any adapter that receives one returns `UNSUPPORTED`.
- **Text-first fallback:** if voice caps are absent, the deployment is a text-only bridge (the common LoRa/hosted case); inbound calls surface as **voicemail-to-text** only where a voice-capable gateway exists (P2, doc 12 §4.3) — otherwise the caller hears the carrier's default and the event is dropped with a logged `UNSUPPORTED`.
- **Media downgrade:** an `MMS` to a `CAP_SEND_MMS`-less or media-type-mismatched adapter is either (a) rejected `UNSUPPORTED`, or (b) **[DEFAULT]** degraded by the core to an SMS carrying a bridge-hosted retrieval link, when `CAP_SEND_SMS` is present and the deployment enables link-degrade. The chosen behaviour is a deployment policy, surfaced to the user.
- The negotiated set is re-read on adapter restart; a hot capability change requires `stop`/`start`.

## 5. Media and codec handling (voice ops — reference only)

Voice media is implemented by **S-14-021**, not here; this section pins the contract's assumptions so that story targets it unmodified.

- **Bearer:** voice is **IP-bearer-only** — HaLow-gatewayed / Wi-Fi / any future cellular/satellite backhaul. **LoRa cannot carry live voice** (`D-0025`); an `originate_call`/`answer_call` on a non-IP bearer returns `VOICE_NOT_ON_LORA`.
- **Transcode at the gateway:** the PSTN leg is **G.711** (µ-law/A-law); the mesh/IP leg anchor is **Opus 1.5/1.6**. `bridge_media` with `transcode` performs **Opus ↔ G.711** at the gateway (doc 12 §4.2). Neural sub-kbps codecs stay on the gateway, never the MCU. Codec2 (LoRa-class) is not a live-PSTN codec.
- `MediaOffer`/`MediaAnswer`/`TranscodeSpec` are opaque to the core in this contract; their concrete SDP-like shape is defined by S-14-021. `media_offer_kind` in `describe()` names the format (`"sdp"` expected).

```idl
type MediaOffer   = { kind: string, blob: bytes }   // e.g. kind="sdp"; opaque to core
type MediaAnswer  = { kind: string, blob: bytes }
type TranscodeSpec= { from_codec: string, to_codec: string }  // e.g. "opus" -> "pcmu"
```

## 6. Error, retry, idempotency, and delivery state

### 6.1 Ownership of reliability

The **core** owns reliability (queue, retry, dedup, state machine); adapters are thin and stateless-preferred. Adapters surface truthful `retryable` flags and idempotency-honouring behaviour.

### 6.2 Retry policy [DEFAULT]

- Only `retryable == true` errors are retried. Policy: exponential backoff, base **1 s**, factor **2**, jitter ±20%, cap **60 s**, max **6** attempts, then terminal `FAILED`.
- `RATE_LIMITED` honours a carrier `Retry-After` if the adapter surfaces one in `detail["retry_after_ms"]`; otherwise the standard backoff applies.
- `AUTH_FAILED` is never retried automatically (operator must fix credentials); the core raises an operator alert.

### 6.3 Idempotency

- Every mutating operation takes an `idem_key` (caller-generated, unique per logical intent, `<= 64` chars). Re-invoking with the same `idem_key` MUST return the same result and cause **at most one** carrier-side effect.
- Outbound dedup is core-side (keyed by `idem_key`); inbound dedup is core-side (keyed by adapter `msg_id`/`call_id`). Adapters that also dedup carrier-side may do so but must remain consistent with the core.
- **[DEFAULT]** idempotency records are retained **24 h**, comfortably covering the retry window.

### 6.4 Delivery-state lifecycle (surfaced to LXMF)

```
   QUEUED ──send accepted──▶ SENT ──carrier confirms──▶ DELIVERED   (terminal)
     │                        │
     └─────────── FAILED ◀────┴── carrier reject / retries exhausted (terminal)
```

- Transitions are **monotonic forward only**; the core ignores regressions and duplicate receipts (§2.6). Legal edges: `QUEUED→SENT`, `QUEUED→FAILED`, `SENT→DELIVERED`, `SENT→FAILED`. `DELIVERED` and `FAILED` are terminal.
- Each transition is surfaced back to the originating **LXMF identity** as a delivery-state update (the on-device UI renders queued/sent/delivered/failed). MMS and voice states reuse the same enum where the carrier supports receipts; absent receipts, a send that is accepted rests at `SENT` and never fabricates `DELIVERED`.

## 7. Security and trust boundary

### 7.1 The plaintext boundary is real and documented

- Mesh-side traffic is **end-to-end encrypted to the subscriber identity** (RNS/LXMF, hybrid X25519+ML-KEM-768 posture, doc 12 §6). That E2E guarantee **ends at the bridge**: PSTN and any cross-protocol leg are **cleartext at the carrier edge by physics and regulation**.
- Every bridge UX MUST show this honestly — **labelled "not end-to-end past the bridge"**, no false E2E shield. The hosted admin surface (EPIC-21) and gateway admin surface (EPIC-17) render the plaintext boundary explicitly.
- **No content logging by default [DEFAULT]:** message bodies and media are relayed, not persisted; `TpiError.detail` and adapter logs MUST NOT contain message content, media bytes, keys, or credentials. Any retention (e.g. for delivery debugging) is opt-in, time-boxed, and passes wg-security review before a bridge ships (doc 12 §6).

### 7.2 "Not for emergency calling" posture (binding)

- All telephony features carry a prominent **"not for emergency calling"** notice; the project offers and implies **no 911/112 termination**. This is a classification/UX signal, never a legal shield.
- **SOS is independent of telephony** and remains **bearer-flood** (`C-05`); it never routes through the TPI. Loss/absence of any adapter has zero effect on SOS.
- Hosted Rung 0 carries **no user-originated PSTN voice** (text-first regulatory posture, doc 12 §7); voice exists only on operator-carrier Rungs 1–4. An `originate_call` under a Rung-0 policy returns `POLICY_FORBIDDEN`.

### 7.3 Credential handling

- Backend credentials (CPaaS API keys, SIP registrar creds) are supplied to `start()` **by reference** (a secret handle resolved from the deployment's secret store), never inlined in the descriptor, logs, or map. For BYO rungs the operator's credentials are isolated per-tenant (S-21-031). Credential handling passes wg-security review.

> Escalation note: this contract deliberately does **not** define key material, wire byte formats, eFuse, or persistence-atomicity mechanisms — those belong to `ss_crypto`/`protocol/**` and are out of the T3 scope here. An implementing story that needs to touch them must escalate (doc 10 §8.3).

## 8. Adapter-authoring guide

To add a new adapter (reference set: `tpi-cpaas`, `tpi-sip`, `tpi-xmpp`, `tpi-webrtc`, `tpi-esim`):

1. **Declare a capability profile.** Implement `describe()` returning the honest `AdapterDescriptor` (§4). Advertise only what you actually implement; unimplemented ops return `UNSUPPORTED`.
2. **Implement the outbound ops** you advertise (`send_message`, and as applicable `originate_call`/`answer_call`/`bridge_media`, `provision_number`/`port_*`/`release_number`, `send_dtmf`), each returning `Result<T>` with correct `code`/`retryable` per §2.9.
3. **Wire the inbound callbacks** (`receive_message`, `delivery_receipt`, `presence_hint`, call-events) into the core's event stream; honour core acks as the delivery contract.
4. **Honour idempotency** (§6.3): the same `idem_key` yields at most one carrier effect. Prefer stateless; if you keep carrier-side dedup, stay consistent with the core.
5. **Map errors precisely** to the taxonomy (§2.9); never leak content/keys into `message`/`detail` (§7.1).
6. **Respect the boundaries:** text-first where no IP bearer; voice only when `ip_bearer` and the rung permits; no content logging; surface the plaintext boundary.
7. **Config by reference** (§7.3): credentials via secret handles in `start()`.
8. **Pass the conformance suite** (§9) in CI before merge. `tpi-cpaas` is the CI-maintained reference (S-21-030) and the template to copy.

Conformance expectations: an adapter is contract-conformant iff (a) `describe()` is pure and matches actual behaviour, (b) every advertised op meets its pre/post/error clauses, (c) unadvertised ops return `UNSUPPORTED`, (d) idempotency and monotonic delivery-state hold, (e) no content/credential leakage, (f) all boundary/policy rules (§7) hold.

## 9. Conformance test outline (spec only — not runnable here)

A future adapter must pass a suite covering (each item is a required test group; the runnable suite is blocked per §10):

- **C-DESCRIBE:** `describe()` is idempotent; advertised caps match the ops that succeed; every non-advertised op returns `UNSUPPORTED`.
- **C-SMS:** send/receive round-trip; UTF-8 + GSM-7/UCS-2 boundary; oversize body → `PAYLOAD_TOO_LARGE`; unknown `to` inbound → `NO_ROUTE`.
- **C-MMS:** media round-trip within `max_mms_bytes`; oversize → `PAYLOAD_TOO_LARGE`; media-type not in `send_media_types` → `UNSUPPORTED` or policy link-degrade (§4).
- **C-IDEM:** duplicate `idem_key` → one carrier effect, identical result; duplicate inbound `msg_id` → one LXMF enqueue.
- **C-STATE:** delivery-state machine is monotonic forward; regressions/dupes ignored; no fabricated `DELIVERED`.
- **C-RETRY:** retryable errors backoff per §6.2; `AUTH_FAILED` not retried; exhaustion → `FAILED`.
- **C-NUMBER:** provision → map entry; collision → `NUMBER_UNAVAILABLE`; port-out no-fee/no-active-sub; release → `NO_ROUTE` + reassignment quarantine.
- **C-ADDR:** `tel:` grammar accept/reject; normalisation; outbound without a bound `from` → `NUMBER_NOT_OWNED`.
- **C-VOICE (voice adapters):** control-plane state machine; `VOICE_NOT_ON_LORA` on non-IP bearer; `POLICY_FORBIDDEN` under Rung-0 policy; DTMF only in `ANSWERED`/`BRIDGED`.
- **C-SECURITY:** no content/keys/credentials in errors or logs; plaintext-boundary flag surfaced; SOS path untouched by adapter presence/absence.
- **C-NEGOTIATE:** graceful degradation — core never calls an unadvertised op; text-first fallback when voice caps absent.

The reference suite runs the same set the CI-maintained `tpi-cpaas` must pass, against a **CPaaS sandbox** (blocked, §10).

## 10. Implementation status / blocked

- **This document (contract/spec/docs) is complete and FROZEN** at v1.0 — it satisfies the spec/design/docs tasks of S-21-030.
- **BLOCKED (out of scope this pass):** the reference `tpi-cpaas` adapter, the CI-maintained conformance suite, and any cloud service are **pending prerequisite infrastructure**: `cloud/` is an unscaffolded epic-gated tree (no build, no CI — `cloud/README.md`), `sdk/` has no source, and no **CPaaS sandbox** exists to run integration/conformance against. The story's impl+test ACs (reference adapter implemented + CI-maintained; conformance suite against a CPaaS sandbox) require cloud-service scaffolding + cloud CI + a provisioned CPaaS sandbox first.
- No STORIES.md / PRD / index edits are made by this pass; the story status flip and evidence linkage are handled by the orchestrating run.
