# User Manual

## Development Build

Run:

```bash
./build/src/goreecloud-terminal
```

The application starts the shell specified by `$SHELL`, falling back to `/bin/sh`.

## First Run and Help

The first-run guide covers local-shell behavior, execution context, tabs, panes, search, guarded clipboard/primary-selection paste review, shortcuts, and current Development limitations. If closed before completion, the current onboarding step is stored locally and resumed later. Help replays the guide without clearing completion state.

## Tabs and Split Panes

- New tab: `Ctrl+Shift+T`
- Close active tab: `Ctrl+Shift+W`
- Next/previous tab: `Ctrl+PageDown` / `Ctrl+PageUp`
- Split left/right: `Ctrl+Shift+E`
- Split top/bottom: `Ctrl+Shift+O`
- Close active pane: `Ctrl+Shift+X`
- Focus previous/next pane: `Alt+Left` / `Alt+Right`

New tabs and panes attempt to inherit the active pane's current working directory. Pane dividers can be dragged to resize them. Pane focus navigation currently cycles rather than performing geometric directional selection.

## Local Profiles and Tab Restore

The sidebar Profile selector chooses the local profile used for a new tab, and the adjacent reload control refreshes profile configuration without restarting the application. The current profile surface covers display name, local shell selection, working directory, terminal font, foreground color, background color, environment overrides, optional `startup-command`, additional profile-local `keybindings`, cursor appearance, and `scrollback-lines`. Environment entries use `NAME=value` list items and are merged over the inherited local environment for newly launched tabs and panes. `scrollback-lines` accepts values from 0 through 1,000,000; profiles that omit it use 10,000 lines. `startup-command` accepts a non-empty command string up to 4,096 bytes. GoreeCloud Terminal runs it through the selected local shell and then replaces that command shell with the ordinary interactive shell when the startup command returns normally. `cursor-shape` accepts `block`, `ibeam`, or `underline`; `cursor-blink` accepts `system`, `on`, or `off`; and `bold-is-bright` accepts a boolean value. Split panes inherit the owning tab profile and therefore the same profile appearance, startup behavior, and additional keybindings.

`keybindings` is a semicolon-separated string list of `action=accelerator` entries, for example `keybindings=search=<Control><Alt>f;split-horizontal=<Control><Alt>e;`. Supported action names are `new-tab`, `close-tab`, `split-horizontal`, `split-vertical`, `close-pane`, `next-pane`, `previous-pane`, `previous-command`, `next-command`, `next-tab`, `previous-tab`, `search`, `search-next`, `search-previous`, `search-close`, `copy`, `paste`, and `help`. Entries are validated when profiles load, profiles are limited to 32 additional bindings, and each entry is limited to 192 bytes. These bindings are additive: the built-in application shortcuts remain available. A profile shortcut invokes the same existing window action rather than arbitrary shell text; for example, `new-tab` still follows the profile currently selected in the sidebar.

After restart, GoreeCloud Terminal restores tabs, each tab's profile, nested horizontal/vertical split layout, each pane's working directory, the active pane, and the selected-tab position. Layout persistence is bounded to 16 panes and 8 split levels per tab. Existing version-1 top-level-tab state is migrated into the current schema when loaded. A previous valid local workspace snapshot is retained so a missing or malformed primary state file can fall back to the prior snapshot.

Terminal output, command history, and running processes are not restored by this Development slice. A saved working directory that no longer exists falls back safely rather than being treated as authoritative runtime state.

## Search

Press `Ctrl+Shift+F` to show search for the active pane.

- Text is treated literally by default.
- Enable **Regex** for regular-expression search.
- Enable **Case** for case-sensitive matching.
- `Ctrl+G` moves to the next match.
- `Ctrl+Shift+G` moves to the previous match.
- Search wraps through terminal content.
- Escape closes the search bar and clears the active pane's search expression while focus is inside the search UI; otherwise Escape continues to go to the active terminal.

Invalid regular expressions are shown inline and are not applied.

## Command Start Navigation

On VTE 0.78+ with compatible shell integration, GoreeCloud Terminal records command-start rows only when the terminal protocol reports a pre-execution event. Use the header up/down glyphs or `Alt+Up` / `Alt+Down` to move to the previous or next recorded command start in the active pane. The controls stay disabled when protocol-derived command metadata is unavailable.

This feature does not infer command boundaries from prompt text or arbitrary terminal output. The VTE 0.76 compatibility path does not provide command-start navigation.

## Links and Local Paths

Ctrl+click a recognized target in the active pane to request opening it with the desktop default handler.

Supported targets in this Development slice are:

- explicit OSC 8 hyperlinks after validation;
- `http://` and `https://` URLs;
- `mailto:` links;
- local `file://` URIs;
- existing absolute paths; and
- existing `~/`, `./`, and `../` paths resolved from the active pane working directory.

Other URI schemes are rejected. Local paths must exist before they are opened. Detected text is not executed through the shell. Opening an HTTP/HTTPS or mail link hands it to the configured external application, which may perform network activity under that application's policies.

## Guarded Paste Review

`Ctrl+Shift+V` and Shift+Insert use the guarded clipboard path. Middle-click primary-selection paste is captured before VTE's default primary-selection paste and reads GTK's primary clipboard through the same safety path.

Single-line text is pasted directly. If clipboard or primary-selection text contains a newline or carriage return, GoreeCloud Terminal opens a modal review that shows the exact text plus line and character counts. Choose **Paste anyway** to send the text or **Cancel** to discard it.

## Workspace Shell and Status Bar

The mockup-inspired shell uses a deep-navy GoreeCloud titlebar, a truthful **Local shell** indicator, a left workspace sidebar, compact split/search/copy/paste/help controls, and a notebook tab strip with an explicit + control and per-tab close buttons. The sidebar exposes only currently implemented local actions: new tab, left/right split, top/bottom split, and scrollback search. The bottom status bar shows local/elevated privilege state, `user@host`, the active pane's working directory, active shell state, and the current Development lifecycle.

The presentation does not imply remote connectivity, cloud-provider state, Kubernetes clusters, persistent command history, or saved sessions. Those capabilities remain outside the current Development boundary.

## Current Limitations

The Development build does not yet provide complete command-boundary navigation across all supported VTE baselines, SSH, profile-specific keybindings, remote/container profile purpose, broader profile appearance controls, terminal-content or process-continuity restoration, container workflows, or complete Glaze 1.6.0 consumer acceptance.
