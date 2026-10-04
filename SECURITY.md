# Security

Do not publish reusable credentials, private keys, tokens, recovery secrets, or exploit details that would unnecessarily increase risk.

The current Development build starts local shells only. It provides visible elevated-local context, performs no automatic privilege escalation, contains no embedded remote credentials, and implements no hidden remote execution or telemetry.

Tabs and split panes are independent local shell sessions. Their labels, layout, focus, and context presentation are informational and must not become authorization signals.

Clipboard paste through GoreeCloud Terminal's clipboard action and VTE's paste-clipboard route is inspected before delivery. Primary-selection middle-click paste is claimed in GTK's capture phase before VTE's default primary-selection paste and reads GTK's primary clipboard through the same review path. Text containing line breaks requires explicit review and confirmation. Confirmed text is delivered through VTE's paste-text API, preserving terminal paste handling such as bracketed paste. This safeguard does not determine whether reviewed commands are safe.

Search patterns remain local to the active terminal process and are not transmitted externally.

Command-start navigation records only terminal cursor rows associated with VTE 0.78+ protocol-derived pre-execution events. It does not parse prompt text or arbitrary terminal output to infer command boundaries. The bounded command-row history is process-local and is not persisted. Shell lifecycle and command-start metadata remain advisory execution context and must not be treated as authorization, identity, privilege, or command-safety evidence.

Ctrl+click link/path interaction uses an allowlist of HTTP, HTTPS, mailto, and local file URI schemes. Local paths must exist before launch, remote file URI hosts are rejected, and detected terminal text is never executed as a shell command. OSC 8 hyperlink targets are treated as untrusted input and pass through the same validation boundary before the desktop handler is invoked.

The onboarding state remains separate from the local profile and workspace-restore files. Profile data and restore metadata are local configuration/state only and must not be treated as authorization signals. Profile environment overrides and optional startup commands are user-controlled process input and can materially change local shell behavior; they are not trusted security policy. Startup commands are bounded to 4,096 bytes and execute only through the selected local shell. They may be briefly visible in local process arguments while the command shell starts, so reusable credentials, tokens, private keys, and other secrets must not be placed in `startup-command` or ordinary profile configuration. Per-profile scrollback is resource-bounded to at most 1,000,000 lines.

Workspace state persists profile identifiers, selected-tab position, bounded nested split structure, active-pane selection, and per-pane working directories. It does not persist command text, terminal output, clipboard content, credentials, or running-process state. Layout parsing is bounded to 16 panes and 8 split levels per tab. State saves preserve one previous valid snapshot using user-only file permissions and recover from that snapshot only through the same versioned parser and validation path. Unsupported future schema versions continue to fail closed instead of silently loading an older backup.

Future remote administration and saved profiles must use explicit host identity, least privilege, protected key/credential handling, fail-closed verification, and clear local/remote/elevated/container differentiation.
