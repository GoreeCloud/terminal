# User Manual

## Development Build

Run:

```bash
./build/src/goreecloud-terminal
```

The application starts the shell specified by `$SHELL`, falling back to `/bin/sh`.

## First Run and Help

The first-run guide covers local-shell behavior, execution context, tabs, panes, search, clipboard-paste review, shortcuts, and current Development limitations. If closed before completion, the current onboarding step is stored locally and resumed later. Help replays the guide without clearing completion state.

## Tabs and Split Panes

- New tab: `Ctrl+Shift+T`
- Close active tab: `Ctrl+Shift+W`
- Next/previous tab: `Ctrl+PageDown` / `Ctrl+PageUp`
- Split left/right: `Ctrl+Shift+E`
- Split top/bottom: `Ctrl+Shift+O`
- Close active pane: `Ctrl+Shift+X`
- Focus previous/next pane: `Alt+Left` / `Alt+Right`

New tabs and panes attempt to inherit the active pane's current working directory. Pane dividers can be dragged to resize them. Pane focus navigation currently cycles rather than performing geometric directional selection.

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

## Clipboard Paste Review

`Ctrl+Shift+V` and Shift+Insert use the guarded clipboard path.

Single-line text is pasted directly. If the clipboard text contains a newline or carriage return, GoreeCloud Terminal opens a modal review that shows the exact text plus line and character counts. Choose **Paste anyway** to send the text or **Cancel** to discard it.

Primary-selection middle-click paste is not yet intercepted by this review path. Treat that as a current Development limitation when working with sensitive shells.

## Context Ribbon

The ribbon shows local/elevated privilege state, `user@host`, the active pane's working directory, and active shell state.

## Current Limitations

The Development build does not yet provide command-boundary navigation, URL/path actions, primary-selection paste review, SSH, profiles, workspace/session restoration, container workflows, or production-grade Glaze UI acceptance.
