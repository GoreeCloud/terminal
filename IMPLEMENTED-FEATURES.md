# Implemented Features

These entries establish Development source implementation only. They do not establish representative runtime acceptance, release qualification, performance claims, production readiness, or Stable/Anchor status.

## Native Session Foundation

- GTK 4 application/window lifecycle.
- VTE GTK4 terminal sessions with asynchronous PTY-backed local shell startup.
- `$SHELL` selection with `/bin/sh` fallback.
- 10,000-line scrollback per session.
- Terminal hyperlink capability enabled.
- Terminal title propagation.
- Copy/paste actions and shortcuts.
- Local/elevated context, user@host, working-directory, and active-session state presentation.

## Workspace and Tabs

- Explicit `GcTerminalSession` and `GcWorkspace` source modules.
- Multiple reorderable local tabs.
- New tabs inherit the active session working directory when available.
- Active tab drives the window title and context ribbon.
- `Ctrl+Shift+T` creates a tab.
- `Ctrl+Shift+W` closes the active tab or closes the window when it is the final tab.
- `Ctrl+PageDown` and `Ctrl+PageUp` navigate tabs.

## Split Panes

- Nested left/right and top/bottom split panes inside each tab.
- New panes inherit the active pane working directory when available.
- GTK paned dividers provide pointer resizing.
- Active pane drives the context ribbon and tab/window title.
- `Ctrl+Shift+E` splits left/right.
- `Ctrl+Shift+O` splits top/bottom.
- `Ctrl+Shift+X` closes the active pane; when only one pane remains, the existing tab/window close behavior is used.
- `Alt+Left` and `Alt+Right` cycle focus through panes in the active tab.
- Pane layout is ephemeral Development state and is not restored after restart.

## First-Run Onboarding

- Automatic first-run guide for current local-shell behavior, execution-context indicators, keyboard shortcuts, and Development safety boundaries.
- Durable onboarding completion/current-step state at `$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`.
- Incomplete onboarding resumes from the persisted step.
- Help action replays the guide without resetting the saved completion state.
- Focused GLib tests cover missing-state defaults and state-file round trips.

## Build and Validation Foundation

- Meson/Ninja build.
- GLib unit tests for context and onboarding-state helpers.
- GitHub Actions build/test/install validation on Ubuntu 24.04.
- Repository-governance validation.

## Glaze UI Boundary

A restrained Development presentation exists for shell chrome, tabs, execution-context indicators, and onboarding. Full Glaze UI 1.6.0 consumer conformance and rendered/accessibility/performance acceptance remain blocked.
