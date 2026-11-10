DSView UI style guide
=====================

How DSView should look, and the rules that keep it that way. The values
below are the ones in `theme.qss`, the palettes and the code; when you
change one, change it here too.

See it: `./build.dir/DSView --gallery` shows every control in the
current theme (see README.md, "UI gallery").

Principles
----------

1. **Calm chrome, loud data.** Window, toolbars and panels stay in greys;
   colour is for the signals, the channel tags and the one action that
   matters on screen.
2. **Native where the desktop decides.** The system title bar on Linux,
   the platform's button order in dialogs.
3. **Cards on a backdrop.** The waveform area, the side panels and dialogs
   are rounded cards (`@bg`) on the darker window (`@chrome`).
4. **One blue button.** In a dialog only the button that does the thing
   (OK, Yes, Save) is `@accent`. Everything else is a plain button.
5. **Icons alone only with a tooltip.** An icon-only button always has
   its text as tooltip.
6. **One place for the look.** Colours, fonts, borders and padding come
   from `theme.qss` and the palettes, not from code.

Colours
-------

Use palette tokens (`@name`), never hex values, in `theme.qss`. In C++
(self-drawn widgets) read them with `ui::Theme::Instance().color("name")`.

| Token | Use it for |
|-------|------------|
| `@chrome` | window backdrop, toolbars, activity bar, status bar |
| `@bg` | cards: waveform area, side panels, dialogs |
| `@surface` | inputs and buttons |
| `@hover` | hover fill, inner dividers |
| `@divider` | lines between chrome areas, under panel headers |
| `@border` | borders of inputs, buttons, group boxes |
| `@border-hover` | input border on hover |
| `@handle` | scroll bar handles, strong lines |
| `@text` | normal text |
| `@text-bright` | text on hover / selection |
| `@text-muted` | secondary text: hints, units, bit numbers, headers of tables, status bar |
| `@text-disabled` | disabled controls |
| `@accent` (+ `-hover`, `-pressed`) | the default button, focus border, checked state, slider, progress |
| `@on-accent` | text and icons on `@accent` |
| `@selection` | selected rows and menu items |
| `@link` | links in text (labels, text browsers) |
| `@danger` (+ `-hover`) | destructive actions and errors |
| `@close-hover` | window close button only |

Not from the palette, on purpose: the channel colours (`PROBE_COLORS`),
the decoder annotation colours and the trigger / cursor marker colours.
They identify data and stay the same in both themes.

Typography
----------

One family (the system UI font, or the one picked in Display options) and
**one size**: `appOptions.fontSize` (default 9 pt, set by the user).
Hierarchy comes from weight and colour, not from size.

| Level | Size | Weight | Colour | Examples |
|-------|------|--------|--------|----------|
| Heading | base | 600 | `@text` | dialog title, panel header, group box title, message box heading |
| Body | base | normal | `@text` | labels, values, menu items |
| Secondary | base | normal | `@text-muted` | hints (`QLabel#hint`), units, bit numbers, message box detail |
| Monospace | base | normal | `@text` | none yet; for hex values if needed |

Rules:

- The app font (`QApplication::setFont`) gets the base size whenever the
  font options change (`UiManager::Update(UI_UPDATE_ACTION_FONT)`), so
  menus, tooltips and new widgets match without extra code.
- Dialogs and panels also apply it with `ui::set_form_font()` /
  `update_font()`, which sets an explicit normal weight. Do not add `+1`
  or other sizes.
- Bold in QSS (`font-weight: 600`), not with `QFont::setBold` in code,
  so the theme can change it. A bold label: object name `heading`.
  Group box titles are bold through the `QGroupBox` rule.
- The "UI gallery" title is the only text bigger than base.

Spacing
-------

All gaps come from this scale: **2 · 4 · 6 · 8 · 12 · 16** px.

| Where | Value |
|-------|-------|
| Icon and its text inside one control | 2–4 |
| Controls in one row of a side panel (e.g. field, icon button, field) | 6 |
| Controls in one row of a dialog; check box / radio indicator and text | 8 |
| Between rows of a form | 8 |
| Label column and field column of a form | 12 |
| Between sections (groups, cards) in a dialog or panel | 12 |
| Inside a group box / card (left, top, right, bottom) | 12, 10, 12, 12 |
| Dialog content margins (left, top, right, bottom) | 16, 4, 16, 16 |
| Between the waveform card, the side panels and the window edge | 4–6 (card `margin` and `QMainWindow::separator` in `theme.qss`) |

Rules:

- Let layouts size things. No `setFixedHeight` / `setFixedSize` /
  spacer widgets to push content around; use the layout's spacing and
  `addStretch`.
- Allowed: a `setMinimumWidth` so a dialog or field does not get too
  narrow, and widths computed from the font (`QFontMetrics`) so columns
  line up. Not allowed: fixed heights and fixed overall sizes; they clip
  text at bigger font sizes.
- A dialog is as big as its content (`sizeHint`). It only scrolls when the
  content is taller than 70% of the screen (see `DeviceOptions`). A dialog
  with long text (About) gets a starting size with `resize()`, not a fixed
  one.
- A dialog without a default button gives the first focus to its content,
  not to a button, so no button looks selected when it opens.

Shapes
------

| Element | Corner radius |
|---------|---------------|
| Inputs, push buttons, check boxes | 4 |
| Tool buttons (toolbar, activity bar, panels) | 6 |
| Cards, group boxes, menus | 8 |
| Window frame, dialog frame | 10 |

