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

- [ ] (TEST) **NDI schema validation harness** — `tests/schema_check.cpp`;
      validates config against NDI SDK field-type rules (docs link in
      [`agent/review.md`](agent/review.md)).
- [ ] (BUG) **B1: wrong leaf types no longer crash** — sanitize types in
      `generateMissingConfig`; [agent/review.md](agent/review.md) §B1.
- [ ] (BUG) **B2: Left/Right blocked in Input fields** — container restructure,
      drop top-level CatchEvent remap; review §B2.
- [ ] (BUG) **B3: HOME fallback** — `getpwuid()` fallback; review §B3.
- [ ] (REFACTOR) **B4: single-source defaults + version** — `version.txt` +
      `-DAPP_VERSION`; review §B4.
- [ ] (BUG) **B5: validator hardening** — reject leading-zero IPv4 octets and
      `0.0.0.0` netmask; discovery entries accept optional `:port` per NDI docs;
      review §B5.
- [ ] (DOCS) **B6: dead code + docs cleanup** — remove `configExists`, add
      `<cstdint>`, fix README Exit wording; review §B6.
- [ ] (DOCS) **CHANGELOG for B1–B6** — dated entry.
- [ ] (larger-research) **REST API prep (deferred)** — typed `NdiConfig` model,
      `core/` split, per-field validation report; review §3.2–3.3.
- [ ] (larger-research) **TUI layout improvements (deferred)** — container
      direction audit, `Terminal::Size()`, status semantics; review §4.

## Done

(empty)
