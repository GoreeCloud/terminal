# Implemented Features

These entries establish Development source implementation only. They do not establish representative runtime acceptance, release qualification, performance claims, production readiness, or Stable/Anchor status.

## Native Session Foundation

- GTK 4 application/window lifecycle.
- VTE GTK4 terminal sessions with asynchronous PTY-backed local shell startup.
- `$SHELL` selection with `/bin/sh` fallback.
- 10,000-line default scrollback with a bounded per-profile override from 0 through 1,000,000 lines.
- Terminal hyperlink capability enabled.
- Terminal title propagation.
- Local/elevated context, user@host, working-directory, and active-session state presentation in a persistent bottom status bar.

## Workspace, Tabs, and Panes

- Explicit `GcTerminalSession` and `GcWorkspace` source modules.
- Multiple reorderable local tabs with explicit close controls and a mockup-style + tab action.
- Nested left/right and top/bottom split panes.
- New tabs and panes inherit the active session working directory when available.
- Active pane drives the bottom status bar and tab/window title.
- Pointer-resizable GTK pane dividers.
- Keyboard tab/pane creation, closing, and focus cycling.
- Pane layout remains ephemeral Development state.

## Local Profiles and Tab Restore

- Local launch profiles can be selected and reloaded from the sidebar.
- Profile environment overrides are validated and merged into the inherited local process environment for newly launched tabs and panes.
- Per-profile scrollback limits are configurable from 0 through 1,000,000 lines; the default remains 10,000 lines.
- Tabs restore their profile, nested split-pane layout, per-pane working directories, active-pane selection, and selected-tab position after restart.
- Workspace persistence uses a versioned schema with bounded layout parsing (up to 16 panes and 8 split levels per tab); version-1 top-level-tab state is migrated into the current schema on load.
- Each successful save preserves the previous valid workspace snapshot as a local backup; load falls back to that snapshot when the primary state is missing or malformed.
- Terminal output, command history, and running-process continuity remain outside this restore slice.

## Scrollback Search

- `Ctrl+Shift+F` opens an active-pane search bar.
- Literal search is the default.
- Regex mode compiles VTE search regexes.
- Optional case-sensitive matching.
- Search wraps through terminal content.
- `Ctrl+G` and `Ctrl+Shift+G` navigate next/previous matches.
- Invalid expressions surface an inline search error rather than being silently accepted.

## Links and Local Paths

- Ctrl+click is required before GoreeCloud Terminal requests an external open.
- Explicit OSC 8 hyperlinks are validated before launch.
- Detected HTTP/HTTPS/mailto/file targets use an allowlisted URI boundary.
- Existing absolute, `~/`, `./`, and `../` local paths can be resolved against the active pane working directory and opened as local file URIs.
- Local paths must exist before an open request is handed to the desktop.
- Detected text is never executed as a shell command by this interaction path.

## Guarded Paste Review

- GoreeCloud's clipboard paste action reads clipboard text before it is sent to the shell.
- VTE's `paste-clipboard` route, including Shift+Insert, is intercepted and routed through the same review path.
- Primary-selection middle-click paste is claimed in GTK's capture phase before VTE's default primary-selection paste and routed through GTK's primary clipboard.
- Any clipboard or primary-selection text containing `\n` or `\r` requires a modal review.
- Review shows exact text plus line and character counts.
- Confirmed text uses VTE's paste-text API so terminal paste processing such as bracketed paste remains available.
- Single-line clipboard or primary-selection text pastes directly.
- Pure GLib tests cover line-break review detection and line counting.

## Shell Lifecycle Awareness

- VTE 0.78+ shell lifecycle term properties are consumed for prompt-ready, pre-execution, and post-execution events.
- The active session status can show shell-ready state, command-running state, and the last validated shell exit status.
- Exit status is accepted only when the VTE post-execution property is set and the value is within the shell-status range `0..255`.
- Term-property reset notifications do not fabricate lifecycle events.
- VTE's legacy OSC 777 translation is enabled on VTE 0.78+ for compatible existing shell integration.
- The lifecycle model is a pure GLib module with focused unit tests.
- On VTE 0.78+ with compatible shell integration, each protocol-derived pre-execution event records the terminal's absolute cursor row in a bounded 512-entry command-start history.
- Header up/down glyphs and `Alt+Up` / `Alt+Down` navigate to the previous/next recorded command start; the actions remain disabled until protocol-derived command metadata exists.
- The Ubuntu 24.04 / VTE 0.76 compatibility path remains generic and does not expose command-start navigation.
- Protocol-derived lifecycle and command-start metadata are advisory execution context, not authorization, privilege, or security-boundary signals.

## First-Run Onboarding

- Automatic first-run guide for current local-shell behavior, execution-context indicators, keyboard shortcuts, search, paste review, and Development safety boundaries.
- Durable onboarding completion/current-step state at `$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`.
- Incomplete onboarding resumes from the persisted step.
- Help action replays the guide without resetting the saved completion state.

## Build and Validation Foundation

- Meson/Ninja build.
- GLib unit tests for context, onboarding state, and paste-safety helpers.
- Xvfb-backed GTK/VTE workspace integration tests, including clipboard-versus-primary paste-source routing.
- GitHub Actions build/test/install validation on Ubuntu 24.04 and Debian 13.
- VTE 0.78+ term-property compatibility for working-directory/title tracking while preserving the VTE >=0.76 baseline.
- Repository-governance validation.

## Glaze Boundary

A mockup-inspired Development presentation exists for the deep-navy shell chrome, branded titlebar, truthful Local Shell sidebar, local-session indicator, closable tab strip, terminal surfaces, bottom execution-context/status bar, search, paste review, and onboarding. Icon-only and glyph controls for tab creation/closing, header actions, profile reload, and search navigation expose descriptive accessibility labels in source. The presentation deliberately does not fabricate remote, cloud, cluster, history, or persistence state. Complete Glaze 1.6.0 consumer conformance and representative rendered, screen-reader, large-text/reflow, high-contrast, focus, and performance acceptance remain blocked.
