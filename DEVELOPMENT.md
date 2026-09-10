# Developing the Ergo68 keymap

See the [README](README.md) for keyboard behavior and build/flash commands.

## Development setup

Follow QMK's [environment setup](https://docs.qmk.fm/newbs_getting_started)
and [External Userspace guide](https://docs.qmk.fm/newbs_external_userspace),
using this repository as your userspace. The Ergo68 target is already registered
in [qmk.json](qmk.json).

For the preview, install [uv](https://docs.astral.sh/uv/getting-started/installation/)
and run `uv tool install keymap-drawer`. See uv's
[tool setup](https://docs.astral.sh/uv/guides/tools#installing-tools) if `keymap`
is unavailable on `PATH`.

### Editor navigation

Follow QMK's [clangd setup](https://docs.qmk.fm/other_vscode#configuring-vs-code),
opening this userspace root. Generate its compilation database with:

```sh
qmk compile -kb yushakobo/ergo68 -km paulovcmedeiros --compiledb
```

This performs a clean firmware build.

## GitHub builds

This repo's [workflow](.github/workflows/build_binaries.yaml) builds upstream
QMK `master`; local builds use your checkout. Firmware changes pushed to `main`
or `develop` build automatically. Other branches can be run manually under
[Actions](https://github.com/paulovcmedeiros/qmk_userspace/actions). Successful
builds on `main` replace `latest`.

Keep a `.hex` tested on both halves for rollback; `latest` is replaced by
subsequent builds.

## Live keymap preview

[watch-ergo68-keymap.py](watch-ergo68-keymap.py) watches `keymap.c`,
`keymap-documentation.json`, and `keymap_drawer_config.yaml`, regenerating the
SVG and refreshing the browser. Failed renders keep the previous SVG.

Run from the userspace root with Python 3, `qmk`, and `keymap` on `PATH`:

```sh
./watch-ergo68-keymap.py           # Watch and open a browser.
./watch-ergo68-keymap.py --no-open  # Watch without opening a browser.
./watch-ergo68-keymap.py --once     # Render once and exit.
```

## Making changes

Firmware sources are in
[`keyboards/yushakobo/ergo68/keymaps/paulovcmedeiros`](keyboards/yushakobo/ergo68/keymaps/paulovcmedeiros).

| File | Edit for |
| --- | --- |
| `keymap.c`, `keymap.h` | Layout, combos, indicators, hooks, and custom keycodes |
| `typing_macros.c`, `mouse_acceleration.c`, `key_lock.c`, `screen_lock.c`, `system_actions.c`, `mode_indicators.c`, `auto_shift_toggle.c` | Custom behaviors |
| `config.h`, `rules.mk` | QMK settings and source registration |
| `keymap-documentation.json` | Diagram layer names and the Enter combo label |
| [keymap_drawer_config.yaml](keymap_drawer_config.yaml) (repo root) | Diagram parsing and styling |

Do not edit generated `keymap.svg` or `keymap-combos.yaml` directly.

Keep layout and combo definitions in `keymap.c` for the preview parser.
Preserve hook ordering: mouse acceleration, typing macros, screen lock, system
actions, then modified-Space handling; stop when a handler consumes the event.
The scan hook resolves typing timers before system-action timers.

When behavior changes, update the README and diagram metadata, then run
`./watch-ergo68-keymap.py --once` and commit the SVG. Keep the
`layers` mapping in `keymap-documentation.json` in `enum layer_names` order.
The preview does not validate firmware behavior.
