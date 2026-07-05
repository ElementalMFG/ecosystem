<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-15 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-15-001 — Framebuffer + double-buffer manager
As a UI engineer I want a double-buffered framebuffer manager owned by `ss_ui` so that screens render tear-free within the epic's memory budget.
- AC: Buffer swap is tear-free at display refresh on Lite 480×320 and 320×240 targets; framebuffer manager exposes a single owner API (no direct display writes from apps); steady-state heap attributable to buffering fits inside the `ss_ui` 32 KB budget
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03, C-00

### S-15-002 — Partial-redraw dirty-rect tracking
As a UI engineer I want dirty-rectangle tracking so that only changed regions are redrawn and 60 FPS is sustainable on Lite hardware.
- AC: Only widgets marked dirty are re-rendered and flushed, verified by draw-call instrumentation; a scrolling list sustains 60 FPS partial redraws on Lite hardware; full-screen invalidation still renders correctly as the fallback path
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01, NF-PERF-01 | Const=C-03, C-00

### S-15-003 — Text widget (font atlas, kerning)
As a UI engineer I want a text widget backed by a pre-baked font atlas with kerning so that readable, correctly spaced text renders fast on every display class.
- AC: Text renders with kerning and baseline alignment matching reference renders per `03_UI_LAYOUT_SPEC.md`; glyph lookup and layout of a full screen of text meets the 60 FPS frame budget on Lite; widget supports wrapping, truncation with ellipsis, and multi-size fonts from one atlas
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03

### S-15-004 — Icon set + icon widget
As a UI engineer I want a core icon set and icon widget so that all apps share consistent, theme-aware iconography.
- AC: Icon widget renders 1-bit/alpha icons tinted by the active theme palette; core set covers navigation, messaging, radio, battery, and SOS glyphs used by the v1 apps; icons scale correctly across the 480×320 and 320×240 display classes
- Meta: Shard=B | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03

### S-15-005 — Button + list widgets
As a UI engineer I want button and scrollable-list widgets so that app screens can be composed from standard interactive primitives.
- AC: Button supports pressed/focused/disabled states with theme-driven styling; list virtualises rows so a 500-item list scrolls at 60 FPS on Lite within the heap budget; both widgets are fully operable via hardware keys alone (one-hand rule)
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01, F-UI-06 | Const=C-03

### S-15-006 — Toast + modal + spinner
As a UI engineer I want toast, modal, and spinner primitives so that apps can show transient status, blocking confirmations, and progress consistently.
- AC: Toasts auto-dismiss on a configurable timeout without stealing input focus; modals trap input and are dismissible via a consistent cancel affordance; spinner animates within the frame budget without blocking the input queue
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03

### S-15-007 — Screen-graph + nav stack
As a UI engineer I want a screen graph with a navigation stack so that apps push/pop screens with predictable lifecycle and back behaviour.
- AC: Push/pop transitions fire mount/unmount lifecycle hooks exactly once per transition; back input always returns to the previous screen with state preserved; a popped screen releases all its widget allocations (verified by heap tracking)
- Meta: Shard=C | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03, C-00

### S-15-008 — Input event queue
As a firmware engineer I want a unified input event queue for buttons, hold, and gestures so that all input reaches the focused screen in order with no events dropped.
- AC: Button press, release, hold, and repeat events are delivered in order with timestamps; queue overflow policy drops oldest non-critical events and never drops SOS-related input; input-to-first-frame response meets NF-PERF-01 (< 100 ms) on Lite
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01, NF-PERF-01 | Const=C-03, C-00

### S-15-009 — PTT gesture handler
As a UI engineer I want a dedicated PTT press/hold/release gesture handler so that the voice subsystem gets reliable talk-state events regardless of the foreground screen.
- AC: PTT down/up events are delivered to the voice subsystem from any screen, including with modals open; hold detection distinguishes PTT-hold from short press per the timings in `03_UI_LAYOUT_SPEC.md`; gesture handler emits state changes with ≤ 20 ms added latency
- Meta: Shard=D | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01, F-MSG-04 | Const=C-03, C-00

### S-15-010 — Notification overlay stack
As a device owner I want a notification overlay that layers over any app so that incoming messages and alerts are visible without losing my place.
- AC: Notifications stack, coalesce per-thread, and are dismissible individually or all at once; SOS-class notifications preempt the stack and cannot be coalesced or auto-dismissed; overlay never blocks input to an active SOS or PTT interaction
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03

