<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Owner decisions — everything waiting on you

This is the complete list of decisions, confirmations, and approvals that only
you (the project owner) can make. Nothing on this list can be decided by an
agent — each item is either an external account/purchase, a legal matter, a
matter of personal identity, or a judgment call the methodology reserves for
you.

**How to answer:** use the answer sheet at the bottom (or just reply in chat
with, e.g., "A1: b, A2: a, B1: yes"). Where an option is marked
**(recommended)** that is the option I would pick and why is stated. You can
answer in any order and in batches — nothing below blocks anything else except
where noted.

Status of everything else: all work that did *not* need your input is done and
committed (governance files, tooling, enforcement, skills, agents, 11 stories
DONE). The remaining 337 P0 stories run through the `story-run` loop and only
need the Group F approvals below to start.

---

## Group A — Code hosting (highest leverage: one decision unblocks six things)

The repo currently has **no remote** — it exists only on this machine. Pushing
it somewhere unblocks: branch protection (S-01-012), the CLA bot (S-01-008),
CODEOWNERS enforcement, Dependabot, the issue/PR templates going live, and the
security contact link. It is also your only real backup.

### A1. Which hosting platform?

- **a) GitHub (recommended)** — the CI workflows (`.github/workflows/`),
  Dependabot config, CODEOWNERS, issue templates, and the planned CLA bot are
  all already written for GitHub. Any other choice means rewriting them.
- b) GitLab — works, but every `.github/` artifact must be ported.
- c) Codeberg / self-hosted Forgejo — most "open" option, same porting cost,
  and no Dependabot/CLA-bot equivalents.
- d) Defer — keep local only (risk: zero backup; a disk failure loses the
  project).

### A2. Personal account or organization?

- **a) New GitHub organization (recommended)** — e.g. an org named `ss-sp`.
  Matches the project's "transition to a foundation later" plan
  (CONTRIBUTING §5), lets you add maintainers/working groups later without
  migrating, and keeps the project separate from your personal account.
- b) Your personal GitHub account — simplest today; migration to an org later
  is possible but breaks existing links/stars/forks history less cleanly.

If (a): tell me the org name you register (I must not guess URLs).

### A3. Repository name?

- **a) `ss-sp` (recommended)** — short, matches the project name.
- b) `SS-SP-SOFTWARE` — matches the local directory name exactly.
- c) Something else — your call.

### A4. Public now, or private first?

- **a) Public immediately (recommended)** — the docs are written for an open
  project; open-hardware credibility and the licensing strategy
  (`04_LICENSING_AND_FORK_STRATEGY.md`) assume openness; nothing secret is in
  the tree (settings deny-rules block `.env`/keys, and none exist anyway).
- b) Private first, public at a milestone — buys polish time, but the repo is
  already gate-clean, and private repos don't get free CI minutes at the same
  tier.

### A5. CLA: keep it, or go DCO-only?

CONTRIBUTING §5 currently promises an Apache-style CLA on first PR.

- **a) Keep DCO + CLA as documented (recommended)** — preserves the "move to a
  foundation later without re-asking every contributor" option. Requires
  naming *who the CLA grants to* (see E1) and installing a CLA bot such as
  `contributor-assistant/github-action` once hosted.
- b) DCO-only — simpler (many major projects do this), but weakens the
  future-foundation story; requires editing CONTRIBUTING §5 and dropping
  story S-01-008.

### A6. Branch protection while you are the only maintainer

CONTRIBUTING §9 requires 1 CODEOWNER approval (2 + wg-security for security
paths). With a single human, you cannot approve your own PRs.

- **a) Protection on, admin-bypass allowed for you (recommended)** — required
  checks (`dco`, `docs-lint`) always enforced; the review requirement exists
  but you may bypass as admin until a second maintainer joins. Honest and
  practical.
- b) Full enforcement, no bypass — blocks all merging until a second
  maintainer exists.
- c) No protection yet — required checks stay advisory (weakest; makes
  S-01-012 unverifiable).

---

## Group B — Domain and communication channels

CONTRIBUTING §12 and SECURITY.md promise channels that are "(to be
published)". Each needs an account or purchase only you can make.

### B1. Register the domain `ss-sp.org`?

- **a) Yes, register `ss-sp.org` (recommended)** — it is the name used
  throughout the docs (security@ss-sp.org, lists.ss-sp.org, discuss.ss-sp.org).
  Check availability first; if taken, we must pick a new canonical domain and
  do a mechanical docs pass.
- b) Different domain — tell me which; same docs pass required.
- c) Defer — email/list/forum stay "(to be published)"; S-01-005 can only
  reach IN_REVIEW, not fully DONE.

