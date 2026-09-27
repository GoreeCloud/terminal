# GoreeCloud Terminal

GoreeCloud Terminal is a native Linux terminal and administration application for local shell work, visible execution context, remote administration, containers, development, troubleshooting, recovery, and long-running GoreeCloud infrastructure operations.

> **Lifecycle:** Development  
> **Repository:** `GoreeCloud/terminal`  
> **Platform Contract:** 0.4  
> **Glaze UI target:** 1.6.0 — consumer acceptance remains blocked

## Current Development Foundation

The current source provides:

- native GTK 4 Linux application/window integration;
- GNOME VTE-backed local terminal sessions and asynchronous PTY shell startup;
- multiple reorderable local tabs and nested split panes;
- active-session execution context and working-directory-aware tab/pane creation;
- searchable active-pane scrollback with literal or regular-expression matching;
- case-sensitive or case-insensitive search and wrap-around navigation;
- clipboard paste review when pasted text contains line breaks;
- keyboard-driven tabs, panes, search, copy, and paste;
- first-run onboarding with durable step progress and Help replay;
- 10,000-line default scrollback per session;
- a restrained Glaze-inspired Development presentation;
- Meson/Ninja build and install plumbing; and
- GitHub Actions compile/test/install validation.

This is Development source evidence only. Command-boundary navigation, URL/path interaction, primary-selection paste review, profiles, SSH, workspace/session persistence, containers, automation, protocol qualification, representative runtime acceptance, performance claims, full Glaze conformance, release qualification, and Stable/Anchor status remain incomplete.

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
- Search active pane: `Ctrl+Shift+F`
- Next search match: `Ctrl+G`
- Previous search match: `Ctrl+Shift+G`
- Copy: `Ctrl+Shift+C`
- Paste clipboard: `Ctrl+Shift+V`

Clipboard paste through GoreeCloud Terminal's paste action and VTE's Shift+Insert route is read before delivery. Text containing any line break opens a modal review instead of being sent immediately. Primary-selection middle-click paste is not yet routed through this review and remains a Development limitation.

## Build

Ubuntu 24.04 dependencies:

```bash
sudo apt update
sudo apt install -y build-essential meson ninja-build pkg-config libgtk-4-dev libvte-2.91-gtk4-dev xvfb
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
