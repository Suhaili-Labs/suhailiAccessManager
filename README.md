<img width="800" height="800" alt="logo" src="https://github.com/user-attachments/assets/b2f48237-c952-41ff-a864-66b63613e504" />

# Suhaili Labs Access Manager for NDI

Terminal UI tool for managing the NDI configuration file on Linux.

<img width="867" height="634" alt="Screenshot 2026-07-23 at 5 28 07 PM" src="https://github.com/user-attachments/assets/403810b7-acde-457c-8530-daa66b9374e0" />

This project edits the NDI SDK config file:

- `~/.ndi/ndi-config.v1.json`

The config controls discovery, groups, transport modes, and multicast behavior used by NDI applications.

NDI configuration reference:
https://docs.ndi.video/all/developing-with-ndi/sdk/configuration-files

## What It Does

The app provides a text-based interface (TUI) to modify:

- Machine name
- Discovery servers (`ndi.networks.discovery`)
- Discovery IP list (`ndi.networks.ips`)
- Send and receive groups (`ndi.groups.send`, `ndi.groups.recv`)
- TCP send/recv enable flags
- RUDP send/recv enable flags
- Unicast send/recv enable flags
- Multicast recv enable flag
- Multicast send settings:
	- enable
	- netmask
	- netprefix
	- TTL

On startup, the app also creates any missing config sections/keys required by the UI so partial JSON files can still be safely edited.

## Behavior on First Run

- If `~/.ndi` does not exist, it is created.
- If `~/.ndi/ndi-config.v1.json` does not exist or cannot be opened, the app starts with defaults in memory and writes a new config file when you exit the UI.

## Requirements

These requirements apply when **building from source**. If you don't want to build from source, pre-built dependency-free binaries are available in the [Releases](../../releases) section.

Tested on Ubuntu 24.04 and Arch Linux.

- Linux
- C++ compiler (`g++`)
- `cmake`
- `git`
- `make`
- Network access on first build (the Makefile caches and builds static FTXUI outside the repo)
- C++17 or newer recommended

`json.hpp` (nlohmann/json single-header) is already included in this repository.

## Build

The project includes a Makefile at `build/makefile`.

From the project root:

```bash
cd build
make -f makefile
```

This generates:

- `build/accessman`

Note: the first build downloads and compiles static FTXUI into:

- `~/.cache/suhailiAccessManager/ftxui`

This keeps FTXUI out of the repository while still producing a dependency-free static binary.

Clean build artifacts:

```bash
cd build
make -f makefile clean
```

## Installation

After building, you can run the app either locally from the build folder or install it system-wide.

Option 1: Run locally (no install)

```bash
./accessman
```

Option 2: Install system-wide

```bash
sudo cp accessman /usr/local/bin/accessman
```

Then run it from anywhere:

```bash
accessman
```

Use keyboard navigation in the TUI to adjust values, then exit using the `Exit` button. The config is written back to `~/.ndi/ndi-config.v1.json` when the program exits.

## Project Structure

- `tui.cpp`: TUI entry point and UI layout (FTXUI)
- `accessman.hpp`: header-only JSON update helpers and missing-key generation
- `tui_support.hpp`: header-only TUI validation and config I/O helpers
- `json.hpp`: nlohmann/json header
- `build/makefile`: build instructions and linking

## Troubleshooting

- Linker errors for FTXUI:
	- Ensure `git`, `cmake`, and network access are available for first-time cache population.
	- If needed, remove `~/.cache/suhailiAccessManager/ftxui` and rebuild.
- Config not updating:
	- Confirm write permissions for `~/.ndi/ndi-config.v1.json`.
- Program starts but config seems incomplete:
	- This is expected for sparse configs; missing fields are auto-generated before rendering the UI.

## Notes

- This tool currently targets Linux.
- NDI is a registered trademark of Vizrt NDI AB.
