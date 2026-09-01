# Changelog

All notable changes to this project are documented in this file.

## 2026-09-01

### Added
- New NDI schema validation harness in `tests/` (`schema_check.cpp`, run via `tests/run.sh`) asserting field/type rules from the NDI SDK configuration docs (link in [`agent/review.md`](agent/review.md)). Run before every commit touching config semantics.
- Discovery entries now accept an optional `:port` per the NDI SDK docs (e.g. `192.168.1.1:5960`); new validator `tui_support::isValidDiscoveryEntry` + `validateDiscoveryCsv`.
- `version.txt` at repo root is the single source for the app version; `build/makefile` injects it via `-DAPP_VERSION`, `flake.nix` reads it via `builtins.readFile`.

### Fixed
- Hand-edited configs with wrong-typed leaf values no longer crash the app: `generateMissingConfig` now sanitizes leaf types (`ensureBool/ensureString/ensureNumber`) in addition to presence checks; non-string `subnets` entries are dropped.
- `$HOME` unset now falls back to the account's home directory via `getpwuid(getuid())->pw_dir` instead of resolving config paths relative to the current working directory.
- Inline Left/Right now reach text `Input` fields again: `modesRowContainer` is `Container::Horizontal`, the multicast row is split into three horizontal sub-containers, and the top-level `CatchEvent` arrow remap was removed. Navigation hint updated accordingly.
- IPv4 validators reject leading-zero octets (octal ambiguity) and all-zero netmask `0.0.0.0`.

### Changed
- Multicast defaults (`255.255.0.0` / `239.255.0.0` / ttl 1) are single-sourced in `accessman.hpp` (`kDefaultMulticastNetmask/Netprefix/Ttl`) and shared by generation, fallback filling, and save-time coercion.
- README now describes the `Save & Exit` / `Discard & Exit` semantics (the `Exit` button no longer exists), including the rolling `.bak` behaviour.
- Removed the unused `configExists()` helper; added an explicit `<cstdint>` include to `tui_support.hpp`.

## 2026-08-06

### Added
- Multicast recv subnets support in the TUI as a CIDR list stored at `ndi.multicast.recv.subnets`.
- A dedicated selectable panel for multicast recv subnets in the bottom status area.

### Changed
- Multicast recv config generation now carries a `subnets` array so partial configs load and save cleanly.
- Multicast recv subnets are validated, round-tripped through backup restore, and saved as JSON arrays.
- The multicast recv subnets panel now uses a fixed width so the input does not jump around while typing.

## 2026-07-23

### Added
- Defensive config loading in `tui.cpp` to handle malformed JSON without crashing.
- Recovery path that resets to defaults if config normalization throws.
- Selection normalization for all boolean toggles and bounds clamping for multicast TTL index.
- A new `Discard & Exit` button in the TUI that exits without writing changes to config.
- Discard confirmation modal when unsaved changes exist.
- A live `Unsaved changes` / `All changes saved` status indicator in the TUI.
- A minimum terminal size gate that shows a resize prompt instead of rendering the full TUI when the screen is too small.
- A `Restore Backup` button that loads `ndi-config.v1.json.bak` into the UI form for review before saving.
- Save-time validation for discovery IPs, network IP list, multicast netmask, and multicast netprefix.

### Changed
- Filesystem directory checks and creation now use non-throwing `std::error_code` flows with user-facing error messages.
- First-run behavior message for missing config file is now informational instead of an error.
- Save behavior is now explicit: config is only written when `Save & Exit` is selected.
- Config save now uses an atomic temp-file write + rename to prevent partial/corrupted writes.
- Save now also attempts a rolling `.bak` backup of the previous config before replacing the main file (best effort).
- The UI now uses a wider `Status` panel for change-state and backup status, with `Restore Backup` available in the bottom action row.
- `Save & Exit` now blocks and shows an inline status error when validation fails.
- Blank multicast netmask/netprefix are now valid and save as defaults (`255.255.0.0` and `239.255.0.0`); discovery/IP lists can remain blank.
- Network and multicast IP input rows now highlight yellow when changed, and red when invalid after a save attempt (label and input box).
- Existing hbox rows now drive color state directly across editable settings: yellow when changed, red on validation errors after save is attempted.

### Notes
- `tui.cpp` compiles to object successfully after these changes.
- Full link step still depends on FTXUI static libraries configured in the build environment.
