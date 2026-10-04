# Privacy Policy

The current Development build is local-first and does not require an account.

It does not implement telemetry, analytics, crash upload, remote connections, or network reporting. Terminal contents, command history, paths, hostnames, clipboard data, search patterns, and session metadata are not sent to a GoreeCloud or third-party service by this build.

Clipboard or primary-selection text is read locally only when the user initiates the corresponding guarded paste action. Multiline paste text is shown in a local modal review and is not persisted by GoreeCloud Terminal.

Search expressions are held in process memory for the active search interaction and are not persisted by this slice.

Detected link/path text is not persisted by GoreeCloud Terminal. Ctrl+click hands a validated target to the desktop default handler. External applications may perform network or other activity according to their own behavior after the user explicitly requests the open.

The first-run guide persists completion state and current step at `$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`.

User-authored local profiles are read from `$XDG_CONFIG_HOME/goreecloud-terminal/profiles.ini`. The current profile fields are profile identity/name, local shell path, working directory, terminal font, foreground color, background color, environment overrides, and bounded scrollback. Environment values remain local and are passed only to the local shell process launched for that profile.

Bounded workspace-restore state is stored at `$XDG_STATE_HOME/goreecloud-terminal/workspace.ini`, with one previous valid snapshot retained at the adjacent `.bak` path for local recovery. The current schema stores a format version, tab count, selected-tab position, profile identifiers, bounded nested split structure, active-pane selection, and per-pane working directories. Version-1 top-level-tab state is accepted and migrated into the current in-memory schema on load. The files do not contain terminal output, command history, command text, running-process state, clipboard content, search content, credentials, or remote identity.

Future remote, synchronization, diagnostics, observability, or account-aware features require separate privacy review before activation.

Reusable credentials, private keys, tokens, and recovery secrets must not be stored in ordinary portable configuration or routine logs.
