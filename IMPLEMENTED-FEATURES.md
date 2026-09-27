# Implemented Features

These entries establish Development source implementation only. They do not establish representative runtime acceptance, release qualification, performance claims, production readiness, or Stable/Anchor status.

## Native Foundation

- GTK 4 application/window lifecycle.
- VTE GTK4 terminal widget and asynchronous PTY-backed local shell startup.
- `$SHELL` selection with `/bin/sh` fallback.
- 10,000-line scrollback.
- Terminal hyperlink capability enabled.
- Terminal title propagation.
- Copy/paste actions and shortcuts.
- Local/elevated context, user@host, working-directory, and session-state presentation.
- Meson/Ninja build, GLib context tests, install-layout validation, and GitHub Actions CI.

Evidence: `src/`, `tests/`, `meson.build`, and `.github/workflows/build.yml`.

## Glaze UI Boundary

A restrained Development presentation exists for shell chrome and context indicators. Full Glaze UI 1.6.0 consumer conformance and rendered/accessibility/performance acceptance remain blocked.
