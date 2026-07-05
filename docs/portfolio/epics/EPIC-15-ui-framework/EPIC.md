<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-15 — UI Framework (`ss_ui`)

**Primary WG:** wg-ui-ux, wg-firmware · **Contributing:** wg-apps, wg-docs
**Priority:** P0 · **SKU:** ★ · **Milestone:** M1

## Outcome
A device UI toolkit that owns the framebuffer, screen graph, input events, notifications, theming, localisation, accessibility, and a screen-scripting DSL for application layer. Fast (60 FPS partial redraws), memory-lean (< 32 KB heap), and pluggable.

## Constitution
C-03 `03_UI_LAYOUT_SPEC.md`; C-00 `00_MASTER_SOFTWARE_PLAN.md` §UI stack.

## Dependencies
EPIC-02, EPIC-03.

## Shards
- **S-15.A Framebuffer + double-buffer.**
- **S-15.B Widget primitives** — text, icon, button, list, toast, spinner.
- **S-15.C Screen-graph & navigation stack.**
- **S-15.D Input events** — buttons, PTT, hold, gesture.
- **S-15.E Notifications overlay.**
- **S-15.F Theming** — light/dark/high-contrast/night.
- **S-15.G Localisation** — string catalogues, RTL support.
- **S-15.H Accessibility** — large-font, screen-reader hook, colour-blind palettes.
- **S-15.I Screen DSL / declarative markup** — for app layer.

## Exit criteria
1. Renders 60 FPS partial redraws on Lite hardware.
2. Heap ≤ 32 KB steady-state.
3. Localised to at least EN, ES, FR at ship.
4. High-contrast + large-font mode functional.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R15-01 | DSL over-scope | MVP DSL only; extend post-M2 |
| R15-02 | Fonts license issues | OFL-1.1 fonts curated |
| R15-03 | Perf on high-DPI Alpha | GPU acceleration lane |