Registrar is your choice (Cloudflare, Porkbun, Namecheap are common low-cost
options); I don't need to know which — only when the domain is live.

### B2. Email for `security@ss-sp.org` (and general project mail)

Needed to make the security-reporting promise real.

- **a) Hosted mail on the domain (recommended)** — any provider with custom
  domains (e.g. Proton, Fastmail, Migadu, Google Workspace). One mailbox or
  an alias to your personal inbox is enough at this stage.
- b) Alias/forwarding only (e.g. registrar or Cloudflare Email Routing,
  forwarding to your Gmail) — cheapest; fine for inbound reports; you'd reply
  from a different address unless you configure send-as.
- c) Defer — keep using GitHub private vulnerability reporting once hosted
  (see C3) as the only channel.

### B3. Developer mailing list `dev@lists.ss-sp.org`

- a) Real mailing list (e.g. Groups.io, or self-hosted Mailman) — traditional,
  more admin work.
- **b) Use GitHub Discussions instead and update CONTRIBUTING §12
  (recommended)** — zero cost/admin, lives with the code; a list can come
  later if the community wants one.
- c) Defer — leave "(to be published)".

### B4. Chat: Matrix `#ss-sp:matrix.org` + IRC `#ss-sp` (Libera)

- **a) Create the Matrix room now, defer IRC (recommended)** — Matrix room
  creation is free and minutes of work; register the room name before someone
  else does. IRC channel registration on Libera can follow.
- b) Create both now.
- c) Discord instead — popular but proprietary; would contradict the docs and
  need a docs pass.
- d) Defer both.

### B5. Forum `discuss.ss-sp.org`

- a) Hosted Discourse — the classic choice, but it has real monthly cost;
  premature for a pre-community project.
- **b) GitHub Discussions for now, Discourse when there's a community
  (recommended)** — update CONTRIBUTING §12 accordingly.
- c) Defer entirely.

---

## Group C — Security contact and keys (story S-01-005, the only READY story)

### C1. Key type for encrypted vulnerability reports

- **a) PGP/GnuPG key (recommended)** — the de-facto standard for
  `security.txt` / SECURITY.md contacts; every security researcher can use it.
- b) age key — simpler tooling, but unusual for this purpose; some reporters
  won't have it.
- c) Both (publish PGP primary, age alternative).

### C2. Key identity and custody

- **a) Key in the project's name, held by you (recommended)** — e.g. UID
  "SS-SP Security <security@ss-sp.org>"; private key stored in your password
  manager plus one offline backup (USB stick or printed revocation
  certificate). Survives a personal-address change.
- b) Key in your personal name/address — simplest, but ties the project's
  security contact to you personally forever.

I can generate the key with you at the terminal, but **you** must choose and
store the passphrase and backup — I should never see or hold private-key
material.

### C3. Interim contact before the domain/mailbox exists

- **a) GitHub Private Vulnerability Reporting (recommended, needs Group A
  first)** — free, built-in, no email required; SECURITY.md points at the
  repo's "Report a vulnerability" button.
- b) Publish `dylanpeterson@gmail.com` as interim security contact — works
  today with zero prerequisites, but puts your personal address in a public
  security doc.
- c) Wait for B1+B2 — S-01-005 stays IN_REVIEW until then.

---

## Group D — Hardware

### D1. Buy the Lite dev board?

The Lite firmware targets the **Elecrow CrowPanel Advance 3.5″ (ESP32-S3)**.
Without a physical board: no flash testing, no pin-map verification, and
EPIC-03 HAL stories can only reach IN_REVIEW, never hardware-verified DONE.

- **a) Buy 2 boards now (recommended)** — one on the desk, one spare (bricked
  board or hardware-revision surprises don't halt work).
- b) Buy 1 board now — cheapest start.
- c) Defer — firmware work continues build-only; a growing set of stories
  parks at IN_REVIEW/BLOCKED.

### D2. Flashing accessories for WSL2

Flashing from WSL2 requires `usbipd-win` on the Windows side (documented in
`docs/dev/BUILDING.md`) plus a data-capable USB-C cable. No decision needed
beyond: confirm you're OK installing `usbipd-win` on Windows when the board
arrives — **yes / no**.

### D3. Test-rig hardware (EPIC-22, 19 P0 stories)

Hardware-in-the-loop rigs come later. Nothing to decide now except
acknowledging the sequencing: **rigs are deferred until after D1 hardware is
in hand and basic firmware boots** — confirm / object.

---

## Group E — Legal and business

I am not a lawyer; items here flag *what needs deciding*, not legal advice.
Consider professional advice for E1/E2 when money or contributors arrive.

