# Code Review — suhailiAccessManager

Review date: 2026-09-01
Scope: `tui.cpp`, `accessman.hpp`, `tui_support.hpp`, `build/makefile`, `flake.nix`, docs.
Forward-looking constraint: a small oat++ REST API server will be added later so remote clients can edit the config file. Recommendations are prioritized accordingly.

Two small standalone test programs were compiled against the repo headers to verify suspected bugs; results are quoted in the relevant sections. Tests lived in `/tmp/opencode/review_tests/` and are not part of the repo.

---

## 1. Verified bugs

### B1. Crash on valid JSON with wrong leaf types (high)

`loadConfigWithDefaults()` (tui_support.hpp:52) protects against *malformed* JSON, and `generateMissingConfig()` only checks key presence with `contains()`. Leaf **types** are never validated. The direct reads in `tui.cpp` then throw `nlohmann::json::type_error`, which is uncaught → `std::terminate`.

Affected reads:

- `tui.cpp:128-136` — toggle ints (`ndi.tcp/rudp/unicast/multicast.send/recv.enable`, `ttl`)
- `tui.cpp:152-159` — strings (groups, networks, machinename, netmask, netprefix)
- `tui.cpp:238-259` — same reads again in `loadBackupIntoForm` (backup path also crashes)

Reproduced with a compiled test against `accessman.hpp` + `json.hpp`:

```
[1] generateMissingConfig: OK (no exception)
[2] int read of string 'enable' THREW (uncaught in tui.cpp => crash):
    type must be number, but is string
[3] int read of string ttl THREW (uncaught in tui.cpp => crash):
    type must be number, but is string
[5] number->string read THREW (uncaught in tui.cpp => crash):
    type must be string, but is number
```

A hand-edited config like `"enable": "yes"` or `"ttl": "high"` kills the app at startup.

**Fix:** sanitize types in `generateMissingConfig()` (if a leaf is present but the wrong type, replace with default), or read with `ndiConfig.value<T>(path, default)` / `get<T>()` guarded. A typed config model (see 3.2) makes this systematic.

### B2. Arrow-key remap breaks cursor movement inside Input fields (high UX bug)

`tui.cpp:653-675` wraps the whole app in `CatchEvent` and remaps `ArrowLeft/Right` → `ArrowUp/Down` at the top level. `ftxui::Input` normally uses Left/Right to move the text cursor; those events are consumed before any focused Input sees them. Result: users cannot move the caret with arrow keys in any text field — only mouse-free focus navigation works. The footer hint (`tui.cpp:640`) even documents the hijack: "Up/Down/Left/Right move focus".

**Fix:** drop the remap and let containers handle keys natively (FTXUI `Container::Vertical` already uses Up/Down; inputs keep Left/Right), or remap only when the focused component is not an Input. This becomes more important, not less, once remote editing exists (fields must be reliably editable).

### B3. HOME unset → config written to current working directory (medium)

`getHomeDir()` (accessman.hpp:11-14) returns `""` when `$HOME` is unset, and `getConfigPaths()` builds `.ndi/ndi-config.v1.json` relative to CWD. Verified:

```
[1] HOME unset -> configPath = ".ndi/ndi-config.v1.json"
```

For a future daemon/REST service (systemd units, containers), $HOME is frequently absent.

**Fix:** fall back to `getpwuid(getuid())->pw_dir`, or refuse to start with a clear error.

### B4. Inconsistent default values across the codebase (low correctness, high maintenance)

Multicast defaults are defined in three places, and two of them disagree:

- `tui.cpp:47-48`: `"255.255.0.0"` / `"239.255.0.0"`
- `accessman.hpp:16-26` (`multicastGenConfig`): `"255.255.0.0"` / `"239.255.0.0"`, ttl 1
- `accessman.hpp:195-197` (`generateMissingConfig`, `multicast` exists but `send` missing): netmask `""`, netprefix `""`, ttl 1

Blank values happen to be re-defaulted at save time in `tui.cpp:694-697`, but the in-memory model is internally inconsistent.

Also `appVersion` is hard-coded in `tui.cpp:46` and duplicated in `flake.nix:17`. Single-source both (e.g. a generated `version.hpp` or compile definition).

### B5. Minor validator edges (low)

Verified behavior:

- `isValidIPv4("010.0.0.1") = true` — leading zeros accepted; many parsers (inet_aton) interpret `010` as octal. Reject leading zeros unless you deliberately accept them.
- `isValidNetmask("0.0.0.0") = true` — `/0` accepted. Decide if intended.
- Blank CSV lists validate as true (documented design; fine).

