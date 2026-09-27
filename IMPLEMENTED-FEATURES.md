# Implemented Features

These entries establish Development source implementation only. They do not establish representative runtime acceptance, release qualification, performance claims, production readiness, or Stable/Anchor status.

## Native Session Foundation

- GTK 4 application/window lifecycle.
- VTE GTK4 terminal sessions with asynchronous PTY-backed local shell startup.
- `$SHELL` selection with `/bin/sh` fallback.
- 10,000-line scrollback per session.
- Terminal hyperlink capability enabled.
- Terminal title propagation.
- Local/elevated context, user@host, working-directory, and active-session state presentation.

## Workspace, Tabs, and Panes

- Explicit `GcTerminalSession` and `GcWorkspace` source modules.
- Multiple reorderable local tabs.
- Nested left/right and top/bottom split panes.
- New tabs and panes inherit the active session working directory when available.
- Active pane drives the context ribbon and tab/window title.
- Pointer-resizable GTK pane dividers.
- Keyboard tab/pane creation, closing, and focus cycling.
- Pane layout remains ephemeral Development state.

## Scrollback Search

- `Ctrl+Shift+F` opens an active-pane search bar.
- Literal search is the default.
- Regex mode compiles VTE search regexes.
- Optional case-sensitive matching.
- Search wraps through terminal content.
- `Ctrl+G` and `Ctrl+Shift+G` navigate next/previous matches.
- Invalid expressions surface an inline search error rather than being silently accepted.

## Clipboard Paste Review

- GoreeCloud's clipboard paste action reads clipboard text before it is sent to the shell.
- VTE's `paste-clipboard` route, including Shift+Insert, is intercepted and routed through the same review path.
- Any clipboard text containing `\n` or `\r` requires a modal review.
- Review shows exact text plus line and character counts.
- Confirmed text uses VTE's paste-text API so terminal paste processing such as bracketed paste remains available.
- Single-line clipboard text pastes directly.
- Primary-selection middle-click paste is not yet intercepted and remains an explicit Development limitation.
- Pure GLib tests cover line-break review detection and line counting.

## First-Run Onboarding

- Automatic first-run guide for current local-shell behavior, execution-context indicators, keyboard shortcuts, search, paste review, and Development safety boundaries.
- Durable onboarding completion/current-step state at `$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`.
- Incomplete onboarding resumes from the persisted step.
- Help action replays the guide without resetting the saved completion state.

## Build and Validation Foundation

- Meson/Ninja build.
- GLib unit tests for context, onboarding state, and paste-safety helpers.
- Xvfb-backed GTK/VTE workspace integration test.
- GitHub Actions build/test/install validation on Ubuntu 24.04.
- Repository-governance validation.

## Glaze UI Boundary

A restrained Development presentation exists for shell chrome, tabs, panes, search, paste review, execution-context indicators, and onboarding. Full Glaze UI 1.6.0 consumer conformance and rendered/accessibility/performance acceptance remain blocked.
