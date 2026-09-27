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

The Development build does not yet provide split panes, SSH, profiles, workspace/session restoration, search, safe multiline-paste review, container workflows, or production-grade Glaze UI acceptance.
