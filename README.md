# GoreeCloud Terminal

GoreeCloud Terminal is a native Linux terminal and administration application for local shell work, visible execution context, remote administration, containers, development, troubleshooting, recovery, and long-running GoreeCloud infrastructure operations.

> **Lifecycle:** Development  
> **Repository:** `GoreeCloud/terminal`  
> **Platform Contract:** 0.4  
> **Glaze UI target:** 1.6.0 — consumer acceptance remains blocked

## Current Development Foundation

The current source provides:

- a native GTK 4 Linux window;
- GNOME VTE-backed local terminal sessions and asynchronous PTY shell startup;
- an explicit session/workspace source boundary;
- multiple reorderable local tabs;
- working-directory-aware new-tab creation;
- visible local/elevated, user@host, working-directory, and active-session state;
- keyboard-driven tab creation, closing, and navigation;
- copy/paste header actions and shortcuts;
- first-run onboarding with durable step progress and Help replay;
- 10,000-line default scrollback per session;
- a restrained Glaze-inspired Development presentation;
- Meson/Ninja build and install plumbing; and
- GitHub Actions compile/test/install validation.

This is Development source evidence only. Split panes, profiles, SSH, workspace/session persistence, search, safe multiline-paste review, containers, automation, protocol qualification, representative runtime acceptance, performance claims, full Glaze conformance, release qualification, and Stable/Anchor status remain unverified or planned.

## Keyboard Shortcuts

- New tab: `Ctrl+Shift+T`
- Close active tab: `Ctrl+Shift+W`
- Next tab: `Ctrl+PageDown`
- Previous tab: `Ctrl+PageUp`
- Copy: `Ctrl+Shift+C`
- Paste: `Ctrl+Shift+V`

## Build

Ubuntu 24.04 dependencies:

```bash
sudo apt update
sudo apt install -y build-essential meson ninja-build pkg-config libgtk-4-dev libvte-2.91-gtk4-dev
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
meson test -C build --print-errorlogs
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
