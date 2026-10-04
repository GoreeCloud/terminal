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

The sidebar Profile selector chooses the local profile used for a new tab, and the adjacent reload control refreshes profile configuration without restarting the application. The current profile surface covers display name, local shell selection, working directory, terminal font, foreground color, and background color.

After restart, GoreeCloud Terminal restores top-level tabs, each tab's profile and working directory, and the selected-tab position. Terminal output, command history, running processes, and split-pane layout are not restored by this Development slice.

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

The Development build does not yet provide command-boundary navigation, SSH, advanced profile options, split-layout or process restoration, container workflows, or complete Glaze 1.6.0 consumer acceptance.