### E1. Who is the legal "project" today?

Matters for: the CLA grantee (A5a), the copyright line
"SS-SP Project Contributors", and any future commercial activity
(docs 05/09).

- **a) Unincorporated project, you as sole maintainer (recommended for now)**
  — costs nothing; CLA (if kept) names you personally as the grantee with the
  documented intent to reassign to a future entity/foundation. Revisit before
  taking money or signing manufacturing agreements.
- b) Form an LLC (or local equivalent) now — clean liability separation
  before commercial activity; real cost and paperwork.
- c) Decide later — fine, but blocks A5a (the CLA needs a named grantee), so
  A5 would fall back to DCO-only until decided.

### E2. Trademark on the names ("SS-SP", "Seekie-Speakie")?

- **a) Defer, but do a basic collision search now (recommended)** — cheap
  sanity check that the names aren't already used for similar goods before
  they're printed on hardware.
- b) File trademark application(s) now — real cost; usually premature
  pre-revenue.

### E3. Funding/sponsorship (FUNDING.yml, GitHub Sponsors)

- **a) Defer (recommended)** — previously assessed as not needed; nothing to
  sponsor yet.
- b) Enable now — tell me the platform(s) and handles.

---

## Group F — Local tooling and how the remaining work runs

### F1. Install clang-format (one command, needs your sudo)

The C style gate exists (`.clang-format`) but the tool is **not installed** on
this machine. Only you can run: `sudo apt install clang-format`.
**Approve / you'll run it yourself / defer.**

### F2. Auto-format hook after F1?

- **a) Yes (recommended)** — a Claude Code PostToolUse hook runs clang-format
  on any C file an agent edits; style drift becomes impossible.
- b) No — format manually / in CI review only.

### F3. Execution order for the 337 remaining P0 stories

- **a) Dependency order (recommended):** finish EPIC-01 governance remainder
  (3 P0) → EPIC-02 firmware-foundation (12 P0) → EPIC-03 HAL-Lite (20 P0) →
  outward through the dependency graph. Rationale: everything else `Deps=`-
  chains back to these.
- b) A different priority you name (e.g. protocol/EPIC-05 first, or
  compliance/EPIC-24 early because it has the most P0s at 25).

### F4. Per-story confirmation, or batch approval?

The `story-run` skill asks you to confirm each story before starting.

- a) Keep per-story confirmation — maximum control, slowest cadence.
- **b) Batch approval per epic (recommended)** — you approve "run EPIC-02"
  once; I still run one story at a time with full gates and a commit per
  story, and stop to ask on anything BLOCKED, T1, or judgment-call.
- c) Standing approval for the whole P0 pipeline — fastest; you review via
  commits/reports; T1 stories still always stop for you.

### F5. T1 stories (crypto/keys/secure-boot/wire/sandbox)

Doc 10 forbids demoting T1 work. Confirm the standing rule: **T1 stories are
always run via the `t1-pipeline` skill at maximum effort and always pause for
your explicit go-ahead, regardless of F4 — confirm / object.**

---

## Information I need from you (once the above are answered)

1. **Org/repo URL** after A1–A3 (never guessed): e.g.
   `github.com/<org>/<repo>` — I'll wire `git remote`, and give you the exact
   branch-protection click-path for S-01-012.
2. **"Domain is live" notification** after B1 — triggers the docs pass
   replacing every "(to be published)".
3. **Security mailbox address** after B2 (if not `security@ss-sp.org`).
4. **DCO identity confirmation** — commits are signed
   `dylan peterson <dylanpeterson@gmail.com>`; confirm this stays correct.
5. **Hardware arrival notification** after D1 — unblocks flash-test stories
   and `usbipd-win` setup.
6. **CLA grantee name** after A5a+E1 — the exact legal name the CLA text
   names.

---

## Answer sheet

Copy, fill, and send back (letters refer to the options above; "—" = decide
later):

```text
A1: _   A2: _   A3: _   A4: _   A5: _   A6: _
B1: _   B2: _   B3: _   B4: _   B5: _
C1: _   C2: _   C3: _
D1: _   D2 (usbipd-win ok?): _   D3 (rig sequencing ok?): _
E1: _   E2: _   E3: _
F1: _   F2: _   F3: _   F4: _   F5 (T1 rule ok?): _
Org/repo URL (when created): _______________
Anything else / overrides: _______________
```

Dependencies between answers (so you can answer partially):
A5a needs E1 answered. C3a needs Group A done. B2–B5 need B1 (or an
alternative domain). F2 needs F1. Everything in Group F can be answered today
with no external prerequisite.
