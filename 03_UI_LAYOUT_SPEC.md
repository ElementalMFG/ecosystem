# SS‑SP Universal UI — Layout Engine Specification
**Status:** Draft‑1 (normative)
**Scope:** `ss_ui` layout engine on LVGL 9.x
**Purpose:** Single UI codebase renders faithfully on every SS‑SP form factor — 3.5" rect (Lite), 2.4" landscape + 12‑LED bezel (Alpha), round watch‑style 240×240 / 320×320, tall bar 128×320, 4.3–7.0" HMI panels, e‑ink 128×296, and future HMD monochrome.

---

## 1. Design principles

1. **One codebase, every form factor.** Aspect ratio, DPI, shape, and input modality are *runtime* concerns.
2. **Content over chrome.** Chrome adapts to available pixels; content survives.
3. **Physical accessories are UI.** LED bezels, haptics, TTS, and BLE HID pendants are first‑class output/input surfaces, not afterthoughts.
4. **Graceful degradation.** No feature relies on a capability that isn't declared. Missing capability → alternate rendering path.
5. **One‑handed, gloved, dark, wet, loud.** Every screen must be usable in all of these conditions.
6. **Accessibility is default.** Screen reader (TTS), high contrast, large text, and keyboard/D‑pad navigation are always available.
7. **Deterministic layout math.** No implicit fluidity; every widget resolves size from an explicit rule.

---

## 2. Layout descriptor (the runtime contract)

Every board provides a `ss_layout_t` at boot:

```c
typedef enum {
  SS_SHAPE_RECT,     // ordinary rectangular panel
  SS_SHAPE_SQUARE,   // 1:1 rect (drawn as rect but layouts prefer symmetric)
  SS_SHAPE_ROUND,    // circular; corners are clipped
  SS_SHAPE_BAR,      // very tall or very wide (>= 3:1)
  SS_SHAPE_EINK,     // partial refresh, mono/grayscale, slow
  SS_SHAPE_HEADLESS  // no display; UI is TTS + LED + haptic + BLE remote only
} ss_shape_t;

typedef enum {
  SS_ORIENT_LANDSCAPE,
  SS_ORIENT_PORTRAIT,
  SS_ORIENT_LANDSCAPE_INV,
  SS_ORIENT_PORTRAIT_INV
} ss_orient_t;

typedef enum {
  SS_INPUT_TOUCH_SINGLE  = 1 << 0,
  SS_INPUT_TOUCH_MULTI   = 1 << 1,
  SS_INPUT_BUTTONS       = 1 << 2,   // physical D-pad / A/B / rocker
  SS_INPUT_ROTARY        = 1 << 3,   // rotary crown
  SS_INPUT_VOICE         = 1 << 4,
  SS_INPUT_BLE_HID       = 1 << 5,
  SS_INPUT_USB_HID       = 1 << 6,
} ss_input_mask_t;

typedef struct {
  ss_shape_t   shape;
  ss_orient_t  orient;
  uint16_t     w_px, h_px;
  uint16_t     dpi;                  // pixels per inch (physical)
  struct { uint8_t top,right,bottom,left; } safe_area_px;
  uint8_t      bezel_led_count;      // 0 = none, else count around perimeter
  ss_input_mask_t inputs;
  bool         color;                // false = mono / e-ink
  uint16_t     min_refresh_ms;       // 33 for 30fps TFT, 500+ for e-ink
  const char*  board_name;           // for logs
} ss_layout_t;
```

The layout descriptor is defined in `boards/<name>/board_config.h` and consumed by `ss_ui` to select templates and adjust widget rules.

### 2.1 Concrete descriptors

