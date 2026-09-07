#!/usr/bin/env python3

"""Render and live-reload the keymap diagram for this Yushakobo Ergo68 keymap."""

from __future__ import annotations

import argparse
import errno
import hashlib
import json
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import time
import webbrowser
from html import escape
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

KEYBOARD = "yushakobo/ergo68"
KEYBOARD_DISPLAY_NAME = "Yushakobo Ergo68"
KEYMAP_NAME = "paulovcmedeiros"
LAYOUT_NAME = "LAYOUT"
USERSPACE_ROOT = Path(__file__).resolve().parent
KEYMAP_DIR = (
    USERSPACE_ROOT
    / "keyboards"
    / "yushakobo"
    / "ergo68"
    / "keymaps"
    / KEYMAP_NAME
)
PROGRAM_NAME = "watch-ergo68-keymap"
POLL_INTERVAL_SECONDS = 0.5
DEFAULT_PREVIEW_PORT = 8000
STACKED_COMBO_OFFSET = 0.5

LIVE_VIEW_HTML = """\
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{keyboard_name} keymap</title>
  <style>
    html, body {
      margin: 0;
      min-height: 100%;
      background: #ddd;
    }

    #keymap {
      display: block;
      width: 100%;
      height: auto;
    }
  </style>
</head>
<body>
  <img id="keymap" src="keymap-live.svg" alt="{keyboard_name} keymap">
  <script>
    const keymap = document.getElementById("keymap");

    function refreshKeymap() {
      const nextImage = new Image();
      nextImage.onload = () => {
        keymap.src = nextImage.src;
        window.setTimeout(refreshKeymap, 500);
      };
      nextImage.onerror = () => window.setTimeout(refreshKeymap, 500);
      nextImage.src = `keymap-live.svg?updated=${Date.now()}`;
    }

    window.setTimeout(refreshKeymap, 500);
  </script>
</body>
</html>
""".replace("{keyboard_name}", KEYBOARD_DISPLAY_NAME)


DEFAULT_SOURCE = KEYMAP_DIR / "keymap.c"
DEFAULT_OUTPUT = KEYMAP_DIR / "keymap.svg"
DEFAULT_COMBOS_OUTPUT = KEYMAP_DIR / "keymap-combos.yaml"
DOCUMENTATION_SOURCE = KEYMAP_DIR / "keymap-documentation.json"
DRAWER_CONFIG_SOURCE = USERSPACE_ROOT / "keymap_drawer_config.yaml"


def log(message: str, *, error: bool = False) -> None:
    """Write a prefixed message to standard output or standard error."""
    stream = sys.stderr if error else sys.stdout
    print(f"{PROGRAM_NAME}: {message}", file=stream, flush=True)


