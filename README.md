# NDI Access Manager on Linux

Terminal UI tool for managing the NDI configuration file on Linux.

<img width="1034" height="690" alt="Screenshot 2026-08-06 at 1 50 55 PM" src="https://github.com/user-attachments/assets/6b154f64-476e-4db1-8a3b-c2463e9813df" />


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
- Multicast recv subnets (CIDR list)
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

Alternatively, Nix/NixOS users can use the included `flake.nix` — see [Nix build](#nix-build) below.

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

## Nix / NixOS

The repo includes a `flake.nix` that builds `accessman` using FTXUI from nixpkgs (no git-clone-to-cache step). The vendored `json.hpp` is used as-is.

### Build and run locally

```bash
nix build
nix run
```

Or run without cloning:

```bash
nix run github:Suhaili-Labs/suhailiAccessManager
```

### NixOS system integration

You can run `accessman` without installing it:

```bash
nix run github:Suhaili-Labs/suhailiAccessManager
```

To install it system-wide, add it as an input in your system flake (`/etc/nixos/flake.nix`):

```nix
inputs.accessman.url = "github:Suhaili-Labs/suhailiAccessManager/v1.2.1";
```

Then add the package to your system configuration:

```nix
{ config, pkgs, inputs, ... }:

{
  environment.systemPackages = [
    inputs.accessman.packages.x86_64-linux.default
  ];
}
```

Rebuild:

```bash
sudo nixos-rebuild switch --flake /etc/nixos#your-hostname
```

#### Updating to a new release

Edit `/etc/nixos/flake.nix` to point at the new tag (e.g. `v1.2.2`), then refresh the lock and rebuild:

```bash
cd /etc/nixos
sudo nix flake lock --update-input accessman
sudo nixos-rebuild switch --flake /etc/nixos#your-hostname
```

The lock file pins the exact commit hash, so the update is explicit and reproducible.

#### Alternative: local source

If you want to build from a local checkout instead of a GitHub tag, use a `path:` input:

```nix
inputs.accessman.url = "path:./packages/accessman";
```

...with a `packages/accessman/` directory containing the `flake.nix` and source from this repo. Update by copying new source into that directory and rebuilding.

#### Multiple inputs

Your system flake can have multiple inputs alongside `accessman`:

```nix
inputs = {
  nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  accessman.url = "github:Suhaili-Labs/suhailiAccessManager/v1.2.1";
  another-tool.url = "github:owner/repo/v2.0";
};
```

Each input is independently pinned in `flake.lock` and can be added to `environment.systemPackages` as needed.

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

<img width="3360" height="840" alt="et_header_photo_logo" src="https://github.com/user-attachments/assets/612d2fac-a7f5-4c3e-8228-d7b9e7799979" />
