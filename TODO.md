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
- [ ] (larger-research) **TUI layout improvements (deferred)** — container
      direction audit, `Terminal::Size()`, status semantics; review §4.

## Done

- [x] (TEST) **NDI schema validation harness** — DONE @ 0d77bf6:
      `tests/schema_check.cpp` built run via `tests/run.sh`; field-type schema
      checks vs [NDI SDK docs](https://docs.ndi.video/all/developing-with-ndi/sdk/configuration-files),
      plus B5 validator regression assertions.
- [x] (BUG) **B1: wrong leaf types no longer crash** — DONE @ 66a4a22:
      `ensureBool/ensureString/ensureNumber` sanitize wrong-typed leaf values to
      defaults inside `generateMissingConfig`; subnets array enforced + filtered.
- [x] (BUG) **B2: Left/Right blocked in Input fields** — DONE @ 8892f41:
      `modesRowContainer` → `Container::Horizontal`, multicast split into three
      horizontal sub-containers, top-level CatchEvent remap removed, footer hint
      updated.
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

