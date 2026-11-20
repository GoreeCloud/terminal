# GoreeCloud Terminal

**Linux-native, local-first terminal emulator — development preview.**

This repository contains a Rust/Slint desktop application that runs ordinary local shells in pseudoterminals. The current branch implements a native window, independent shell tabs, basic VT escape parsing, input-mode-aware keyboard handling, scrollback navigation, explicit clipboard actions, and live PTY resizing. It is not a stable release or a feature-complete Ghostty substitute.

## Architecture

| Layer | Implementation | Status |
| --- | --- | --- |
| Native Linux interface | Slint 1.19.0 (Rust, Winit/FemtoVG) | Development source |
| Shell/PTY | portable-pty 0.9.0 | Independent local shell per tab, bounded output channel |
| Terminal-state parser | vt100 0.18.1 | ANSI/VT input, cursor, 2,000-row scrollback |
| Clipboard | arboard 3.7.0 | Explicit Copy/Paste; X11/XWayland default |
| Design foundation | [GoreeCloud UI](https://github.com/GoreeCloud/gc-ui) | Alpha dark semantic palette; native WCN acceptance pending |

Slint is one of GoreeCloud's approved native Linux frameworks. This is a real Linux GUI, not a browser wrapper. All terminal output stays on the local machine. No telemetry, cloud account, Mesh service, or background server is required.

## Build and run

Requirements: Linux graphical desktop, Rust >= 1.88, C/C++ toolchain and Slint/Winit/FemtoVG Linux libraries (fontconfig, X11/Wayland, EGL/OpenGL, pkg-config).

~~~bash
cargo build --locked
cargo test --locked --all-targets
cargo clippy --locked --all-targets -- -D warnings
cargo run --locked
~~~

Build a local desktop install (not a signed or audited release):

~~~bash
cargo build --locked --release
install -Dm755 target/release/goreecloud-terminal ~/.local/bin/goreecloud-terminal
install -Dm644 packaging/linux/goreecloud-terminal.desktop ~/.local/share/applications/goreecloud-terminal.desktop
~~~

## Keyboard and shell navigation

- Type directly into the terminal pane; Escape, Enter, Tab, arrows, navigation and F1–F12 are encoded as PTY input.
- **Ctrl+Shift+T:** open a shell tab (maximum eight).
- **Ctrl+Shift+W:** close the active tab and terminate its direct shell child.
- **Ctrl+Shift+C:** copy the visible screen as plain text to the system clipboard.
- **Ctrl+Shift+V:** paste the system clipboard. Multiline paste is blocked unless the terminal application has enabled bracketed-paste mode. Control characters and oversized pastes are blocked.
- **Ctrl+Shift+PageUp/PageDown:** navigate bounded scrollback; the same actions are available through toolbar buttons.
- Use native tab controls for switching sessions. The application also has a status line for shell exits, resize/paste errors and scrollback state.

**Important limitations:** The current renderer is monochrome text, with approximate character width/height; it is not an accurate per-cell truecolor renderer. Resizing uses estimated cell metrics, not measured font glyphs. Copy currently copies the visible screen, not a drag selection. Mouse tracking, IME, hyperlink activation, Kitty graphics, splits, persistent profiles and full screen-reader acceptance are not implemented.

On pure Wayland without XWayland, arboard's default clipboard backend may not work. There is no fallback command invoking untrusted external helpers. Use a genuine supported desktop session when validating the native GUI.

## Development, security and governance

- [Architecture](docs/ARCHITECTURE.md)
- [Security and privacy](docs/SECURITY.md)
- [Third-party licensing](docs/THIRD_PARTY.md)
- [Delivery roadmap and acceptance](docs/ROADMAP.md)
- [Main-branch rule configuration](config/main-protection.json) — snapshot of active GitHub ruleset; GitHub is authoritative.

**License:** GPL-3.0-only for original GoreeCloud code. Third-party libraries retain their own licenses. This distribution model uses Slint's GPLv3 path. Full dependency and native-device security acceptance remain outstanding; do not use the preview as your sole trusted terminal.