Borders are 1 px. The focus border is `@accent`. No drop shadows inside
the window; dialogs get the one from `Shadow`.

Components
----------

**Push buttons.** Plain by default. In a `QDialogButtonBox`: min width
72, padding 5 × 12. Exactly one default (blue) button per dialog.

**Dialog buttons.** Always a `QDialogButtonBox` at the bottom right, in
its own row of the dialog layout. Button order is the platform's
(`MyStyle`: GNOME order on Linux, so Cancel / No on the left, the action
at the right end). Do not build OK / Cancel rows by hand.

**Dialogs.** Derive from `DSDialog`: title bar (title left, DemiBold),
then content in group boxes (`QGroupBox` with a title, `QFormLayout`
inside), then the button box. No icon in the title bar.

**Message boxes.** `MsgBox::Show` / `MsgBox::Confirm` (`DSMessageBox`).
No icon. Text: one sentence that says what happened or what is asked
("Save captured data?"). Details, if any, go in the informative text;
then the first line is shown as a heading and the detail muted. Yes / OK
is the default.

**Side panels.** A `panel_card` with a header label, then the content
with the side panel row spacing (6) and 12 between sections. Content
follows the panel width (`setWidgetResizable(true)`); a panel never
scrolls sideways: `PanelCard` turns the horizontal scroll bar off and
keeps the card as wide as the content's minimum width. Section titles
inside a panel: `QLabel#heading` or a `QGroupBox` title, no trailing
colon. Column names above a list: `QLabel#hint`.

**Icon-only buttons.** `QToolButton`, tooltip = the button's text.
Icons are 24 px in the toolbar and activity bar, 16 px inside panels
and dialogs.

**Secondary labels.** `QLabel` with object name `hint`
(`setObjectName("hint")`). **Bold labels:** object name `heading`.

**Styled variants.** Distinguish them by object name and give them a rule
in `theme.qss`; do not call `setStyleSheet` in code (exceptions: colours
that come from data, like the palette swatches in the gallery). Existing
names: `hint`, `heading`, `trigger_hint`, `inline_search_icon`, `panel_card`,
`panel_card_header`, `central_card`, `dialog_frame`, `window_frame`,
`app_status_bar`, `main_bar_row`, `activity_bar`.

Icons
-----

Generated by `DSView/icons/genicons.py` into `icons/dark` and
`icons/light` (`python3 icons/genicons.py icons`).

- 24 × 24 viewBox, 1.5 px stroke, round caps and joins, no fill.
- Colour `#cccccc` (dark) / `#3b3b3b` (light), the palette's `@text`.
- Exceptions with colour: `start` (green), `stop` (red), `single`
  (blue); the mode waves `la`, `daq`, `osc` use a 28 × 14 viewBox.
- New icon: add it to `genicons.py`, regenerate, add both files to
  `DSView.qrc`, check it in the gallery's icon card.

Self-drawn parts
----------------

Waveforms, the ruler, channel rows and cursors are painted in C++.

- Chrome colours from `Theme::color()`; data colours as listed above.
- Interactive marks (trigger buttons on channel rows) show only the
  current state until hovered.
- Logic waves: the channel colour (`LogicSignal::wave_colour`, lightness
  clamped per theme), a light tint of it under high levels, edges closer
  than a pixel drawn lighter so bursts read as a band. The lane line is
  a faint line at the low level in the channel colour.
- Decoder rows: annotations are rounded bars with a 1 px gap; the text
  shrinks to the row height. Row titles are chips (`@bg`, muted text) on
  top of the annotations. "Zoom in for details" is muted.
- Decoder rows are at least `View::MinDecoderRowHeight` (18 px); logic
  rows give up height for them down to 14 px, then the channels scroll
  (wheel over the names, Shift + wheel over the waves).
- The row under the mouse (over the waves or its name) gets a faint
  `@hover` band (`Viewport::paintHoverRow`).
- Floating measurement (logic): a card like a menu (`@surface`,
  `@border`, radius 8) 14 px below right of the pointer, flipped at the
  view edges; title = channel name with its colour dot, names
  `@text-muted` left, values `@text` right (`Viewport::paintMeasureCard`).
- Leave room for labels at both ends of the time axis (`View::edge_pad`).
  Code that turns view pixels into sample indexes must cope with the
  negative offset there.

Text
----

- UI strings go through the language files (`L_S(...)`, `lang/en` and
  `lang/cn`); add both.
- Labels: title case as in the existing strings ("Trigger Position").
  Messages: sentence case, ending with a full stop or a question mark.
- Units after values with a space: `1.00 s`, `25 MHz`, `3.3 V`.

Before you commit a UI change
-----------------------------

1. `./build.dir/DSView --gallery-shot /tmp/before` on the old build,
   `--gallery-shot /tmp/after` on the new one; compare both themes.
2. Open the screens you touched in dark and light.
3. Check a narrow window: nothing clipped, no horizontal scroll in panels.
4. If you added a colour, size or variant, add it to this guide.

Known deviations (to fix)
-------------------------

Found with the gallery; the code does not follow the guide here yet.

- `Calibration` (DSCope only) was moved to natural sizes without being
  checked on a device.
- `WaitingDialog` keeps its fixed size (a small busy box).
- The channel rows, ruler and waveform labels use their own font options
  (Display options → fonts), not the base size. Intended for now.
