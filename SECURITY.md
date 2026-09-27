# Security

Do not publish reusable credentials, private keys, tokens, recovery secrets, or exploit details that would unnecessarily increase risk.

The current Development build starts local shells only. It provides visible elevated-local context, performs no automatic privilege escalation, contains no embedded remote credentials, and implements no hidden remote execution or telemetry.

Tabs and split panes are independent local shell sessions. Their labels, layout, focus, and context presentation are informational and must not become authorization signals.

Clipboard paste through GoreeCloud Terminal's clipboard action and VTE's paste-clipboard route is inspected before delivery. Text containing line breaks requires explicit review and confirmation. Confirmed text is delivered through VTE's paste-text API, preserving terminal paste handling such as bracketed paste. This safeguard does not yet cover primary-selection middle-click paste, and it does not determine whether reviewed commands are safe.

Search patterns remain local to the active terminal process and are not transmitted externally.

Ctrl+click link/path interaction uses an allowlist of HTTP, HTTPS, mailto, and local file URI schemes. Local paths must exist before launch, remote file URI hosts are rejected, and detected terminal text is never executed as a shell command. OSC 8 hyperlink targets are treated as untrusted input and pass through the same validation boundary before the desktop handler is invoked.

The onboarding state file stores only completion/current-step state and must never become a credential, policy, authorization, command-history, or session-secret store.

Future remote administration and saved profiles must use explicit host identity, least privilege, protected key/credential handling, fail-closed verification, and clear local/remote/elevated/container differentiation.