### B6. Dead code & documentation drift (low)

- `accessman.hpp:28-30` `configExists()` is never called (tui.cpp uses `std::filesystem::exists` directly at `tui.cpp:415`).
- `README.md:184` still says "exit using the `Exit` button" — no such button exists since the Save/Discard split.
- `tui_support.hpp` uses `uint32_t` (line 236) without `#include <cstdint>` — currently pulled in transitively; add it explicitly (include-what-you-use).

---

## 2. Boilerplate (mechanical duplication)

### 2.1 Backend — `accessman.hpp`

- `tcpSet`, `rudpSet`, `unicastSet` (lines 36-91) are byte-for-byte identical except for the key name. Collapse into one helper:
  `transportSet(json& cfg, const char* mode, bool send, bool recv)`.
- `generateMissingConfig` (lines 162-263) repeats the same `if (!contains) assign default` shape ~30 times, plus per-key sub-object checks in the setters. A typed model (below) or a small `ensureKey(json&, path..., default)` helper removes most of it.

### 2.2 Frontend — `tui.cpp`

- Lines 161-197: 17 `const initial*` snapshot variables + a 17-term `hasUnsavedChanges` lambda.
- Lines 423-439: another 17 `changed*` booleans computed in the render lambda.
- Lines 227-266 (`loadBackupIntoForm`) duplicate the load logic of lines 128-159.
- Lines 293-327: two near-identical Modal confirm dialogs (same colors, same shape).

A `FormState` struct holding the fields with `snapshot()` / `isDirty() const` / `loadFromJson()` would delete ~100 lines and, more importantly, makes the dirty/validation logic testable outside FTXUI. A `makeConfirmDialog(title, subtitle, onConfirm)` helper halves the modal code.

### 2.3 Layering note

`using namespace ftxui;` and `using namespace tui_support;` are both dumped into `tui.cpp` (lines 22-23). Acceptable for one file, but qualify names once a second translation unit (REST server) exists.

---

## 3. Backend improvements (timed for REST API readiness)

The key structural problem today: **config semantics live in headers and get re-derived everywhere.** Raw `json` is threaded through setters that all mutate by string-typed paths. A REST server will need the same "load → normalize → validate → mutate → save" pipeline; it should not live in `tui_support.hpp`.

### 3.1 Introduce a typed `NdiConfig` model (the big one)

Define a plain struct:

```cpp
struct TransportFlags { bool send; bool recv; };
struct NdiConfig {
  std::string machinename;
  std::string discoveryServers;   // CSV string (or vector<string>)
  std::string ips;
  std::string sendGroups, recvGroups;
  TransportFlags tcp, rudp, unicast;
  struct Multicast {
    bool sendEnable; bool recvEnable;
    std::string netmask, netprefix;
    int ttl;
    std::vector<std::string> recvSubnets;
  } multicast;
};
NdiConfig from_json(const json&);
json to_json(const NdiConfig&);
```

Benefits:

- Deletes most of `accessman.hpp` (both setters and `generateMissingConfig`) — defaults live in the struct's member initializers.
- Type safety enforced once, in `from_json` with per-field fallbacks (fixes B1).
- TUI binds fields to the struct directly; dirty-checking becomes `current != initial` on one object.
- oat++ can either bind its DTOs to this struct or serialize it through nlohmann — either way there is **one** canonical mapping between wire and model.
- Version constant B4 likewise lives in one place.

### 3.2 Split `core/` from `tui/`

Current layout is header-only `inline` everything. Proposed:

```
src/
  core/        ndi_config.{hpp,cpp}   (model, defaults, validation)
  core/        config_store.{hpp,cpp} (paths, atomic save, backup, locking)
  tui/         tui.cpp, form helpers
  server/      oatpp wiring (later)
```

Validation (`isValidIPv4`, `isValidCidr`, CSV helpers, netmask checks) is **not TUI support** — move it to `core/` so the REST layer shares it. `tui_support.hpp` keeps only FTXUI-facing glue. Headers-only was fine at this size; with a second binary it slows every compile and makes linkage fragile.

### 3.3 Validation as a per-field report, not first-error bool

`validateBeforeSave(string&)` (tui.cpp:199) returns a single message. A REST `PUT /config` wants `{ "errors": [ {"field": "multicast.recv.subnets", "message": "..."} ] }` with HTTP 400. Even the TUI benefits — it can highlight all invalid rows at once instead of one per save attempt. Signature suggestion:

