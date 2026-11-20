# Security and privacy boundaries

A terminal runs commands with the user's own operating-system privileges. **It is not a sandbox.** Do not run code you have not reviewed. The app does not attempt to scan or block arbitrary user-entered shell commands.

## Current controls

- The shell starts by absolute executable path via portable-pty, not by shell-string evaluation.
- Each tab owns an independent local PTY. Dropping a tab kills its direct shell child, but independent grandchildren may survive; full process-group cleanup needs verification.
- Shell output is parsed by vt100 and rendered as plain-text rows; raw terminal content cannot become executable Slint/HTML markup.
- Reader channels are bounded (128 × 8 KiB chunks per tab); scrollback is limited to 2,000 rows. Up to eight tabs are supported. A noisy child can still consume significant CPU; memory caps are not complete denial-of-service protection.
- Clipboard access is explicit (Ctrl+Shift+C, Ctrl+Shift+V or buttons). Copy is limited to 128 KiB of visible text. Paste is limited to 32 KiB, denies terminal control characters, and blocks unbracketed multiline input to reduce accidental command injection.
- Shell-enabled bracketed paste surrounds sanitized content with the conventional delimiters. Users must still inspect pasted commands.
- No OSC 52 clipboard handling, hyperlink opening, telemetry, session upload, automatic shell-history recording, remote command service, or cloud synchronization.
- No default privileged service, sandbox-bypass mechanism or user data upload.

## Current limitations and follow-up

Pure Wayland without XWayland may not support the default arboard backend. No unsafe helper shell commands are run as fallback. Test native shell invocation, kill behavior, clipboard ownership, font-scale geometry, scrolling and color escape handling on real hardware. Add fuzz tests for VT streams and dependency vulnerability screening before stable acceptance.

The Linux application has not completed independent security assessment and should not be used as the sole terminal for sensitive credentials. Never share real shell history, environment variables, private keys, clipboard secrets or unredacted terminal screenshots in public issues.

## Dependency handling

Top-level dependencies are pinned where possible, and Cargo.lock is committed. See [third-party notices](THIRD_PARTY.md). This is not proof that every transitive package is vulnerability-free. Run a verified advisory and license scanner during a release candidate, without automatically executing unreviewed third-party scripts.
