# NDI Access Manager on Linux

Terminal UI tool for managing the NDI configuration file on Linux.

<img width="1034" height="690" alt="Screenshot 2026-08-06 at 1 50 55 PM" src="https://github.com/user-attachments/assets/6b154f64-476e-4db1-8a3b-c2463e9813df" />

This project edits the NDI SDK config file: `~/.ndi/ndi-config.v1.json`. The
config controls discovery, groups, transport modes, and multicast behavior used
by NDI applications. Reference: [NDI SDK config docs](
https://docs.ndi.video/all/developing-with-ndi/sdk/configuration-files)

## What It Does

The app provides a text-based interface (TUI) to modify:

- Machine name
- Discovery servers (`ndi.networks.discovery`; optional `:port` per entry)
- Discovery IP list (`ndi.networks.ips`)
- Send and receive groups (`ndi.groups.send`, `ndi.groups.recv`)
- TCP / RUDP / Unicast send/recv enable flags
- Multicast recv enable flag and recv subnets (CIDR list)
- Multicast send settings: enable, netmask, netprefix, TTL numeric 0–255

Missing config sections/keys are created on startup, so sparse JSON files stay
safe to edit.

## Behavior on First Run

- If `~/.ndi` does not exist, it is created.
- If `~/.ndi/ndi-config.v1.json` does not exist or cannot be opened, the app
  starts with defaults in memory and writes a new config file when you choose
  `Save & Exit`.

## Requirements

Building from source is optional — pre-built dependency-free binaries are in
[Releases](../../releases).

Tested on Ubuntu 24.04 and Arch Linux. Requirements: Linux, `g++` (C++17+),
`cmake`, `git`, `make`, and network access on first build (static FTXUI is
cached outside the repo). Nix/NixOS users can use the included `flake.nix`
instead — see [docs/nixos.md](docs/nixos.md).

## Build

```bash
cd build
make -f makefile
```

Produces `build/accessman`. First build downloads and compiles static FTXUI
into `~/.cache/suhailiAccessManager/ftxui`, keeping FTXUI out of the
repository while still producing a dependency-free static binary.

Clean:

```bash
cd build
make -f makefile clean
```

## Install and Run

Run locally from the build folder:

```bash
./accessman
```

Or install system-wide:

```bash
sudo cp build/accessman /usr/local/bin/accessman
accessman
```

Use keyboard navigation in the TUI to adjust values, then choose either
`Save & Exit` (validates before writing and atomically updates the config) or
`Discard & Exit` (leaves the config untouched; a confirmation appears when
there are unsaved changes). On save, the previous config is rolled to
`~/.ndi/ndi-config.v1.json.bak`.

## Project Structure

- `tui.cpp`: TUI entry point and UI layout (FTXUI)
- `accessman.hpp`: header-only JSON update helpers and missing-key generation
- `tui_support.hpp`: header-only validation and config I/O helpers
- `json.hpp`: vendored nlohmann/json header
- `build/makefile`: build instructions and linking
- `version.txt`: single source of the app version (read by makefile and
  `flake.nix`)
- `tests/`: NDI schema validation harness (`tests/run.sh`)
- `TODO.md`: work queue with Done hashes
- `agent/review.md`: full code review notes

## Troubleshooting

- Linker errors for FTXUI: ensure `git`, `cmake`, and network access are
  available for first-time cache population; if needed, delete
  `~/.cache/suhailiAccessManager/ftxui` and rebuild.
- Config not updating: confirm write permissions for `~/.ndi/`.
- Sparse config: missing fields are auto-generated before rendering the UI.

## Notes

- Linux only.
- NDI is a registered trademark of Vizrt NDI AB.

<img width="3360" height="840" alt="et_header_photo_logo" src="https://github.com/user-attachments/assets/612d2fac-a7f5-4c3e-8228-d7b9e7799979" />
