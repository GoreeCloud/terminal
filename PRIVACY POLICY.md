# Privacy Policy

The current Development build is local-first and does not require an account.

It does not implement telemetry, analytics, crash upload, remote connections, or network reporting. Terminal contents, command history, paths, hostnames, clipboard data, search patterns, and session metadata are not sent to a GoreeCloud or third-party service by this build.

Clipboard text is read locally only when the user initiates a guarded clipboard paste. Multiline clipboard text is shown in a local modal review and is not persisted by GoreeCloud Terminal.

Search expressions are held in process memory for the active search interaction and are not persisted by this slice.

The first-run guide persists only two local onboarding fields — completion state and current step — at `$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`. This file does not contain terminal content, command history, credentials, host secrets, remote identities, clipboard content, search content, or session-restore data.

Future remote, synchronization, diagnostics, observability, or account-aware features require separate privacy review before activation.

Reusable credentials, private keys, tokens, and recovery secrets must not be stored in ordinary portable configuration or routine logs.
