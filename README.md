# GoreeCloud Terminal

**Linux-first, native, local-first terminal emulator.** This repository now contains an initial **development preview** of a real shell/PTY terminal application, not a completed replacement for Ghostty or a production-certified release.

## Architecture

- **Native GUI:** Slint 1.19, Rust and a real Linux desktop window; an approved GoreeCloud Linux UI toolkit.
- **Process integration:** `portable-pty` (the WezTerm ecosystem) spawns the user's existing executable `$SHELL` in a local PTY. No shell text evaluation is used to start it.
- **Terminal parser:** `vt100` 0.18 interprets incoming terminal control sequences in native Rust, separates process output from UI and keeps bounded scrollback.
- **Rendering:** Slint monochrome text grid plus cursor preview. This milestone does **not yet** handle per-cell ANSI foreground/background colors, wide grapheme rendering, mouse/selection, IME, resizing, tabs, sessions or GPU-optimized cell rendering.
- **Keyboard:** Unicode text, Enter, Tab, Backspace, Escape, arrows, common navigation keys and Ctrl/Alt escape sequences.
- **Security/privacy:** Shell and PTY are local; no telemetry, remote account, cloud dependency, background daemon or automatic command execution beyond starting the selected shell. Running arbitrary commands typed in the terminal is intentionally privileged to the *user's own login account*.

The architecture uses original GoreeCloud glue/code with attributed third-party crates. It does not copy Ghostty's code, UI or branding.

## Build and run (Linux)

Install an up-to-date Rust toolchain (at least 1.88), C/C++ build tools, and the packages required by Slint's Winit + FemtoVG Linux backend (graphics stack, Wayland/X11 as appropriate).

```bash
cargo build
cargo run
cargo test
cargo fmt --all -- --check
```

The application starts in a normal, resizable non-maximized desktop window. The current initial viewport is 80 columns × 24 rows; resizing the desktop window is **not yet** synchronized with PTY dimensions. While typing, click the terminal surface to restore keyboard focus. `$SHELL` must point to an absolute executable path; the app otherwise falls back to `/bin/sh`.

## Desktop integration

```bash
cargo build --release
install -Dm755 target/release/goreecloud-terminal ~/.local/bin/goreecloud-terminal
install -Dm644 packaging/linux/goreecloud-terminal.desktop ~/.local/share/applications/goreecloud-terminal.desktop
```

This is a developer-local installation recipe, not a signed distributable. Do not use the preview as your sole production terminal.

## Governance

- **GoreeCloud UI:** Uses the currently verified alpha semantic dark colors, materials and spacing from [gc-ui](https://github.com/GoreeCloud/gc-ui); platform-native window chrome remains OS owned. Native Window Command Nodes and full color/token synchronization remain outstanding and must not be presented as implemented.
- **GoreeShield:** no application telemetry, no shell output collection/upload.
- **Wardveil:** no privileged daemon, sandbox escape mechanism or unverified shell startup substitutions; terminal command execution remains the user's responsibility.
- **GoreeCloud Mesh/Identity:** no network or identity integration is needed for an offline shell. Future opt-in integrations need explicit data-flow, auth and threat-model review.

Read [architecture](docs/ARCHITECTURE.md), [security](docs/SECURITY.md), [third-party notices](docs/THIRD_PARTY.md), and [roadmap](docs/ROADMAP.md) before proposing extensions.

**License:** GPL-3.0-only for this repository's original implementation, selected for the open-source Slint GPLv3 route; see [LICENSE](LICENSE). Third-party code retains its own copyright/license. The dependency ecosystem and production distribution require independent ongoing review.
