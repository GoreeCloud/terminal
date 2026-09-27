# Architecture

The initial Development implementation is an original GoreeCloud Linux application using GTK 4, VTE GTK4, GLib/GIO, C17, and Meson/Ninja.

GoreeCloud owns product behavior, session/workspace design, context/safety, profiles, remote administration, shell awareness, containers, automation, notifications, platform integration, and Glaze UI presentation.

VTE is a bounded supporting dependency for terminal emulation and PTY primitives.

Current source separation:

```text
main.c
  -> gc-terminal-window.c
       -> context ribbon
       -> actions
       -> VTE local session
  -> gc-context.c
       -> user@host
       -> shell selection
       -> privilege label
```

As the product grows, tabs/panes, sessions, profiles, shell integration, remote transport, container context, persistence/recovery, automation, and platform adapters should become distinct modules rather than accumulate in the window source.
