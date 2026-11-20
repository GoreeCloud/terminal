# Architecture — GoreeCloud Terminal

## Ownership and data flow

```
Slint keyboard FocusScope -> validated key encoder -> PTY master writer -> user's shell
                                                         |
                                               Linux child + local TTY
                                                         |
Slint text rows <- bounded 30 fps snapshot <- vt100 parser <- PTY reader
```

- A single Rust process owns the native Slint window. It does not serve HTTP.
- `portable-pty` creates a real Unix PTY; the child shell runs as the current user, not root.
- PTY reader thread sends 8 KiB chunks into a bounded 128-item in-process channel. The main loop reads at most 64 chunks per 33ms tick and applies escape processing in-process.
- `vt100` 0.18.1 limits scrollback to 2,000 lines; the screen is initially 80×24.
- The Slint render model is a safe plain-text snapshot and does not treat terminal text as code, rich text, SVG, HTML or markup.
- The entire initial product is offline by construction. No backend, analytics, AI, account or remote APIs are required.

## Known preview limitations

This is a staged baseline, **not** feature parity with Ghostty or complete VT/terminal conformance:
- Monochrome text rendering and fallback block cursor can misalign Unicode-wide graphemes or glyph clusters.
- Screen and PTY remain 80×24 when the desktop window resizes. Fonts, fractional DPI and compositor metrics need measured native resize integration.
- No alternate-screen visual qualification, mouse reporting, selection, copy/paste, OSC 8 hyperlink click, kitty graphics protocol, sixel, IME, accessibility screen-reader grid or GPU-optimized render pipeline.
- Shell lifecycle, terminal output flood handling and PTY ioctl behavior need device/CI tests and deliberate cancellation semantics.
- Native Window Command Nodes (Dismiss/Dock/Expand) need supported host window APIs and accessible qualification; no fake controls in the preview.

## Architecture decisions

`Slint` satisfies the currently authorized GoreeCloud native Linux UI frameworks; the widget backend is not a webview. `portable-pty` avoids writing bespoke unsafe fork/exec/termios code. `vt100` is an independent parser for escape and cursor state. The immature third-party `slint-term` alpha was researched but intentionally not incorporated before verified Linux and dependency acceptance.

Future: separate rendering core (per-cell paint/semantic spans), PTY resizing based on measured font metrics, keyboard/mouse protocol fidelity, tabs/splits, session persistence only when explicitly opted in, accessibility and high-contrast fallback.
