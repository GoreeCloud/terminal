# Linux terminal architecture

## Local process model

~~~text
Slint focus/click/shortcuts
         |
     Rust State (up to eight tabs)
         |      \---- explicit arboard clipboard read/write
    each Tab { vt100 Screen, portable-pty Session }
         |                |
      VT parser      PTY master <----> current user's shell
         ^
  bounded reader thread (128 × 8192 B maximum queued)
~~~

- Each tab independently owns a shell child, PTY master/writer/reader and terminal parser; closing a tab drops and kills its direct child.
- Window resize is polled on the 33ms UI tick. Approximate logical cell dimensions are calculated from the window scale factor and layout dimensions, and applied to both PTY and VT parser after the PTY resize succeeds.
- All tabs' queues are drained with a per-tab budget to prevent background PTYs filling memory and starving the active tab.
- VT parsing handles cursor, alternate-screen, application-cursor mode, bracketed paste and scrollback. The native UI gets typed cell snapshots with xterm ANSI 16/256/RGB foreground/background colors, inverse, dim, bold, italic, underline, cursor inversion, and wide-character spans. Raw escape sequences are never passed to the UI for interpretation as commands or markup. The original plain-text snapshot remains for clipboard access and tests.
- Only explicit user actions access the clipboard. The clipboard is **not** a command history, a process log, remote integration or an OSC 52 service.
- No server, external network API, login, synchronization service or daemon is created by the application.

## Unimplemented architecture milestones

The native cell model now preserves VT cell colors, common attributes and double-width spans, but logical monospaced metrics are estimated, font fallback and complex grapheme shaping require physical-display testing, and a snapshot currently rebuilds visual cells on output. Efficient diff rendering, true measured font geometry, paste confirmation UI, selection, mouse tracking, IME, accessibility semantics, saved profiles and split panes remain unimplemented.

The application does **not** implement GoreeCloud UI's Window Command Nodes. The host window manager retains genuine close/minimize/maximize controls. The fixed dark palette is sourced from current [gc-ui](https://github.com/GoreeCloud/gc-ui) alpha tokens, but automatic token synchronization and founder visual acceptance are not established.

## Operational boundaries

- GoreeShield owns privacy: no shell output leaves the host automatically.
- Wardveil owns cybersecurity: no privilege escalation or network-facing command service is introduced; untrusted terminal output remains a hostile input surface.
- GoreeCloud Mesh owns optional future integrations; none are currently required or enabled.
- GoreeCloud Identity has no network auth role in a local-shell application.
- GitHub is code authority. Google Drive owns the canonical GoreeCloud project governance documentation.