```cpp
std::vector<FieldError> validate(const NdiConfig&);
```

### 3.4 Concurrency and durability for a future server

The REST server introduces a second writer to the file. Today's read-modify-write cycle has no exclusion:

- Add an `flock` (or `std::filesystem`-scoped lock file) around the whole load→save cycle; `saveConfigAtomicallyWithBackup` (tui_support.hpp:103) already does atomic rename, which is the right primitive to keep.
- Add `fsync` on the temp file before rename for crash durability (currently data may still be in page cache on power loss).
- Decide the architecture: either (a) both processes lock the file, or (b) the server owns all writes and the TUI talks to it (cleanest long-run answer).
- Set config file permissions explicitly (0600) — matters once the API tokens/auth material live anywhere near it.

### 3.5 Small cleanups

- Pass strings to setters by `const std::string&` (all of `accessman.hpp`).
- `tui_support.hpp` `getConfigPaths()` — see B3 fix.
- Remove `configExists` (B6) or use it at tui.cpp:415.

---

## 4. TUI layout improvements

Beyond the B2 arrow-key bug:

### 4.1 Container direction vs render direction

`modesRowContainer` is `Container::Vertical` (tui.cpp:363-367) but rendered in an `hbox`. FTXUI focus traversal follows the container direction, which is why the Left/Right remap (B2) had to be hand-rolled. Using `Container::Horizontal` for horizontal rows and a nested `Container::Vertical` wrapper restores natural navigation without the CatchEvent hack.

### 4.2 Terminal size: use FTXUI instead of raw ioctl

`getTerminalSize()` (tui.cpp:32-38) does a raw `ioctl(TIOCGWINSZ)` every render frame. FTXUI exposes `Terminal::Size()` — same information without duplicating platform code. Also note: when the "too small" gate is active, inputs in `mainContainer` still receive events (the gate only swaps the rendered element). With the CatchEvent remap removed (B2), gate interaction handling explicitly.

### 4.3 Width semantics

Row widths are set mostly by hard-coded padding strings (e.g. `tui.cpp:482` `"    Machine Name    "`) except one place that uses `size(WIDTH, EQUAL, 38)` (tui.cpp:604). Prefer `size()` consistently — padding strings break alignment the moment a label changes, and make future localization/theming needlessly painful.

### 4.4 Status semantics

- "No changes to save" and "Not loaded" render green (tui.cpp:463-465, 459-461) — neutral states painted as success. Use `dim` for neutral, green for affirmative, red for errors.
- `Save & Exit` with no changes reports "No changes to save" **and stays in the app** (tui.cpp:92-110). Either make it exit anyway, or relabel the intent in the status line.

### 4.5 Modal duplication

Fold the two confirm dialogs into one factory (see 2.2) — they differ only in title, body text, and confirm handler.

---

## 5. REST API (oat++) integration sketch

Concrete, minimal path when the time comes — kept here so the refactor above points at it:

```cpp
// Pseudo-layout after 3.1/3.2 exist:
GET  /api/v1/config          -> 200 to_json(current)
PUT  /api/v1/config          -> validate -> save -> 200, else 400 {errors}
GET  /api/v1/config/backup   -> backup document
POST /api/v1/config/restore  -> load backup, validate, save
GET  /api/v1/status          -> version, dirty flag, backup presence
```

Practical notes:

- **Bind/Auth:** bind to 127.0.0.1 by default; token or mTLS if binding to LAN must be deliberate. oatpp's `ApiController` + DTO error mapping covers the validation-report shape from 3.3.
- **Model format on the wire** matches `to_json`, so REST and file formats can never drift.
- `main()` becomes a subcommand dispatcher: `accessman tui` (current behavior) / `accessman serve --port N`. Keep TUI-only deps (FTXUI) out of the server target.
- Vendor/versioning: oatpp via nixpkgs (`flake.nix` already handles deps there) or the existing cache-clone makefile pattern. Bump the single version source (B4 fix) in one place.

---

## 6. Suggested priority order

1. **B1 type-crash fix** — it's a live crash on hand-edited configs.
2. **B2 arrow-key remap** — daily UX wart, trivial to fix.
3. **3.1 typed model + validation report (3.3)** — unblocks everything REST-related and deletes the worst boilerplate.
4. **B3 HOME fallback**, **B4 single defaults/version**, **B6 dead code/docs**.
5. 4.x TUI refactors (layout/navigation) once the model is in place.

None of these pre-requisites require the REST server to exist; each one is independently shippable.
