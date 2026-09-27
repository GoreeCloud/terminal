# Security

Do not publish reusable credentials, private keys, tokens, recovery secrets, or exploit details that would unnecessarily increase risk.

The current Development build starts local shells only. It provides visible elevated-local context, performs no automatic privilege escalation, contains no embedded remote credentials, and implements no hidden remote execution or telemetry.

Multiple tabs are independent local shell sessions. Their labels and context presentation are informational and must not become authorization signals.

The onboarding state file stores only completion/current-step state and must never become a credential, policy, authorization, command-history, or session-secret store.

Future remote administration and saved profiles must use explicit host identity, least privilege, protected key/credential handling, fail-closed verification, and clear local/remote/elevated/container differentiation.