### S-15-011 — Theme engine (light/dark/hi-contrast/night)
As a device owner I want selectable light, dark, high-contrast, and night themes so that the screen is readable in daylight and preserves night vision after dark.
- AC: All widgets pull colours exclusively from the theme palette (no hard-coded colours, enforced by lint); switching theme applies to every live screen without reboot; high-contrast palette meets the daylight-readability contrast targets in `03_UI_LAYOUT_SPEC.md`
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-05 | Const=C-03

### S-15-012 — L10N string catalogue (EN, ES, FR at ship)
As a UI engineer I want all user-facing strings routed through `ss_i18n` catalogues so that the device ships localised in EN, ES, and FR.
- AC: CI fails on any user-facing string literal not extracted via `ss_i18n`; EN, ES, and FR catalogues are complete for all v1 screens with fallback to EN for missing keys; language switch takes effect on all screens without reboot
- Meta: Shard=G | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-04, NF-L10N-01 | Const=C-03

### S-15-013 — RTL layout support
As a UI engineer I want right-to-left layout mirroring in the layout engine so that Arabic-script locales render correctly at v1.5.
- AC: Layout engine mirrors widget order, alignment, and navigation affordances when an RTL locale is active; bidirectional text (RTL with embedded Latin) renders per Unicode bidi rules in the text widget; RTL snapshot tests run in CI alongside LTR
- Meta: Shard=G | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-UI-04, NF-L10N-02 | Const=C-03

### S-15-014 — Large-font accessibility mode
As a device owner I want a large-font mode so that I can read the screen without glasses in the field.
- AC: Large-font mode scales text on every v1 screen without clipping or overlapping widgets; setting persists across reboot; toggling requires no restart and reflows live screens
- Meta: Shard=H | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-02, NF-A11Y-02 | Const=C-03

### S-15-015 — Screen-reader event hook
As a UI engineer I want a screen-reader event hook emitting semantic focus/content events so that the TTS voice UI can speak what is on screen.
- AC: Every focusable widget emits a semantic description (role, label, state) on focus change; hook delivers events to the TTS consumer with ≤ 100 ms latency; all v1 app screens are fully traversable via the hook (parity check per NF-A11Y-02)
- Meta: Shard=H | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-UI-02, NF-A11Y-02 | Const=C-03, C-00

### S-15-016 — Colour-blind palette variants
As a device owner I want colour-blind-safe palette variants so that status colours remain distinguishable with deuteranopia, protanopia, or tritanopia.
- AC: Each variant passes a simulated colour-vision-deficiency contrast check for all status colour pairs; no v1 screen conveys state by colour alone (icon/text redundancy verified); variants are selectable in Settings alongside the base themes
- Meta: Shard=H | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-UI-05 | Const=C-03

### S-15-017 — Screen DSL v1 spec
As a firmware engineer I want a v1 specification of the declarative screen DSL so that the application layer can define screens without hand-writing widget code.
- AC: Spec defines the MVP element set (layout containers, text, icon, button, list, bindings) and explicitly defers post-M2 extensions per R15-01; every construct maps to a defined `ss_ui` widget with documented semantics on both display classes; spec is reviewed and accepted by wg-ui-ux and wg-firmware
- Meta: Shard=I | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03, C-00

### S-15-018 — Screen DSL runtime interpreter
As a firmware engineer I want a runtime interpreter for the screen DSL so that DSL-defined screens render identically to hand-built widget trees.
- AC: Interpreter renders the full v1 DSL element set with output identical to equivalent hand-coded screens (snapshot tests); malformed DSL documents fail with a diagnostic error, never a crash; interpreter overhead keeps DSL screens within the 60 FPS and 32 KB heap budgets
- Meta: Shard=I | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-UI-01 | Const=C-03, C-00

### S-15-019 — Font bundling (OFL-1.1)
As a UI engineer I want the shipped fonts curated and bundled under OFL-1.1 so that font licensing is clean and Latin plus the v1 locale scripts are covered.
- AC: Every bundled font is OFL-1.1 licensed with licence text included in the firmware notices; glyph coverage verified for EN, ES, FR catalogues (and RTL script subset staged for v1.5); font atlas sizes fit the flash budget assigned in the UI design note
- Meta: Shard=B | Type=Task | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-04 | Const=C-03, C-04

### S-15-020 — UI performance harness (60 FPS assertion)
As a firmware engineer I want an automated UI performance harness so that the 60 FPS and heap exit criteria are asserted in CI on real hardware.
- AC: Harness replays scripted interaction scenarios on Lite hardware and asserts 60 FPS partial redraws and touch-to-first-frame < 100 ms; harness asserts `ss_ui` steady-state heap ≤ 32 KB after each scenario; a regression on any assertion fails the CI run
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PERF-01 | Const=C-03, C-00
