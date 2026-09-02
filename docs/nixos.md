# Nix / NixOS Integration

The repo includes a `flake.nix` that builds `accessman` using FTXUI from
nixpkgs (no git-clone-to-cache step). The vendored `json.hpp` is used as-is.

## Build and run locally

```bash
nix build
nix run
```

Or run without cloning locally:

```bash
nix run github:Suhaili-Labs/suhailiAccessManager
```

## System-wide installation

Add the repo as an input in your system flake (`/etc/nixos/flake.nix`):

```nix
inputs.accessman.url = "github:Suhaili-Labs/suhailiAccessManager/v1.3.0";
```

Then rebuild:

```bash
sudo nixos-rebuild switch --flake /etc/nixos#your-hostname
```

## Updating to a new release

Edit `/etc/nixos/flake.nix` to point at the new tag (e.g. `v1.4.0`), refresh
the lock, and rebuild:

```bash
cd /etc/nixos
sudo nix flake lock --update-input accessman
sudo nixos-rebuild switch --flake /etc/nixos#your-hostname
```

The lock file pins the exact commit hash, so the update is explicit and
reproducible.

## Local source (instead of a tag)

```nix
inputs.accessman.url = "path:./packages/accessman";
```

...with a `packages/accessman/` directory containing the `flake.nix` and source
from this repo. Update by copying new source into that directory and
rebuilding.

## Multiple inputs

Your system flake can include additional inputs alongside `accessman`:

```nix
inputs = {
  nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  accessman.url = "github:Suhaili-Labs/suhailiAccessManager/v1.3.0";
  another-tool.url = "github:owner/repo/v2.0";
};
```

Each input is independently pinned in `flake.lock` and can be added to
`environment.systemPackages` as needed.
