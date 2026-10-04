# GoreeCloud Terminal

GoreeCloud Terminal is a native Linux terminal and administration application for local shell work, visible execution context, remote administration, containers, development, troubleshooting, recovery, and long-running GoreeCloud infrastructure operations.

> **Lifecycle:** Development  
> **Repository:** `GoreeCloud/terminal`  
> **Platform Contract:** 0.4  
> **Glaze target:** 1.6.0 — consumer acceptance remains blocked

## Current Development Foundation

The current source provides:

- native GTK 4 Linux application/window integration;
- GNOME VTE-backed local terminal sessions and asynchronous PTY shell startup;
- multiple reorderable local tabs and nested split panes;
- active-session execution context and working-directory-aware tab/pane creation;
- protocol-derived shell ready/running/last-exit status plus bounded previous/next command-start navigation on VTE 0.78+ when compatible shell integration emits lifecycle metadata;
- searchable active-pane scrollback with literal or regular-expression matching;
- case-sensitive or case-insensitive search and wrap-around navigation;
- guarded clipboard and primary-selection paste review when pasted text contains line breaks;
- explicit Ctrl+click opening for validated hyperlinks and existing local paths;
- keyboard-driven tabs, panes, search, copy, paste, and link/path interaction;
- local launch profiles with in-app selection/reload, validated environment overrides, bounded scrollback, optional startup commands, cursor shape/blink and bold-is-bright controls, and bounded versioned workspace restoration for nested split layouts, per-pane working directories, and active-pane selection;
- first-run onboarding with durable step progress and Help replay;
- 10,000-line default scrollback per session;
- a mockup-inspired deep-navy workspace shell with GoreeCloud branding, a truthful Local Shell sidebar, closable tabs, a + tab action, compact pane/search controls, and a persistent bottom execution-context/status bar;
- Meson/Ninja build and install plumbing; and
- GitHub Actions compile/test/install validation on Ubuntu 24.04 and Debian 13.

This is Development source evidence only. Local profiles now include shell, working directory, colors/font, cursor shape/blink, bold-is-bright behavior, environment overrides, bounded scrollback, and optional startup commands; bounded nested split-layout restoration and VTE 0.78+ protocol-derived command-start navigation are also implemented. Profile-specific keybindings, remote/container profile purpose, additional appearance controls, terminal-content/process-continuity restoration, complete command navigation and shell integration across supported VTE baselines, SSH, containers, automation, protocol qualification, representative runtime acceptance, performance claims, full Glaze conformance, release qualification, and Stable/Anchor status remain incomplete.

On VTE 0.78+ (including Debian 13), compatible shell integration can drive advisory `Shell ready`, `Command running`, and last-exit-code status. Ubuntu 24.04's VTE 0.76 path retains generic session status. GoreeCloud Terminal does not infer command lifecycle from prompt text, and lifecycle metadata is not treated as a privilege or authorization signal.

Validated Development build/test environments are Ubuntu 24.04 and Debian 13 (Trixie). An owner-supplied Debian 13.7 x86_64 VPS also passed the repository build, all Meson tests, and staged install-layout validation on 2026-09-27. This evidence does not establish representative graphical desktop acceptance.

## Keyboard Shortcuts

- New tab: `Ctrl+Shift+T`
- Close active tab: `Ctrl+Shift+W`
- Next tab: `Ctrl+PageDown`
- Previous tab: `Ctrl+PageUp`
- Split left/right: `Ctrl+Shift+E`
- Split top/bottom: `Ctrl+Shift+O`
- Close active pane: `Ctrl+Shift+X`
- Focus previous pane: `Alt+Left`
- Focus next pane: `Alt+Right`
- Previous command start (VTE 0.78+ protocol integration): `Alt+Up`
- Next command start (VTE 0.78+ protocol integration): `Alt+Down`
- Search active pane: `Ctrl+Shift+F`
- Next search match: `Ctrl+G`
- Previous search match: `Ctrl+Shift+G`
- Copy: `Ctrl+Shift+C`
- Paste clipboard: `Ctrl+Shift+V`
- Open a detected link or existing path: `Ctrl+click`

Clipboard paste through GoreeCloud Terminal's paste action and VTE's Shift+Insert route is read before delivery. Primary-selection middle-click paste is claimed before VTE's default primary-selection paste and reads GTK's primary clipboard through the same review path. Text containing any line break opens a modal review instead of being sent immediately; confirmed text is delivered through VTE's paste-text processing.

Ctrl+click may open explicit OSC 8 hyperlinks, detected HTTP/HTTPS/mailto/file targets, or existing absolute/`~/`/`./`/`../` local paths with the desktop default handler. URI schemes are allowlisted and local paths must exist; GoreeCloud Terminal does not execute detected text as a shell command.

Profile `startup-command` values are user-authored local shell input. When configured, the command is run by the selected local shell before GoreeCloud Terminal replaces that command shell with the ordinary interactive shell. Keep credentials and other secrets out of startup commands and profile files.

## Build

Ubuntu 24.04 or Debian 13 dependencies:

```bash
sudo apt update
sudo apt install -y build-essential meson ninja-build pkg-config libgtk-4-dev libvte-2.91-gtk4-dev libpcre2-dev xvfb
```

Build and run:

```bash
meson setup build
meson compile -C build
./build/src/goreecloud-terminal
```

Validate:

```bash
python3 scripts/validate_repository.py
xvfb-run -a meson test -C build --print-errorlogs
```

## Documentation

- [PROJECT-SPECIFICATIONS.md](PROJECT-SPECIFICATIONS.md) — canonical requirements.
- [PROJECT-RECORD.md](PROJECT-RECORD.md) — significant history/evidence.
- [IMPLEMENTED-FEATURES.md](IMPLEMENTED-FEATURES.md) — source-implemented Development capabilities.
- [PLANNED-FEATURES.md](PLANNED-FEATURES.md) — incomplete/planned capabilities.
- [CHANGELOGS.md](CHANGELOGS.md) — source/release change history.
- [SECURITY.md](SECURITY.md) and [PRIVACY POLICY.md](PRIVACY%20POLICY.md) — current boundaries.

## License

AGPL-3.0-or-later is used as the current GoreeCloud fallback license pending any later documented project-specific licensing decision. See [LICENSE](LICENSE).
