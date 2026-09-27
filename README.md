# GoreeCloud Terminal

GoreeCloud Terminal is a native Linux terminal and administration application for local shell work, visible execution context, remote administration, containers, development, troubleshooting, recovery, and long-running GoreeCloud infrastructure operations.

> **Lifecycle:** Development  
> **Repository:** `GoreeCloud/terminal`  
> **Platform Contract:** 0.4  
> **Glaze UI target:** 1.6.0 — consumer acceptance remains blocked

## Current Development Foundation

The current source foundation provides:

- a native GTK 4 Linux window;
- a GNOME VTE-backed terminal widget and asynchronous PTY shell spawn;
- one local shell session using `$SHELL` with `/bin/sh` fallback;
- visible local/elevated, user@host, working-directory, and session-state context;
- 10,000-line default scrollback;
- copy/paste header actions and `Ctrl+Shift+C` / `Ctrl+Shift+V`;
- a restrained Glaze-inspired Development presentation;
- Meson/Ninja build and install plumbing; and
- GitHub Actions compile/install validation.

This is Development source evidence only. Tabs, panes, profiles, SSH, persistence, containers, automation, protocol qualification, representative runtime acceptance, performance claims, full Glaze conformance, release qualification, and Stable/Anchor status remain unverified or planned.

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
