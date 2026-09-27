# Architecture

The Development implementation is an original GoreeCloud Linux application using GTK 4, VTE GTK4, GLib/GIO, C17, and Meson/Ninja.

GoreeCloud owns product behavior, session/workspace design, context/safety, profiles, remote administration, shell awareness, containers, automation, notifications, platform integration, and Glaze UI presentation.

VTE is a bounded supporting dependency for terminal emulation, PTY, search, and paste-processing primitives.

## Current Source Boundaries

```text
main.c
  -> gc-terminal-window.c
       -> application actions + presentation
       -> context ribbon
       -> search UI + guarded clipboard/primary-selection paste review
       -> validated link/path open requests
       -> onboarding launch/replay
       -> gc-workspace.c
            -> tab membership + active pane selection
            -> ephemeral nested GtkPaned layout trees
            -> paste-source routing (clipboard / primary selection)
            -> gc-terminal-session.c
                 -> VTE widget + PTY shell lifecycle
                 -> session title/CWD/status
                 -> VTE search + paste-text primitives
                 -> OSC 8 / regex target detection + Ctrl+click intent
       -> gc-link-utils.c
            -> URI scheme allowlist + local-path resolution
       -> gc-paste-safety.c
            -> pure multiline-review detection + line counting
  -> gc-onboarding.c
       -> first-run/replay presentation
       -> gc-onboarding-state.c
            -> minimal XDG-local progress persistence
  -> gc-context.c
       -> user@host
       -> shell selection
       -> privilege label
```

## Clipboard Safety Boundary

VTE's clipboard-paste signal is intercepted and routed to the GoreeCloud review path. A capture-phase GTK middle-click gesture claims primary-selection paste before VTE's default handler and routes GTK's primary clipboard through the same path. Single-line text is sent directly; text containing `\n` or `\r` requires modal confirmation. Confirmed text is sent with VTE's paste-text API so bracketed-paste handling remains available.

## Search Boundary

Search is active-pane-local and ephemeral. GoreeCloud Terminal compiles either escaped literal text or user-supplied regex into VTE's search engine, enables wrap-around navigation, and does not persist or transmit the expression.

## Link and Path Boundary

Link/path interaction requires explicit Ctrl+click. VTE supplies explicit OSC 8 targets or bounded regex matches, but GoreeCloud validates the target before launch. Only HTTP, HTTPS, mailto, and local file URI schemes are allowed. Local paths are canonicalized against the active pane working directory where applicable and must exist. No detected text is sent to a shell for execution.

## Persistent State Boundary

The only GoreeCloud-owned persistent state remains onboarding progress at `$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`, containing only `completed` and `step`.

Profiles, search history, clipboard history, link/path history, pane layout, shell command history, and workspace/session restore remain outside this state contract.
