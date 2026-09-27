# Security

Do not publish reusable credentials, private keys, tokens, recovery secrets, or exploit details that would unnecessarily increase risk.

The initial foundation starts a local shell only. It provides visible elevated-local context, performs no automatic privilege escalation, contains no embedded remote credentials, and implements no hidden remote execution or telemetry.

Future remote administration and saved profiles must use explicit host identity, least privilege, protected key/credential handling, fail-closed verification, and clear local/remote/elevated/container differentiation.
