# Master TODO

The single place to queue all open work across the repo. Link occasionally to full
design docs when needed; this list is the canonical work queue.

## Conventions

- Mark items `- [x]` and move them to **Done** with the commit hash, e.g.
  `DONE @ ca5f394`. On completion, also cross-link any relevant `agent/` log entry.
- New items go under **Open** with the most specific source-doc link (usually an
  `agent/` file). Group labels (FEATURE / BUG / DOCS / REFACTOR / TEST) optional.
- Prefer **specific priority tags** (quick-win / larger-research) over generic
  optional labels.

## Open

- [ ] (larger-research) **REST API prep (deferred)** — typed `NdiConfig` model,
      `core/` split, per-field validation report; review §3.2–3.3.
- [ ] (larger-research) **TUI status/semantics cleanup (deferred)** — status
      line semantics (neutral vs validated), `Terminal::Size()` instead of
      ioctl, min-size gate event handling; review §4.
- [ ] **Adaptive band width (came up in 90-char equal-width work)** — compute
      `EQUAL,90` from `Terminal::Size()` at runtime instead of fixed constant.

## Done

- [x] (TEST) **NDI schema validation harness** — DONE @ 0d77bf6:
      `tests/schema_check.cpp` built run via `tests/run.sh`; field-type schema
      checks vs [NDI SDK docs](https://docs.ndi.video/all/developing-with-ndi/sdk/configuration-files),
      plus B5 validator regression assertions.
- [x] (BUG) **B1: wrong leaf types no longer crash** — DONE @ 66a4a22:
      `ensureBool/ensureString/ensureNumber` sanitize wrong-typed leaf values to
      defaults inside `generateMissingConfig`; subnets array enforced + filtered.
- [x] (BUG) **B2: arrow navigation broken** — DONE series 8892f41 → 18edf88:
      `modesRowContainer` → `Container::Horizontal`; nested Vertical wrappers so
      Up/Down bubbles at each band level (4888ba4, bc80360); swap Toggles for
      Checkboxes and TTL for numeric Input so Left/Right stays focus navigation
      everywhere (18edf88).
- [x] (BUG) **B3: HOME fallback** — DONE @ dd5f296: `getpwuid(getuid())->pw_dir`
      fallback when `$HOME` is unset.
- [x] (REFACTOR) **B4: single-source defaults + version** — DONE @ f224cd8:
      `version.txt` read by `build/makefile` (`-DAPP_VERSION`) and `flake.nix`
      (`builtins.readFile`); multicast defaults in `accessman.hpp`
      (`kDefaultMulticast*`) shared by generation, fallback filling, and
      save-time coercion.
- [x] (BUG) **B5: validator hardening** — DONE @ feff00a: leading-zero IPv4
      octets rejected, all-zero netmask `0.0.0.0` rejected, discovery entries
      accept optional `:port` per NDI SDK docs; harness regression assertions.
- [x] (DOCS) **B6: dead code + docs cleanup** — DONE @ a7b2e69: removed unused
      `configExists()`, added explicit `<cstdint>` include, README Exit wording
      replaced with Save/Discard semantics.
- [x] (DOCS) **CHANGELOG entry for B1–B6** — DONE @ 985d1a6: dated entry and
      Done-hashed items moved in `TODO.md`.
- [x] (FEATURE) **Protocol row layout evolved** — DONE series 9d3bbb6, 3270d15,
      26ea5d0, b26366a, c450d55, bb24209, 2e9e661: Mcast box merged into the
      protocols row; two-line titles on TTL & Recv Subnets; protocols widened
      16→21; multicast band reduced to Settings+TTL; subnets moved in.
- [x] (STYLE) **Equal-width band centering** — DONE series 1c4696f, 9421985,
      4fd8ccd: all three band rows bottom at `size(WIDTH, EQUAL, 90)` wrapped in
      `filler()` children so terminal centering is preserved.
- [x] (STYLE) **Checkbox spacing fix** — DONE @ ee07da3 then corrected @ b2ed930:
      text-label padding inserted explicitly as `text("  ")` element between
      separator and render (label spaces alone didn't move the check glyph).
- [x] (CHORE) **Version bump to 1.3.0** — DONE @ 172b673; forced-rebuild required
      because `version.txt` isn't tracked by make.
- [x] (DOCS) **README split + slim** — DONE @ 3b2e611: NixOS integration moved
      to `docs/nixos.md`, tags refreshed to `v1.3.0`, README reduced from 209
      lines to 109 and structure section refreshed (`version.txt`, `tests/`,
      `TODO.md`, `agent/`).
- [x] **Release v1.3.0 tagged** — tag `v1.3.0` pushed to origin; release notes
      prepared for manual attachment (gh auth required).

