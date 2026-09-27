# Architecture

The Development implementation is an original GoreeCloud Linux application using GTK 4, VTE GTK4, GLib/GIO, C17, and Meson/Ninja.

GoreeCloud owns product behavior, session/workspace design, context/safety, profiles, remote administration, shell awareness, containers, automation, notifications, platform integration, and Glaze UI presentation.

VTE is a bounded supporting dependency for terminal emulation and PTY primitives.

## Current Source Boundaries

```text
main.c
  -> gc-terminal-window.c
       -> application actions + presentation
       -> active-session context ribbon
       -> onboarding launch/replay
       -> gc-workspace.c
            -> tab membership + active selection
            -> working-directory-aware tab creation
            -> gc-terminal-session.c
                 -> VTE widget + PTY shell lifecycle
                 -> session title/CWD/status
                 -> copy/paste/focus
  -> gc-onboarding.c
       -> first-run/replay presentation
       -> gc-onboarding-state.c
            -> minimal XDG-local progress persistence
  -> gc-context.c
       -> user@host
       -> shell selection
       -> privilege label
```

## State Boundary

The only GoreeCloud-owned persistent state in this slice is onboarding progress:

`$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`

It contains only `completed` and `step`. It is not a workspace/session-restore schema, profile store, command-history store, credential store, or authorization source.

## Future Separation

Split-pane layout state, profiles, shell integration, remote transport, container context, persistent workspace recovery, automation, and platform adapters remain separate future modules rather than being folded back into the window layer.
