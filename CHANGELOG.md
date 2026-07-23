# Changelog

All notable changes to this project are documented in this file.

## 2026-07-23

### Added
- Defensive config loading in `tui.cpp` to handle malformed JSON without crashing.
- Recovery path that resets to defaults if config normalization throws.
- Selection normalization for all boolean toggles and bounds clamping for multicast TTL index.
- A new `Discard & Exit` button in the TUI that exits without writing changes to config.
- A live `Unsaved changes` / `All changes saved` status indicator in the TUI.
- A minimum terminal size gate that shows a resize prompt instead of rendering the full TUI when the screen is too small.
- A `Restore Backup` button that restores `ndi-config.v1.json.bak` to the active config file.

### Changed
- Filesystem directory checks and creation now use non-throwing `std::error_code` flows with user-facing error messages.
- First-run behavior message for missing config file is now informational instead of an error.
- Save behavior is now explicit: config is only written when `Save & Exit` is selected.
- Config save now uses an atomic temp-file write + rename to prevent partial/corrupted writes.
- Save now also attempts a rolling `.bak` backup of the previous config before replacing the main file (best effort).

### Notes
- `tui.cpp` compiles to object successfully after these changes.
- Full link step still depends on FTXUI static libraries configured in the build environment.
