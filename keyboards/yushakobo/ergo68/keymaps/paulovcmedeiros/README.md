# Paulo Medeiros's Yushakobo Ergo68 keymap

![Yushakobo Ergo68 keymap](keymap.svg)

This is my custom [QMK Firmware](https://docs.qmk.fm/) keymap for the
[Yushakobo Ergo68](https://shop.yushakobo.jp/products/ergo68). Its upstream
keyboard definition lives at
[`keyboards/yushakobo/ergo68`](https://github.com/qmk/qmk_firmware/tree/master/keyboards/yushakobo/ergo68).

## Host keyboard layout

This keymap is designed for the
[EurKEY](https://eurkey.steffen.bruentjen.eu/) host layout. This choice is
deliberate:

- Its base and shifted symbol positions are compatible with US QWERTY, which
  keeps programming punctuation convenient.
- Its AltGr layer supports my everyday writing in Portuguese and
  Swedish.
- It avoids making common programming characters into dead keys, as happens
  with traditional US International layouts.

Consequently, keycodes such as `S(KC_3)`, `S(KC_4)`, `KC_LBRC`, and `KC_BSLS`
refer to US-compatible key positions rather than layout-independent
characters. QMK's default
[`SEND_STRING`](https://docs.qmk.fm/features/send_string) translation relies
on the same compatibility. Right Alt serves as AltGr, whose combinations
depend specifically on EurKEY.

Plain US should produce the intended ASCII letters and symbols, but it will not
preserve EurKEY's AltGr and multilingual behavior. Using another host layout
requires auditing the keymap's printable keycodes and all `SEND_STRING` macros.

## Notable behavior

- The thumb Space keys are dual-role. Tapping one while Shift or Alt is held
  sends Backspace instead.
- The Mouse and Symbols thumb keys form Enter. This combo works only on Base,
  must be tapped within its 30 ms chord window, and is disabled while Shift or
  Alt is held so that modified-Space Backspace takes priority.
- Holding a mouse direction after a double tap within 100 ms uses maximum
  pointer acceleration.
- The custom punctuation keys append Space when held for 150 ms. The custom
  slash and minus keys select `/` versus `~/`, and `-` versus ` -`, by tap or
  hold.
- Both Shifts activate Caps Word. Auto Shift can be toggled, and Key Lock locks
  the next basic key until that key or Key Lock is pressed again.
- System keys require a two-second hold. Make types and submits the QMK compile
  command, Shift+Make does the same for flash, Ctrl+Shift+Make enters the
  bootloader, EEPROM Clear erases persisted QMK settings and restarts the
  keyboard, and Build Dates types both halves' compilation timestamps. While a
  system key is arming, the other LEDs turn off and the Numpad indicator blinks
  red; after confirmation, it turns yellow briefly before the action runs.

> **Caution:** Make and Shift+Make type a command followed by Enter. Use them
> only while a trusted terminal is focused.

## Build and flash

From a configured QMK environment, build or build-and-flash with:

```sh
qmk compile -kb yushakobo/ergo68 -km paulovcmedeiros
qmk flash -kb yushakobo/ergo68 -km paulovcmedeiros
```

The upstream definition targets a Pro Micro-compatible controller. Flash the
same firmware to each half separately, resetting the half being flashed when
QMK prompts. See QMK's [flashing guide](https://docs.qmk.fm/newbs_flashing).

Shift+Make first looks for
`$(qmk userspace-path)/yushakobo_ergo68_paulovcmedeiros.hex`; if it is absent, it
falls back to the build-and-flash command above.

In [External QMK Userspace](https://docs.qmk.fm/newbs_external_userspace),
register and build the target with:

```sh
qmk userspace-add -kb yushakobo/ergo68 -km paulovcmedeiros
qmk userspace-compile
```

## Live keymap preview

The userspace-root `watch-ergo68-keymap.py` is a small development helper for
this keymap. It renders `keymap.c` to `keymap.svg`, watches the source for
changes, and updates a browser preview after each successful render. It also
watches
`keymap-documentation.json`, which provides human-readable layer names, rich
combo labels, and the behavior notes appended to the SVG. The userspace-root
`keymap_drawer_config.yaml` controls keymap-drawer's parsing, key labels, SVG
dimensions, and styling. Changes to either file trigger a redraw. The layer
mapping in `keymap-documentation.json` also lets `keymap.c` use the layer enum
identifiers directly inside QMK's
[`LT(...)`](https://docs.qmk.fm/feature_layers) expressions while retaining
keymap-drawer's layer highlighting.

The script extracts the simple
[QMK combo](https://docs.qmk.fm/features/combo) definitions used here into
`keymap-combos.yaml` so that they appear in the drawing. The combo labels use
the `combo_labels` mapping in `keymap-documentation.json`. If conversion,
metadata validation, or rendering fails, the previous SVG is kept in place.

The browser preview uses `http://127.0.0.1:8000` when that port is available.
If another process is already using it, the watcher automatically selects an
available port instead and prints the resulting URL.

The script mainly coordinates existing tools rather than implementing a
keymap renderer. It uses the [QMK CLI](https://docs.qmk.fm/cli), specifically
[`qmk info`](https://docs.qmk.fm/cli_commands#qmk-info) and
[`qmk c2json`](https://docs.qmk.fm/cli_commands#qmk-c2json), to obtain the
Ergo68 geometry and convert the C keymap to JSON. It then relies on the
`keymap` command provided by the
[`keymap-drawer`](https://github.com/caksoylar/keymap-drawer) package to parse
the JSON and draw the SVG. Python 3, `qmk`, and `keymap` must therefore be
available on `PATH`.

Run the watcher from the userspace root with:

```sh
./watch-ergo68-keymap.py
```

Useful alternatives are:

```sh
./watch-ergo68-keymap.py --no-open  # Watch without opening a browser.
./watch-ergo68-keymap.py --once     # Render once and exit.
```

This utility is tailored to this Ergo68 keymap and its current combo syntax. It
is a visual aid, not a compiler or substitute for firmware validation.

## Making changes

This firmware does not enable [VIA](https://www.usevia.app/), so live remapping
with VIA or [Remap](https://github.com/remap-keys/remap) is unavailable. Layout
changes must be made in `keymap.c`, compiled, and flashed. The SVG preview tool
provides quicker visual feedback during that process.

| File | Purpose |
| --- | --- |
| `keymap.c`, `config.h`, `rules.mk` | Firmware mapping, behavior, and configuration |
| `keymap-documentation.json` | Layer names, combo labels, and behavior notes |
| `keymap_drawer_config.yaml` (userspace root) | Key labels and keymap-drawer parsing and SVG options |
| `keymap.svg` | Generated diagram; regenerate and commit it |
| `keymap-combos.yaml` | Generated intermediate; never edit it |

The `layers` mapping in `keymap-documentation.json` must follow the order of
`enum layer_names` in `keymap.c`. The watcher does not infer arbitrary behavior
from the C event hooks, so update its config labels and documentation notes when
those behaviors change. Regenerate once without starting the live viewer with:

```sh
./watch-ergo68-keymap.py --once
```