def parse_args() -> argparse.Namespace:
    """Parse the command-line arguments."""
    parser = argparse.ArgumentParser(
        prog=PROGRAM_NAME,
        description=(
            f"Watch a {KEYBOARD_DISPLAY_NAME} keymap.c and redraw its "
            "keymap-drawer SVG after each change."
        ),
    )
    parser.add_argument(
        "--once",
        action="store_true",
        help="render once and exit instead of watching",
    )
    parser.add_argument(
        "--no-open",
        action="store_true",
        help="do not open the live SVG view in the default browser",
    )
    parser.add_argument("source", nargs="?", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("output", nargs="?", type=Path, default=DEFAULT_OUTPUT)
    return parser.parse_args()


def find_commands() -> dict[str, str]:
    """Locate the external commands required for rendering."""
    names = ["qmk", "keymap"]

    commands: dict[str, str] = {}
    for name in names:
        path = shutil.which(name)
        if path is None:
            raise RuntimeError(f"required command not found: {name}")
        commands[name] = path
    return commands


def file_signature(path: Path) -> bytes | None:
    """Return a content hash, or None when the file does not exist."""
    try:
        return hashlib.sha256(path.read_bytes()).digest()
    except FileNotFoundError:
        return None


def copy_atomically(source: Path, destination: Path) -> None:
    """Copy a file and atomically replace the destination."""
    temporary = destination.with_name(f"{destination.name}.new")
    shutil.copy2(source, temporary)
    temporary.replace(destination)


def split_c_arguments(arguments: str) -> list[str]:
    """Split a comma-separated C argument list without splitting nested calls."""
    parts: list[str] = []
    start = 0
    depth = 0

    for index, character in enumerate(arguments):
        if character == "(":
            depth += 1
        elif character == ")":
            depth -= 1
        elif character == "," and depth == 0:
            parts.append(arguments[start:index].strip())
            start = index + 1

    parts.append(arguments[start:].strip())
    return parts


def normalize_keycode(keycode: str) -> str:
    """Normalize whitespace so C and c2json keycode expressions compare equally."""
    return re.sub(r"\s+", "", keycode)


def load_documentation(path: Path) -> dict[str, object]:
    """Load and minimally validate the declarative drawing metadata."""
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise ValueError("documentation root must be an object")

    layers = data.get("layers")
    if not isinstance(layers, dict) or not layers or not all(
        isinstance(identifier, str)
        and identifier
        and isinstance(label, str)
        and label
        for identifier, label in layers.items()
    ):
        raise ValueError("layers must map enum identifiers to display labels")

    combo_labels = data.get("combo_labels")
    if not isinstance(combo_labels, dict) or not all(
        isinstance(keycode, str)
        and keycode
        and isinstance(label, str)
        and label
        for keycode, label in combo_labels.items()
    ):
        raise ValueError("combo_labels must map keycodes to display labels")

    return data


def resolve_layer_taps(keymap_json: bytes, layer_identifiers: list[str]) -> bytes:
    """Replace enum identifiers in LT() expressions for keymap-drawer."""
    keymap_data = json.loads(keymap_json)
    for layer in keymap_data["layers"]:
        for position, keycode in enumerate(layer):
            for layer_number, identifier in enumerate(layer_identifiers):
                keycode = re.sub(
                    rf"\bLT\(\s*{re.escape(identifier)}\s*,",
                    f"LT({layer_number},",
                    keycode,
                )
            layer[position] = keycode
    return json.dumps(keymap_data).encode()


def keycode_label(keycode: str, combo_labels: dict[str, str]) -> str:
    """Return a documented combo label, falling back to a compact keycode name."""
    if keycode in combo_labels:
        return combo_labels[keycode]
    return keycode.removeprefix("KC_").replace("_", " ")


def generate_combos_overlay(
    source: Path,
    keymap_json: bytes,
    output: Path,
    base_layer_name: str,
    combo_labels: dict[str, str],
) -> None:
    """Extract simple QMK COMBO definitions and write a keymap-drawer overlay."""
    source_text = source.read_text(encoding="utf-8")
    keymap_data = json.loads(keymap_json)
    base_layer = keymap_data["layers"][0]

    combo_arrays = {
        name: [
            keycode
            for keycode in split_c_arguments(contents)
            if keycode and keycode != "COMBO_END"
        ]
        for name, contents in re.findall(
            r"const\s+uint16_t\s+PROGMEM\s+(\w+)\s*\[\]\s*=\s*\{(.*?)\}\s*;",
            source_text,
            flags=re.DOTALL,
        )
    }

    combo_table_match = re.search(
        r"combo_t\s+key_combos\s*\[\]\s*=\s*\{(.*?)\}\s*;",
        source_text,
        flags=re.DOTALL,
    )
    combo_entries: list[tuple[list[int], str, str | None, float | None]] = []
    occupied_midpoints: set[tuple[int, int]] = set()
    if combo_table_match is not None:
        for array_name, result_keycode in re.findall(
            r"\[[^]]+\]\s*=\s*COMBO\s*\(\s*(\w+)\s*,\s*([^,)]+(?:\([^)]*\))?)\s*\)",
            combo_table_match.group(1),
        ):
            if array_name not in combo_arrays:
                raise ValueError(f"combo references unknown key array: {array_name}")

            positions = []
            for trigger_keycode in combo_arrays[array_name]:
                normalized_trigger = normalize_keycode(trigger_keycode)
                matches = [
                    index
                    for index, layer_keycode in enumerate(base_layer)
                    if normalize_keycode(layer_keycode) == normalized_trigger
                ]
                if len(matches) != 1:
                    raise ValueError(
                        f"combo key {trigger_keycode} has {len(matches)} matches on the base layer"
                    )
                positions.append(matches[0])

            midpoint = (sum(positions), len(positions))
            align = "bottom" if midpoint in occupied_midpoints else None
            offset = STACKED_COMBO_OFFSET if align is not None else None
            occupied_midpoints.add(midpoint)
            combo_entries.append(
                (
                    positions,
                    keycode_label(result_keycode.strip(), combo_labels),
                    align,
                    offset,
                )
            )

    lines = [
        "# Generated by watch-ergo68-keymap.py from keymap.c; do not edit.",
        "combos:",
    ]
    for positions, label, align, offset in combo_entries:
        align_spec = f", a: {align}" if align is not None else ""
        offset_spec = f", o: {offset}" if offset is not None else ""
        lines.append(
            f"  - {{p: {json.dumps(positions)}, k: {json.dumps(label)}, "
            f"l: [{json.dumps(base_layer_name)}]{align_spec}{offset_spec}}}"
        )
    temporary = output.with_name(f"{output.name}.new")
    temporary.write_text("\n".join(lines) + "\n", encoding="utf-8")
    temporary.replace(output)


def add_svg_title(svg: Path, title: str) -> None:
    """Add an accessible title to a keymap-drawer SVG."""
    contents = svg.read_text(encoding="utf-8")
    root_tag_end = contents.find(">\n")
    if root_tag_end == -1:
        raise ValueError("generated SVG has no complete root tag")
    title_element = f"<title>{escape(title)}</title>\n"
    contents = contents[: root_tag_end + 2] + title_element + contents[root_tag_end + 2 :]
    svg.write_text(contents, encoding="utf-8")


class NoCacheHandler(SimpleHTTPRequestHandler):
    """Serve preview files without client-side caching or request logs."""

    def end_headers(self) -> None:
        """Add the cache policy before completing the response headers."""
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def log_message(self, format: str, *args: object) -> None:
        """Suppress the default HTTP request log."""
        pass


class LiveViewer:
    """Serve and refresh a generated SVG in the default browser."""

    def __init__(self, directory: Path) -> None:
        """Initialize a viewer rooted in a temporary directory."""
        self.directory = directory
        self.svg = directory / "keymap-live.svg"
        self.html = directory / "keymap-live.html"
        self.server: ThreadingHTTPServer | None = None
        self.server_thread: threading.Thread | None = None

    def start(self, output: Path) -> None:
        """Start the preview server and open its page."""
        copy_atomically(output, self.svg)
        self.html.write_text(LIVE_VIEW_HTML, encoding="utf-8")

        handler = lambda *args, **kwargs: NoCacheHandler(
            *args, directory=str(self.directory), **kwargs
        )
        try:
            self.server = ThreadingHTTPServer(
                ("127.0.0.1", DEFAULT_PREVIEW_PORT), handler
            )
        except OSError as error:
            if error.errno != errno.EADDRINUSE:
                raise
            log(
                f"preview port {DEFAULT_PREVIEW_PORT} is busy; "
                "using an available port instead"
            )
            self.server = ThreadingHTTPServer(("127.0.0.1", 0), handler)
        self.server.daemon_threads = True
        self.server_thread = threading.Thread(
            target=self.server.serve_forever, daemon=True
        )
        self.server_thread.start()

        port = self.server.server_address[1]
        url = f"http://127.0.0.1:{port}/{self.html.name}"
        log(f"live view {url}")
        if not webbrowser.open(url, new=2):
            log("could not open the live browser view; continuing to watch", error=True)

    def update(self, output: Path) -> None:
        """Replace the SVG served by the preview."""
        copy_atomically(output, self.svg)

    def close(self) -> None:
        """Stop the preview server and wait for its thread."""
        if self.server is not None:
            self.server.shutdown()
            self.server.server_close()
        if self.server_thread is not None:
            self.server_thread.join()


def generate_keyboard_info(qmk: str, info_json: Path) -> None:
    """Write the Yushakobo Ergo68 geometry reported by QMK to a JSON file."""
    with info_json.open("wb") as output:
        subprocess.run(
            [qmk, "info", "-kb", KEYBOARD, "-f", "json"],
            cwd=USERSPACE_ROOT,
            stdout=output,
            check=True,
        )


def render(
    source: Path,
    output: Path,
    work_dir: Path,
    commands: dict[str, str],
    viewer: LiveViewer | None,
) -> bool:
    """Render and publish the keymap, returning whether it succeeded."""
    info_json = work_dir / "info.json"
    parsed_yaml = work_dir / "keymap.yaml"
    rendered_svg = work_dir / "keymap.svg"
    try:
        documentation = load_documentation(DOCUMENTATION_SOURCE)
        layers = documentation["layers"]
        combo_labels = documentation["combo_labels"]

        assert isinstance(layers, dict)
        assert isinstance(combo_labels, dict)
        layer_identifiers = list(layers)
        layer_names = list(layers.values())

        converted = subprocess.run(
            [
                commands["qmk"],
                "c2json",
                "--no-cpp",
                "-kb",
                KEYBOARD,
                "-km",
                KEYMAP_NAME,
                str(source),
            ],
            cwd=USERSPACE_ROOT,
            stdout=subprocess.PIPE,
            check=True,
        )
        generate_combos_overlay(
            source,
            converted.stdout,
            DEFAULT_COMBOS_OUTPUT,
            layer_names[0],
            combo_labels,
        )
        drawer_keymap = resolve_layer_taps(converted.stdout, layer_identifiers)
        subprocess.run(
            [
                commands["keymap"],
                "-c",
                str(DRAWER_CONFIG_SOURCE),
                "parse",
                "-q",
                "-",
                "-l",
                *layer_names,
                "-o",
                str(parsed_yaml),
            ],
            cwd=USERSPACE_ROOT,
            input=drawer_keymap,
            check=True,
        )
        subprocess.run(
            [
                commands["keymap"],
                "-c",
                str(DRAWER_CONFIG_SOURCE),
                "draw",
                "-j",
                str(info_json),
                "-l",
                LAYOUT_NAME,
                str(parsed_yaml),
                str(DEFAULT_COMBOS_OUTPUT),
                "-o",
                str(rendered_svg),
            ],
            cwd=USERSPACE_ROOT,
            check=True,
        )
        add_svg_title(rendered_svg, f"{KEYBOARD_DISPLAY_NAME} keymap")
    except (OSError, json.JSONDecodeError, KeyError, ValueError) as error:
        log(f"could not prepare drawing metadata: {error}", error=True)
        log("render failed; keeping the previous SVG", error=True)
        return False
    except subprocess.CalledProcessError:
        log("render failed; keeping the previous SVG", error=True)
        return False

    copy_atomically(rendered_svg, output)
    if viewer is not None:
        viewer.update(output)
    log(f"updated {output} at {time.strftime('%H:%M:%S')}")
    return True


def main() -> int:
    """Render once or watch the source until interrupted."""
    args = parse_args()
    source = args.source.expanduser().resolve()
    output = args.output.expanduser().resolve()

    if not source.is_file():
        log(f"source file not found: {source}", error=True)
        return 1
    if not output.parent.is_dir():
        log(f"output directory not found: {output.parent}", error=True)
        return 1
    if not DRAWER_CONFIG_SOURCE.is_file():
        log(f"config file not found: {DRAWER_CONFIG_SOURCE}", error=True)
        return 1

    open_browser = not args.once and not args.no_open
    try:
        commands = find_commands()
    except RuntimeError as error:
        log(str(error), error=True)
        return 1

    signal.signal(
        signal.SIGTERM, lambda _signum, _frame: signal.raise_signal(signal.SIGINT)
    )

    try:
        with tempfile.TemporaryDirectory(prefix=f"{PROGRAM_NAME}.") as temporary:
            temp_dir = Path(temporary)
            info_json = temp_dir / "info.json"

            try:
                generate_keyboard_info(commands["qmk"], info_json)
            except subprocess.CalledProcessError:
                log("could not generate the local keyboard geometry", error=True)
                return 1

            viewer: LiveViewer | None = None
            succeeded = render(
                source,
                output,
                temp_dir,
                commands,
                viewer,
            )
            if args.once:
                return 0 if succeeded else 1

            if open_browser and succeeded:
                viewer = LiveViewer(temp_dir)
                viewer.start(output)

            watched_files = (source, DOCUMENTATION_SOURCE, DRAWER_CONFIG_SOURCE)
            previous_signatures = tuple(file_signature(path) for path in watched_files)
            log(
                f"watching {source}, {DOCUMENTATION_SOURCE}, and "
                f"{DRAWER_CONFIG_SOURCE} "
                "(press Ctrl-C to stop)"
            )

            try:
                while True:
                    time.sleep(POLL_INTERVAL_SECONDS)
                    current_signatures = tuple(
                        file_signature(path) for path in watched_files
                    )
                    if current_signatures == previous_signatures:
                        continue

                    previous_signatures = current_signatures

                    succeeded = render(
                        source,
                        output,
                        temp_dir,
                        commands,
                        viewer,
                    )
                    if open_browser and succeeded and viewer is None:
                        viewer = LiveViewer(temp_dir)
                        viewer.start(output)
            finally:
                if viewer is not None:
                    viewer.close()
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    sys.exit(main())
