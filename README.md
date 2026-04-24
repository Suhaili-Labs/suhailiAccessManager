# Suhaili Labs Access Manager for NDI

Terminal UI tool for managing the NDI configuration file on Linux.

<img width="1249" height="826" alt="suhailiAccessManager" src="https://github.com/user-attachments/assets/b9f3e2ff-08b6-4aee-b1d5-cd53d3596c22" />


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
- FTXUI static libraries available at:
	- `/usr/lib/libftxui-component.a`
	- `/usr/lib/libftxui-dom.a`
	- `/usr/lib/libftxui-screen.a`
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

Clean build artifacts:

```bash
cd build
make -f makefile clean
```

## Run

From project root:

```bash
./build/accessman
```

Use keyboard navigation in the TUI to adjust values, then exit using the `Exit` button. The config is written back to `~/.ndi/ndi-config.v1.json` when the program exits.

To make `accessman` available system-wide, copy the binary to `/usr/local/bin`:

```bash
sudo cp build/accessman /usr/local/bin/accessman
```

It can then be run from anywhere with:

```bash
accessman
```

## Project Structure

- `tui.cpp`: TUI entry point and UI layout (FTXUI)
- `accessman.cpp`: JSON update helpers and missing-key generation
- `accessman.h`: helper function declarations
- `json.hpp`: nlohmann/json header
- `build/makefile`: build instructions and linking

## Troubleshooting

- Linker errors for FTXUI:
	- Ensure static FTXUI libraries exist in `/usr/lib` with the exact names used in `build/makefile`.
- Config not updating:
	- Confirm write permissions for `~/.ndi/ndi-config.v1.json`.
- Program starts but config seems incomplete:
	- This is expected for sparse configs; missing fields are auto-generated before rendering the UI.

## Notes

- This tool currently targets Linux.
- NDI is a registered trademark of Vizrt NDI AB.
