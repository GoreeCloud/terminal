# User Manual

## Development Build

Run:

```bash
./build/src/goreecloud-terminal
```

The application starts the shell specified by `$SHELL`, falling back to `/bin/sh`.

## First Run and Help

On first run, GoreeCloud Terminal presents a four-step Development guide covering:

- local-shell behavior;
- the execution-context ribbon;
- tab and clipboard shortcuts; and
- current safety and feature limitations.

If the guide is closed before completion, the current step is stored locally and resumed later. The Help button in the header replays the guide without clearing completion state.

## Tabs

- New tab: `Ctrl+Shift+T`
- Close active tab: `Ctrl+Shift+W`
- Next tab: `Ctrl+PageDown`
- Previous tab: `Ctrl+PageUp`

A new tab attempts to start in the active tab's current working directory. Tabs can be reordered with the pointer.

Closing the final tab closes the window.

## Split Panes

- Split left/right: `Ctrl+Shift+E`
- Split top/bottom: `Ctrl+Shift+O`
- Close active pane: `Ctrl+Shift+X`
- Focus previous pane: `Alt+Left`
- Focus next pane: `Alt+Right`

A new pane attempts to start in the active pane's current working directory. Dividers can be dragged with the pointer to resize panes. Pane focus navigation currently cycles through the active tab's panes; it is not geometric directional navigation.

Closing a pane collapses its surrounding split. If a tab has only one pane, Close Pane follows the existing close-tab/window fallback.

Pane layouts are not persisted or restored yet.

## Context Ribbon

The ribbon shows:

- **Local** or **Elevated local**;
- **user@host**;
- the active session's working directory; and
- the active shell session state.

## Clipboard

- Copy selected terminal text: `Ctrl+Shift+C`
- Paste clipboard text: `Ctrl+Shift+V`

## Current Limitations

The Development build does not yet provide SSH, profiles, workspace/session restoration, search, safe multiline-paste review, container workflows, or production-grade Glaze UI acceptance.