**Lite (CrowPanel Advance 3.5")**
```c
{ .shape=RECT, .orient=LANDSCAPE, .w_px=480, .h_px=320, .dpi=167,
  .safe_area_px={0,0,0,0}, .bezel_led_count=0,
  .inputs = TOUCH_SINGLE | BLE_HID | USB_HID | VOICE,
  .color=true, .min_refresh_ms=33, .board_name="ss-sp-lite" }
```

**Alpha 1.0 (2.4" 320×240 + 12 SK6805 ring)**
```c
{ .shape=RECT, .orient=LANDSCAPE, .w_px=320, .h_px=240, .dpi=167,
  .safe_area_px={2,2,2,2}, .bezel_led_count=12,
  .inputs = TOUCH_SINGLE | BUTTONS | VOICE | BLE_HID,
  .color=true, .min_refresh_ms=33, .board_name="ss-sp-alpha" }
```

**Round watch prototype**
```c
{ .shape=ROUND, .orient=LANDSCAPE, .w_px=240, .h_px=240, .dpi=326,
  .safe_area_px={20,20,20,20},   // circular clip
  .bezel_led_count=0,
  .inputs = TOUCH_SINGLE | ROTARY | VOICE, ... }
```

**E‑ink pager**
```c
{ .shape=EINK, .orient=PORTRAIT, .w_px=128, .h_px=296, .dpi=200,
  .safe_area_px={4,4,4,4}, .bezel_led_count=0,
  .inputs = BUTTONS, .color=false, .min_refresh_ms=800, ... }
```

**Headless gateway**
```c
{ .shape=HEADLESS, .w_px=0, .h_px=0, .bezel_led_count=0,
  .inputs = USB_HID | BLE_HID | VOICE, .color=false, ... }
```

---

## 3. Design tokens

Centralized in `ss_ui/theme.h`. Every widget consumes tokens; nothing hardcodes colors, sizes, or radii.

```c
typedef struct {
  // Colors
  lv_color_t bg, surface, surface_alt, text, text_dim, accent, warn, danger, ok;
  // Typography (LVGL font pointers)
  const lv_font_t *font_xs, *font_sm, *font_md, *font_lg, *font_xl, *font_mono;
  // Spacing scale (relative dp)
  uint8_t sp_1, sp_2, sp_3, sp_4, sp_6, sp_8, sp_12;
  // Radius scale
  uint8_t r_sm, r_md, r_lg, r_pill;
  // Widths
  uint8_t stroke_thin, stroke_med, stroke_thick;
  // Motion
  uint16_t motion_fast_ms, motion_med_ms, motion_slow_ms;
  // Focus ring
  uint8_t focus_ring_width;
  lv_color_t focus_ring;
} ss_theme_t;
```

### 3.1 Built‑in themes
- `theme_tactical_dark` — near‑black bg, high‑contrast accent, red danger.
- `theme_civilian_light` — off‑white bg, teal accent, calm.
- `theme_nightvision_red` — pure red on black; preserves scotopic vision.
- `theme_highvis` — yellow on black; for outdoor sun readability.
- `theme_accessible_xl` — huge fonts, thick focus rings, high contrast, minimal chrome.
- `theme_eink_mono` — 4‑gray palette, thicker strokes.

Themes are swappable at runtime. Every widget calls `ss_theme_get()` — never a hard‑coded literal.

### 3.2 DP unit
Sizes are expressed in **dp** (density‑independent px). Conversion:
```
px = dp * dpi / 160
```
Base dp scale: 4, 8, 12, 16, 20, 24, 32, 40, 56, 80.

---

## 4. Layout primitives

### 4.1 Region system
`ss_ui` divides the screen into semantic **regions**. Which regions exist depends on shape:

**Rect (Lite / Alpha)**
```
+------------------------------------+
| STATUS_BAR                         |  h = 40 dp (auto-fit)
+------------------------------------+
|                                    |
|             CONTENT                |  fills remainder
|                                    |
+------------------------------------+
| NAV_BAR                            |  h = 56 dp (or hidden if D-pad exists)
+------------------------------------+
```

**Round**
```
       .............
     .   RING_LEDs   .
   . +---------------+ .
  .  |               |  .
 .   |    CONTENT    |   .
  .  |               |  .
   . +---------------+ .
     . STATUS_ARC .
       .........
```
No nav bar; navigation is rotary or edge‑swipe. Status is drawn as an arc around the top.

**Bar (128×320 or similar)**
Single column, no nav bar, content = whole panel. Status is a thin top strip. Regions collapse into an accordion.

**E‑ink**
Static regions with minimal repaints. Nav shown as physical button labels along the bottom edge.

**Headless**
Regions map to TTS phrases and LED patterns. There is no visual layout — but the *same* app state renders faithfully.

### 4.2 Adaptive nav
- **Bottom bar** on rect landscape 480×320 (Lite portrait/landscape both work).
- **Right rail (icon column)** on rect landscape 320×240 (Alpha — saves the horizontal space for map/chat).
- **Rotary menu** on round.
- **Long‑press status arc** on bar (there's no room for a bar).
- **Physical button legend** on e‑ink.

The app declares which nav model it prefers; `ss_ui` picks the best available.

### 4.3 Adaptive typography
Font sizes chosen from a scale that adjusts to DPI and physical width:

```c
lv_font_t* ss_pick_font(ss_font_role_t role, ss_layout_t* L);
// role in { HEADLINE, TITLE, BODY, CAPTION, MONO, HUGE }
```

On dense small screens (round watch), roles collapse (HEADLINE == TITLE). On e‑ink, HEADLINE gets a heavier weight for contrast.

### 4.4 Content templates
Common patterns are shipped as ready‑made templates each app can drop in:
- `tpl_list` — vertical scroll list (chat, roster, waypoints).
- `tpl_split` — 2‑pane list + detail (large panels only; auto‑collapses on small).
- `tpl_map` — full‑bleed map with floating actions.
- `tpl_dial` — center content + radial actions (SOS, PTT).
- `tpl_form` — settings, provisioning.
- `tpl_hud` — Seekie compass overlay + peripheral data.
- `tpl_carousel` — for round shape swipe‑paged content.

Each template respects region rules and consumes the theme.

### 4.5 Focus + navigation graph
Every screen has a **focus graph** so it works with:
- Touch (tap on a widget).
- D‑pad (BUTTONS: up/down/left/right/select/back).
- Rotary (turn = next/prev focus in ring; press = select).
- Voice ("Focus reply. Send.").
- BLE HID / USB HID.

`ss_ui` maintains focus state per app; widgets declare focus neighbors via `ss_widget_link(a, b, DIR_RIGHT)` etc.

---

## 5. Multi‑surface output

Every app produces a **UIState** object each frame:
```c
typedef struct {
  ss_visual_tree_t*  visual;   // LVGL object tree
  ss_led_pattern_t*  leds;     // bezel LED effect
  ss_haptic_seq_t*   haptic;   // vibration/buzz
  ss_tts_utterance_t*tts;      // spoken text (if voice)
  ss_focus_state_t*  focus;    // current focus + graph
} ss_ui_state_t;
```

`ss_ui` dispatches each field to the appropriate output:
- `visual` → LVGL renderer (skipped on HEADLESS).
- `leds` → bezel driver (drawn as on‑screen ring if no physical LEDs).
- `haptic` → LRA / buzzer (queued if none available).
- `tts` → speech synth if enabled or if HEADLESS.

Result: **the same app runs on a display or on a blind gateway** — because it emits the same UIState.

---

## 6. Input abstraction

`ss_input_event_t` normalizes every input:
```c
typedef enum {
  SS_IN_TOUCH_DOWN, SS_IN_TOUCH_UP, SS_IN_TOUCH_MOVE,
  SS_IN_BUTTON_DOWN, SS_IN_BUTTON_UP,
  SS_IN_ROTARY,
  SS_IN_VOICE_CMD,
  SS_IN_HID_KEY,
  SS_IN_HW_WAKE,
} ss_in_kind_t;

typedef struct {
  ss_in_kind_t kind;
  uint32_t     ts_ms;
  union {
    struct { int16_t x, y; }         touch;
    struct { uint16_t code; }        button;    // enum SS_BTN_*
    struct { int16_t delta; }        rotary;
    struct { const char* utterance; float confidence; } voice;
    struct { uint16_t keycode; uint8_t mods; }  hid;
  } as;
} ss_input_event_t;
```

Voice commands are compiled to a small grammar per screen (`ss_voice_bind("reply", CMD_REPLY)`), and dispatched as `SS_IN_VOICE_CMD`.

---

## 7. Widget rules

- Every widget accepts a **min/preferred/max** size in dp.
- Every widget declares a **shrink priority** (higher shrinks first when space is tight).
- Widgets can be **collapsed** to a compact icon+label form (nav bar / bar shape / small round).
- Every interactive widget has a **role** (`ss_role_t`) for the screen reader.
- Every widget must survive a **200 ms freeze** — no animation is critical to comprehension.

### 7.1 Core widgets shipped
- `ss_button` — pill or icon
- `ss_list_item` — icon + primary + secondary + trailing
- `ss_chat_bubble` — self / peer / system
- `ss_toast` — transient banner
- `ss_dial_action` — circular action (SOS, PTT)
- `ss_status_dot` — signal / battery / mesh
- `ss_map` — tile renderer
- `ss_compass` — Seekie needle + peer markers
- `ss_ring_indicator` — physical LED bezel mirror
- `ss_progress` — linear + circular
- `ss_form_field` — text/number/switch/select
- `ss_qr` — QR code renderer
- `ss_barcode` — reader (with camera-less fallback via serial)
- `ss_voice_wave` — mic input visualization

---

## 8. Screen inventory (v1)

Every app shells to a small set of screens. Each screen lists which templates it uses and which regions are populated.

### 8.1 Home
- Rect: `tpl_carousel` of apps (Chat, Voice, Seekie, Map, SOS, Roster, Settings). Nav bar visible.
- Round: `tpl_dial` — icons arrayed on a ring, rotary selects.
- Bar: vertical list, physical buttons page.

### 8.2 Chat
- List of channels + DMs.
- Selecting a channel → `tpl_list` of messages + input bar.
- On Alpha/Lite touch keyboard; on round: dictate‑only + suggested replies.
- Voice: press‑hold on any message → TTS reads aloud; long‑press → reply via dictation.

### 8.3 Voice / PTT
- `tpl_dial` — big Talk button center; peer/channel top; VU meter around.
- Lite: shows "Half‑duplex" badge; the mux state machine visualized as two flip states.
- Alpha: shows codec (Opus) and full‑duplex option; bezel LEDs mirror VU.

### 8.4 Seekie
- `tpl_hud` — north‑up map background, compass needle rotates to target, peripheral data on cardinal edges.
- Round: full circular compass; needle to target; peer glyphs orbit.
- Bezel LEDs (Alpha): pulse blue toward the target; on Lite the on‑screen ring does the same.

### 8.5 Map
- `tpl_map` — offline tiles (PMTiles preferred; MBTiles supported).
- Waypoints, tracks, mesh peer positions, geofences.
- Long‑press → set waypoint / share to channel.

### 8.6 SOS
- `tpl_dial` — hold red button for 3 s → confirm → broadcasting.
- While broadcasting: flashing status bar + LED ring + repeating haptic + TTS confirmation.
- Cancel screen requires two confirmations.

### 8.7 Roster
- `tpl_list` of known peers with online/last‑seen, RSSI, distance, note.
- Detail view: identity fingerprint (QR), start chat, start voice, Seekie‑point.

### 8.8 Signal / Mesh graph
- `tpl_hud` — force‑directed graph of peers and bearer edges.
- Colors: bearer type; thickness: bandwidth; opacity: freshness.

### 8.9 Settings
- `tpl_form` — provisioning, region, keys, backups, OTA, plugin manager.
- QR export/import for offline config transfer.

### 8.10 Provisioning wizard
- First‑boot: language → region → identity generate → optional callsign → key backup QR → optional companion‑phone pair.

---

## 9. Motion, sound, haptics

- **Motion budget:** ≤ 250 ms per transition; no gratuitous animation. Reduced‑motion switch available (also auto‑engages under low battery or e‑ink).
- **Haptic library:**
  - `HAPTIC_TICK` — 20 ms focus change
  - `HAPTIC_ACK` — 40 ms
  - `HAPTIC_ERROR` — 40‑40‑40 ms triple
  - `HAPTIC_INCOMING` — 80‑pause‑80 ms
  - `HAPTIC_SOS` — dot‑dot‑dot‑dash‑dash‑dash‑dot‑dot‑dot (Morse SOS)
  - `HAPTIC_DIRECTION_TICK` — 10 ms micro‑pulse for Seekie updates
- **Sound library:** same set as haptic + optional TTS.
- **Bezel LED patterns:**
  - `LED_BREATHE_OK` (green), `LED_BREATHE_LOST` (red), `LED_NORTH` (white on north LED)
  - `LED_VU_METER` (bottom→top mic gain)
  - `LED_DIR_POINT(angle)` (single blue LED at bearing)
  - `LED_SOS_SWEEP` (red rotating)
  - `LED_INCOMING` (single amber blink at source direction if known)

---

## 10. Accessibility

- **Screen reader:** every widget has a `ss_role_t` + label; on focus change, TTS reads it.
- **Voice control:** every screen exposes a small grammar; grammar auto‑docs are shown on request.
- **High contrast + XL font mode** toggles; layouts must still reflow.
- **Reduced motion** and **reduced flashing** (SOS mode too — no strobes above 3 Hz outside SOS).
- **Color‑blind safe palettes** — no info encoded in color alone; icons/labels back everything.
- **Left‑handed mode** — mirrors nav rails.

---

## 11. Internationalization

- All user‑visible strings via `i18n_t("KEY")`; catalog under `firmware/i18n/<lang>.json`.
- **Launch languages:** EN, ES, FR, DE, JA, AR, ZH‑Hans.
- RTL: LVGL supports it; layout templates test both.
- Fonts include Latin + Arabic + CJK subsets (subset packs on SD to save flash).
- TTS voices switchable per language when Piper voice packs are on SD.

---

## 12. Testing

- **Snapshot tests** per screen per layout descriptor. LVGL fbuf → PNG → diff.
- **Golden layouts** for: Lite 480×320 L, Lite 320×480 P, Alpha 320×240 L, round 240×240, bar 128×320, e‑ink 128×296.
- **Interaction tests** for each nav model (touch, D‑pad, rotary, voice).
- **Screen reader smoke test**: every screen produces sensible TTS on focus walk.
- **Perf budget**: ≥ 30 fps interactive; ≥ 60 fps preferred on Alpha; e‑ink relaxed to partial‑refresh budget.

---

## 13. Files and modules

```
components/ss_ui/
├── ss_ui.h                  # public API
├── layout/
│   ├── ss_layout.c/.h       # ss_layout_t + region math
│   ├── templates/
│   │   ├── tpl_list.c
│   │   ├── tpl_split.c
│   │   ├── tpl_map.c
│   │   ├── tpl_dial.c
│   │   ├── tpl_form.c
│   │   ├── tpl_hud.c
│   │   └── tpl_carousel.c
├── theme/
│   ├── ss_theme.c/.h
│   ├── theme_tactical_dark.c
│   ├── theme_civilian_light.c
│   ├── theme_nightvision_red.c
│   ├── theme_highvis.c
│   ├── theme_accessible_xl.c
│   └── theme_eink_mono.c
├── widgets/
│   ├── ss_button.c
│   ├── ss_list_item.c
│   ├── ss_chat_bubble.c
│   ├── ss_dial_action.c
│   ├── ss_map.c
│   ├── ss_compass.c
│   ├── ss_ring_indicator.c
│   ├── ss_status_dot.c
│   ├── ss_qr.c
│   └── ...
├── input/
│   ├── ss_input.c/.h
│   ├── input_touch.c
│   ├── input_buttons.c
│   ├── input_rotary.c
│   ├── input_voice.c
│   └── input_hid.c
├── output/
│   ├── ss_leds.c            # physical or virtual ring
│   ├── ss_haptic.c
│   └── ss_tts.c
├── i18n/
│   ├── i18n.c/.h
│   └── catalogs/*.json
└── tests/
    ├── snapshot/
    └── nav/
```

---

## 14. Summary

`ss_ui` is a small, disciplined layout engine on top of LVGL 9 that treats **shape, DPI, inputs, and output surfaces as runtime data**. Every screen emits a `ss_ui_state_t` (visual + LEDs + haptics + TTS + focus). Every widget consumes a theme and a layout descriptor. Templates and themes cover the common cases; capability flags handle the edge cases. This is how one codebase serves 3.5" Lite, 2.4" Alpha with bezel, round watches, tall bar pagers, e‑ink, and headless gateways with no per‑device forks.
