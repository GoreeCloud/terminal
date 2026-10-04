# Changelogs

## Unreleased

### Added

- Native GTK 4 + VTE Development foundation.
- Explicit terminal-session and workspace modules.
- Multiple reorderable local tabs.
- Nested split panes with draggable dividers.
- Working-directory-aware tab and pane creation.
- Active-pane focus cycling and split collapse.
- Active-pane scrollback search with literal/regex and case options.
- Wrap-around next/previous search navigation.
- Multiline clipboard paste review for GoreeCloud paste actions and VTE's paste-clipboard route.
- Guarded primary-selection middle-click paste through the same multiline review path.
- Validated Ctrl+click opening for explicit hyperlinks, allowlisted URLs, and existing local paths.
- Paste-safety unit tests.
- Durable first-run onboarding with Help replay.
- Local shell execution-context status bar.
- Meson build, Xvfb-backed workspace tests, install validation, and CI.
- Debian 13 build/test/install CI coverage plus VTE 0.78+ term-property compatibility while retaining the VTE >=0.76 floor.
- Protocol-derived shell ready/running/last-exit status on VTE 0.78+ with tested state transitions and no prompt-text inference.
- Bounded protocol-derived command-start navigation on VTE 0.78+ using header glyphs and `Alt+Up` / `Alt+Down`, with no prompt-text inference.
- Mockup-inspired GoreeCloud Terminal shell with deep-navy chrome, branded titlebar, truthful Local Shell sidebar and local-session indicator, closable tabs, + tab action, compact pane/search controls, and a persistent bottom status bar.
- Canonical first-party GoreeCloud Terminal SVG identity integrated from `GoreeCloud/branding-assets` for the Linux desktop entry and embedded native header.
- Local profile selection, per-profile environment overrides, optional bounded startup commands, cursor shape/blink and bold-is-bright appearance controls, configurable bounded scrollback, and bounded versioned workspace restoration with previous-state recovery, version-1 migration, nested split layouts, per-pane working directories, and active-pane selection.
- Platform Contract 0.4 manifest and repository governance baseline.

No release, deployment, production acceptance, or Stable/Anchor status is implied.
