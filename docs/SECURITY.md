# Security, privacy and threat boundaries

## Operating model

A terminal is a local command-execution application. Once the user opens it, the shell and programs they run have the **same OS permissions as that user**. This program does not promise to sandbox commands, malware, scripts, build tools or terminal content. Never execute untrusted scripts without an independent review.

- Startup selects an absolute, executable `$SHELL` or falls back to `/bin/sh`; it does not launch via `sh -c`.
- Remote shell output is parsed as untrusted bytes by `vt100`; the UI receives only text rows and never executes or resolves links in the preview.
- OSC 52 clipboard access and URL auto-opening are **not implemented**. Keyboard paste, copy/selection and clipboard integration remain out of scope until threat review.
- No telemetry, uploads, central session logs, shell command history database, remote AI assistance or automatic cloud backup of terminal content.
- 128×8192 bytes maximum queued reader-chunk payloads; process-output and scrollback sizes bounded. This cannot prevent every denial-of-service (CPU, graphics, extremely long escape streams).
- Do not run the UI as root; never grant a network-facing API or unattended privileged control as an implicit integration.

## Dependency/provenance policy

Pinned top-level direct versions: Slint 1.19.0, portable-pty 0.9.0, vt100 0.18.1; other transitive dependencies are locked by Cargo.lock once dependency resolution is performed. Run `cargo audit` against the completed dependency graph, inspect advisories and licensing. Do not treat a passing security scanner as proof of safety.

## Report issues

Use [GitHub issues](https://github.com/GoreeCloud/terminal/issues) for non-sensitive bugs. Avoid posting credentials, shell history, keys, environment values or exploit details publicly. For critical vulnerabilities, request a private disclosure channel rather than attaching secrets to an issue.
