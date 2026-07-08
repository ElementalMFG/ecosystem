<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Copyright 2026 SS-SP Project Contributors -->

# S-02-019 — Frozen contract: per-artifact CycloneDX SBOM + keyless CI attestation

**STATUS: FROZEN** (t2-designer, 2026-07-07). `t2-builder` implements this
verbatim; any deviation requires re-opening the contract, not improvising.
**Amendment 2026-07-07** (t2-designer): corrected §3.4 first-party component
count premise (tree yields 2 at HEAD, not 18) and replaced the §10.3
`count >= 18` threshold with a dynamic tree-equality assertion. No other
sections touched.

Story AC (verbatim): *"CycloneDX JSON emitted per artifact; SBOM signed by CI;
SBOM published alongside the release."*
PRD: NF-SEC-05, NF-REG-03. Constitution: C-00; C-OA (open assurance —
reproducible builds + public SBOMs make silent backdoor introduction
detectable); `05_SECURITY_MODEL.md` §14.2 (CRA alignment), §15 (reproducible
builds), §16 rule 13 ("SBOM + reproducible builds every release — enforced in
CI").

---

## 1. Purpose and scope

One host-side generator, `tools/gen-sbom.py`, produces a CycloneDX **1.6 JSON**
SBOM per firmware artifact (one per board: `lite`, `alpha`, `omega`) from
(a) the build output directory and (b) the repo tree. CI generates the SBOM in
the existing `firmware-build` matrix and, on release tags, attests it keylessly
(Sigstore via GitHub OIDC) and publishes it alongside the release assets.

### 1.1 Security boundary — NO T1 scope creep (binding)

- Signing is **keyless only**: `actions/attest-sbom` /
  `actions/attest-build-provenance` (GitHub OIDC → Sigstore Fulcio/Rekor).
- This story MUST NOT introduce private signing keys, HSM/KMS material,
  eFuse operations, secure-boot signing, or OTA image signing. Those are
  T1 territory owned by EPIC-08/09/23 (see `05_SECURITY_MODEL.md` §5–§7).
  If implementation pressure pushes toward key material, STOP and escalate
  per doc 10 §8.
- The SBOM describes artifacts; it never contains secrets, device identity,
  or provisioning data.

### 1.2 Out of scope (and who owns it)

| Not here | Owner |
|---|---|
| Firmware image signing / secure boot chain | EPIC-08/09 (T1) |
| Vulnerability disclosure automation (VEX, advisories) | NF-REG-03 follow-on story, EPIC-23 |
| Compiled-in version resource on device | S-02-020 |
| Reproducible-build byte-for-byte verifier | C-OA verifier story (EPIC-02, later) |
| SBOMs for companion apps / cloud | their own epics |

---

## 2. CLI contract — `tools/gen-sbom.py`

Pure Python 3 (stdlib only — house rule for `tools/*.py`; no pip
dependencies), SPDX `Apache-2.0` header, runs on a plain host/CI runner.
No hardware, no network, no ESP-IDF installation required.

```
usage: gen-sbom.py --board {lite,alpha,omega} --build-dir DIR
                   [--output PATH] [--version-source SPEC]
                   [--repo-root DIR]
```

| Arg | Required | Meaning |
|---|---|---|
| `--board` | yes | Board variant; must be one of `lite`, `alpha`, `omega`. |
| `--build-dir` | yes | The `idf.py -B` output dir (e.g. `firmware/build/lite`). Must contain `ss_sp_firmware.bin`; `bootloader/bootloader.bin` and `partition_table/partition-table.bin` included when present (see §3.5). |
| `--output` | no | Output file path, or `-` for stdout. Default: `<build-dir>/<canonical filename per §5>`. Parent dirs are NOT created; missing parent is an input error (exit 3). |
| `--version-source` | no | One of `auto` (default), `env`, `git`, or `literal:<string>`. See §6.1. |
| `--repo-root` | no | Repo checkout root for component enumeration. Default: resolved as `Path(__file__).resolve().parent.parent` (matches `gen-stories-index.py`). |

- **stdin**: never read. **stdout**: the SBOM JSON only when `--output -`;
  otherwise a single line `wrote <path>`. All diagnostics go to stderr.
- **Exit codes**: `0` success; `2` usage error (argparse default — do not
  remap); `3` missing/unreadable input (build dir, `.bin`, components dir,
  version unresolvable under `--version-source git|env`); `4` internal
  invariant violation (e.g. duplicate `bom-ref`). No other codes.
- **Postcondition**: on exit 0 the output is a complete, deterministic
  (§7) CycloneDX 1.6 document passing every rule in §8. On non-zero exit
  no partial output file is left behind (write to temp file in the same
  directory, then atomic rename).
- Idempotent: re-running with identical inputs yields byte-identical output.

---

## 3. CycloneDX 1.6 document structure (exact)

Top-level object — exactly these keys, in this order (serialization rules
in §7):

```json
{
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "serialNumber": "urn:uuid:<uuidv5 per §3.1>",
  "version": 1,
  "metadata": { ... §3.2 ... },
  "components": [ ... §3.4/§3.5 ... ],
  "dependencies": [ ... §3.6 ... ]
}
```

`version` is the constant integer `1` (BOM revision, not firmware version).

### 3.1 `serialNumber` — deterministic UUID

```
SS_SBOM_NS = uuid5(NAMESPACE_DNS, "sbom.ss-sp.elementalmfg")   # frozen constant
serialNumber = "urn:uuid:" + uuid5(SS_SBOM_NS, f"{board}:{fw_version}:{bin_sha256_hex}")
```

Never `uuid4`. Same inputs ⇒ same serial; differing bin bytes ⇒ differing
serial (ties the SBOM identity to the artifact, per C-OA).

### 3.2 `metadata`

```json
"metadata": {
  "timestamp": "<RFC3339 UTC from SOURCE_DATE_EPOCH — OMITTED if unset, §7>",
  "tools": { "components": [ {
      "type": "application", "name": "gen-sbom.py",
      "version": "<fw_version>",
      "supplier": { "name": "SS-SP Project" } } ] },
  "component": { ... §3.3 ... }
}
```

`metadata.tools` uses the 1.6 `components`-array form (the legacy tool
array is deprecated in 1.6). The tool's `version` is the same `fw_version`
string (§6.1) — the tool is versioned with the repo.

### 3.3 `metadata.component` — the firmware artifact

```json
{
  "type": "firmware",
  "bom-ref": "ss-sp:firmware:<board>",
  "name": "ss_sp_firmware",
  "version": "<fw_version>",
  "supplier": { "name": "SS-SP Project" },
  "purl": "pkg:github/ElementalMFG/ecosystem@<git_sha>?board=<board>",
  "hashes": [ { "alg": "SHA-256", "content": "<hex sha256 of ss_sp_firmware.bin>" } ],
  "properties": [
    { "name": "ss:board", "value": "<board>" },
    { "name": "ss:artifact", "value": "ss_sp_firmware.bin" }
  ]
}
```

`<git_sha>` is the full 40-hex commit SHA (§6.1). The SHA-256 of the `.bin`
in `hashes` is mandatory — exit 3 if the `.bin` is missing.

### 3.4 `components[]` — first-party components, ESP-IDF, toolchain

One entry per first-party component: every directory matching
`firmware/components/ss_*` that contains a `CMakeLists.txt` (enumerated at
runtime from `--repo-root`, NOT a hardcoded list — the count is whatever the
tree yields at the built commit (2 at HEAD: `ss_hal`, `ss_log`; it grows as
skeleton `ss_*` dirs gain a `CMakeLists.txt`) and must track the tree). Shape:

```json
{
  "type": "library",
  "bom-ref": "ss-sp:component:<dirname>",
  "name": "<dirname>",
  "version": "<fw_version>",
  "supplier": { "name": "SS-SP Project" },
  "purl": "pkg:github/ElementalMFG/ecosystem@<git_sha>#firmware/components/<dirname>"
}
```

(First-party components version with the repo — single git SHA; no
`hashes` on source components.)

Plus exactly two third-party entries:

```json
{ "type": "framework", "bom-ref": "ss-sp:dep:esp-idf", "name": "esp-idf",
  "version": "<idf_version per §6.2>", "supplier": { "name": "Espressif Systems" },
  "purl": "pkg:github/espressif/esp-idf@<idf_version>" }

{ "type": "application", "bom-ref": "ss-sp:dep:xtensa-toolchain",
  "name": "xtensa-esp-elf-gcc", "version": "<toolchain_version per §6.3>",
  "supplier": { "name": "Espressif Systems" },
  "purl": "pkg:generic/espressif/xtensa-esp-elf@<toolchain_version>" }
```

### 3.5 Secondary artifacts (bootloader, partition table)

If present in the build dir, add (type `"firmware"`, with SHA-256 hashes):

- `bootloader/bootloader.bin` → `bom-ref` `ss-sp:artifact:bootloader`,
  name `bootloader`, version `<idf_version>` (it is IDF-generated),
  purl `pkg:github/espressif/esp-idf@<idf_version>#components/bootloader`.
- `partition_table/partition-table.bin` →
  `ss-sp:artifact:partition-table`, name `partition-table`, version
  `<fw_version>`, purl = firmware purl + `#firmware/partitions`.

Missing secondary artifacts are a warning to stderr, not an error (host
tests use fixture dirs without them; the `.bin` alone is mandatory).

**`bom-ref` invariant**: all refs unique; namespace prefixes
`ss-sp:firmware:`, `ss-sp:component:`, `ss-sp:artifact:`, `ss-sp:dep:` are
reserved and exhaustive. Duplicate ref ⇒ exit 4.

### 3.6 `dependencies[]` graph

- One entry `{ "ref": "ss-sp:firmware:<board>", "dependsOn": [ ...every
  other bom-ref in the document, sorted... ] }` — the image links
  everything.
- One entry per first-party component:
  `{ "ref": "ss-sp:component:<name>", "dependsOn": [...] }` where
  `dependsOn` is derived by parsing that component's `CMakeLists.txt`
  `idf_component_register(...)` call: tokens of `REQUIRES` and
  `PRIV_REQUIRES` (regex over the call body; whitespace/newline tolerant).
  Each token mapping: `ss_*` → `ss-sp:component:<token>`; anything else
  (IDF built-in like `nvs_flash`, `esp_timer`) → collapse to
  `ss-sp:dep:esp-idf` (deduplicated). Unresolvable `ss_*` token (names a
  non-existent component) ⇒ exit 4.
- Entries `{ "ref": "ss-sp:dep:esp-idf", "dependsOn": [] }` and the same
  for the toolchain and secondary artifacts (empty `dependsOn`, key still
  present).
- Every `bom-ref` in the document appears exactly once as a `ref`.

---

## 4. (reserved)

Section intentionally unused; numbering kept stable for review citations.

## 5. Output filename convention

```
ss_sp_firmware-<board>-<version>.cdx.json
```

`<version>` = `fw_version` sanitized: every char not in `[A-Za-z0-9._-]`
replaced with `-` (so `v0.3.0-12-gabc1234-dirty` passes through, `/` etc.
cannot). Example: `ss_sp_firmware-lite-v0.3.0-12-gabc1234.cdx.json`.
Provide `def sbom_filename(board: str, version: str) -> str` as an
importable pure function (host tests call it directly).

## 6. Version sourcing

### 6.1 Firmware version (`fw_version`) and `git_sha` — `--version-source`

| Spec | Behavior |
|---|---|
| `git` | `git -C <repo-root> describe --tags --always --dirty` → `fw_version`; `git rev-parse HEAD` → `git_sha`. Git failure ⇒ exit 3. |
| `env` | `fw_version` from `$SS_FW_VERSION`, `git_sha` from `$SS_FW_GIT_SHA` (40-hex enforced). Either missing ⇒ exit 3. |
| `literal:<s>` | `fw_version = <s>`; `git_sha` from env `SS_FW_GIT_SHA` else placeholder §6.4. |
| `auto` (default) | try `env`, else `git`, else placeholders §6.4 (with stderr warning; exit 0). |

### 6.2 ESP-IDF version (`idf_version`) — precedence

1. `$IDF_VERSION` env (CI sets it explicitly from the pinned container tag);
2. `<build-dir>/project_description.json` — first present of the keys
   `idf_version`, `git_revision` (builder verifies which key IDF v5.3.5
   actually emits and keeps only real ones, dropping this precedence note
   to a code comment);
3. placeholder §6.4.

### 6.3 Toolchain version — precedence

1. `$SS_TOOLCHAIN_VERSION` env;
2. parse from the compiler path in `project_description.json`
   (`.../xtensa-esp-elf/esp-<ver>...` pattern) if resolvable;
3. placeholder §6.4.

### 6.4 Documented placeholder

Unknown version fields use the exact string `UNKNOWN` **and** the owning
component gains `"properties": [{ "name": "ss:version-unresolved",
"value": "true" }]` so downstream CRA tooling can flag it. Placeholders
must never appear in release-tag CI (release workflow greps the SBOM for
`ss:version-unresolved` and fails the job — see §9.2 step R4).

## 7. Determinism rules (C-OA binding)

- **Timestamp**: if `$SOURCE_DATE_EPOCH` is set (firmware-build.yml already
  exports it from `git log -1 --format=%ct`), emit
  `metadata.timestamp` as that instant in RFC3339 UTC (`...Z`). If unset,
  **omit the key entirely**. Never call `now()`.
- All arrays sorted: `components[]` by `bom-ref` (ASCII), `dependencies[]`
  by `ref`, each `dependsOn` sorted, `properties` by `name`.
- Serialization: `json.dumps(doc, indent=2, sort_keys=True,
  ensure_ascii=False)` + single trailing `\n`, UTF-8, LF endings.
  (Insertion order in §3 is for readability; `sort_keys=True` is the
  normative wire order.)
- No environment leakage: absolute paths, hostnames, usernames, locale
  never appear in output.
- Byte-identical output for identical `(repo tree, build dir, env
  {SOURCE_DATE_EPOCH, SS_*, IDF_VERSION})` — this is a host-test assertion.

## 8. Validation rules (what "valid" means, host-test enforceable)

No external schema fetch and no pip `cyclonedx` dependency — validation is
these explicit rules, implemented in the host test (and importable as
`validate_sbom(doc) -> list[str]` from `gen-sbom.py` so CI can reuse it via
`--output -` piping in review, builder's choice):

V1. Top-level keys exactly `{bomFormat, specVersion, serialNumber, version,
    metadata, components, dependencies}`; `bomFormat == "CycloneDX"`,
    `specVersion == "1.6"`, `version == 1`.
V2. `serialNumber` matches `^urn:uuid:[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12}$`
    and equals the §3.1 recomputation.
V3. `metadata.component.type == "firmware"`; has non-empty `name`,
    `version`, `purl`; `hashes` contains exactly one `{alg: "SHA-256",
    content: 64-hex}` entry.
V4. Every `firmware/components/ss_*` dir (with CMakeLists.txt) in the repo
    has a matching `ss-sp:component:` entry; `ss-sp:dep:esp-idf` and
    `ss-sp:dep:xtensa-toolchain` present.
V5. All `bom-ref` unique; every `dependencies[].ref` and every `dependsOn`
    element resolves to a `bom-ref` in the document; every `bom-ref` has a
    `dependencies` entry.
V6. All arrays sorted per §7; if `metadata.timestamp` present it matches
    RFC3339 UTC `Z` form.
V7. Every `purl` starts with `pkg:` and every component has `type` in
    `{firmware, library, framework, application}`.

## 9. CI integration contract

### 9.1 `firmware-build.yml` (every push/PR — modify existing workflow)

In the existing `firmware` matrix job, after the `Size report` step:

- **B1 "Generate SBOM (<board>)"** — inside the same pinned container:
  `python3 tools/gen-sbom.py --board <board> --build-dir build/<board>
  --version-source auto` (env: `SOURCE_DATE_EPOCH` already set; set
  `IDF_VERSION` from the container's known tag `v5.3.5` — single source:
  a workflow-level `env:` var next to the pinned digest comment).
- **B2 "Validate SBOM"** — re-run the generator to a second path and
  `cmp` the two files (determinism gate), then run the V1–V7 validator.
- **B3 "Upload SBOM artifact"** — `actions/upload-artifact` with name
  `sbom-<board>`, path = the `.cdx.json`.

No new required checks; B1–B3 live inside the existing `build (<board>)`
gate. The `changes` path filter already covers
`.github/workflows/firmware-build.yml`.

Host-test job addition: `tools/tests/test_gen_sbom.py` runs under the
existing `host-tests` gate (fixture-driven, §10 — no container needed).

### 9.2 `release-sbom.yml` (NEW workflow — tag-triggered publish + attest)

Trigger: `push: tags: ['v*']`. Permissions (job-scoped, minimal):
`contents: write`, `id-token: write`, `attestations: write`.
Jobs: build matrix (lite/alpha/omega) reusing the same pinned container
digest + build steps as firmware-build.yml, then per board:

- **R1** generate SBOM with `--version-source env`
  (`SS_FW_VERSION=${GITHUB_REF_NAME}`, `SS_FW_GIT_SHA=${GITHUB_SHA}`,
  `IDF_VERSION=v5.3.5`) — release SBOMs never use `auto`.
- **R2** validate + determinism `cmp` (same as B2).
- **R3** `actions/attest-build-provenance@v2` on `ss_sp_firmware.bin`
  (SLSA provenance, keyless) and `actions/attest-sbom@v2` with
  `subject-path` = the `.bin`, `sbom-path` = the `.cdx.json`. Both produce
  Sigstore bundles stored in the repo's attestation store
  (`https://github.com/ElementalMFG/ecosystem/attestations`), verifiable
  offline-of-CI via `gh attestation verify <bin> --repo
  ElementalMFG/ecosystem`. **This is the "SBOM signed by CI" AC clause —
  keyless by construction (§1.1).** Pin both actions by full commit SHA
  in the workflow (supply-chain hygiene; builder resolves current SHAs).
- **R4** fail the job if the SBOM contains `ss:version-unresolved` (§6.4).
- **R5** `gh release upload` (or `softprops` equivalent — builder's choice,
  SHA-pinned) attaching, per board: `ss_sp_firmware-<board>-<ver>.bin`
  (renamed copy) + `ss_sp_firmware-<board>-<ver>.cdx.json` — the
  "published alongside the release" clause.

Attestation steps run on the ubuntu runner, not inside the IDF container
(the attest actions need the runner's OIDC environment); build in
container job → upload artifact → attest/publish in a follow-on
runner job is the required job split.

### 9.3 AC verifiability split (for story status)

| AC clause | Verifiable when |
|---|---|
| "CycloneDX JSON emitted per artifact" | NOW — B1–B3 run on every push; host tests gate structure. |
| "SBOM signed by CI" | Workflow + steps land now; **exercised only on a real `v*` tag** → story parks at IN_REVIEW until the first tag (or a `v0.x.y-rc` pre-release tag cut deliberately as evidence). |
| "SBOM published alongside the release" | Same as above (R5, tag-only). |

## 10. Host test contract — `tools/tests/test_gen_sbom.py`

Pure pytest, stdlib + pytest only, no network/container/hardware. Fixtures:
a `tmp_path` fake build dir factory writing a small deterministic
`ss_sp_firmware.bin` (known bytes ⇒ known SHA-256), optional
bootloader/partition-table files, and a minimal `project_description.json`.
Real repo tree is used for component enumeration (`--repo-root` = actual
repo root). Invoke the generator both as a subprocess (CLI/exit-code cases)
and via import (pure-function cases).

Mandatory cases (builder may add more, not fewer):

1. **Structure**: generated doc passes V1–V7 in full.
2. **Determinism**: two runs, same inputs, byte-identical files; with
   `SOURCE_DATE_EPOCH=1700000000` → exact expected `metadata.timestamp`;
   with it unset → key absent; output identical regardless of `PYTHONHASHSEED`.
3. **Component completeness**: the set of `ss-sp:component:` bom-refs in the
   SBOM must EQUAL the set of `firmware/components/ss_*` dirs containing a
   `CMakeLists.txt` under `--repo-root`, with that expected set computed by
   the test from the tree at runtime (no hardcoded names or counts); adding
   a fake `ss_zz_test` component dir (tmp repo-root copy or monkeypatched
   enumeration) appears without code change.
4. **Board variance**: `lite` vs `alpha` SBOMs differ in exactly
   `serialNumber`, `metadata.component` (`bom-ref`/`purl` board qualifier,
   hashes if bins differ, `ss:board` property), and the firmware node in
   `dependencies`; component list identical.
5. **Hashes**: `metadata.component.hashes[0].content` equals the fixture
   bin's known SHA-256; bootloader/partition-table entries carry correct
   hashes when the files exist and are absent (with the run still exit 0)
   when they don't.
6. **Version sourcing**: `env` mode honors `SS_FW_VERSION`/`SS_FW_GIT_SHA`
   and exits 3 when missing; `literal:` passes through; `auto` with no git
   and no env yields `UNKNOWN` + `ss:version-unresolved` property, exit 0.
7. **CLI errors**: missing `--build-dir`/`.bin` ⇒ exit 3, no output file
   created; bad `--board` ⇒ exit 2; stdout mode (`--output -`) emits JSON
   only, diagnostics on stderr.
8. **Filename**: `sbom_filename()` matches §5 including sanitization of
   `+`, `/`, space.
9. **Dependency graph**: parsing a fixture CMakeLists with `REQUIRES` +
   `PRIV_REQUIRES` (multi-line) maps `ss_*` refs and collapses IDF
   built-ins to `ss-sp:dep:esp-idf`; unknown `ss_*` token ⇒ exit 4.

## 11. Builder verification checklist

- `python3 tools/lint-docs.py` and `gen-stories-index.py --check` green.
- Host tests §10 all pass locally (`pytest tools/tests/test_gen_sbom.py`).
- `firmware-build` workflow green on all three boards with SBOM steps.
- Confirm which `project_description.json` key IDF v5.3.5 really provides
  (§6.2) and freeze the code to reality.
- Resolve and SHA-pin `attest-sbom` / `attest-build-provenance` versions.
- Story `Status` → IN_REVIEW (not DONE) until a real tag exercises §9.2.
