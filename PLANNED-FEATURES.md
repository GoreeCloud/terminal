# Planned Features

The following remain planned or materially incomplete:

- complete command-boundary navigation across supported VTE baselines; bounded VTE 0.78+ protocol-derived command-start navigation is now implemented, while the VTE 0.76 compatibility path remains generic;
- remaining profile options for remote/container purposes, broader appearance settings, and richer shortcut replacement/disable semantics; optional bounded startup commands, environment overrides, additive profile-local keybindings, cursor shape/blink, bold-is-bright behavior, and scrollback limits are now implemented;
- complete shell integration across supported VTE baselines for command boundaries, process context, reliable CWD, and navigation; VTE 0.78+ lifecycle, exit-status, and bounded command-start navigation remain a Development slice rather than complete shell integration;
- remaining restore work for terminal content, long-running process continuity, clean-target migration, export/portability, and broader recovery acceptance; nested split layout and active-pane restoration are now implemented with bounded schema-v2 persistence and version-1 migration;
- general settings beyond onboarding, local profiles, and bounded workspace restore;
- SSH host profiles, host verification, reconnection, and secure key/credential handling;
- remote/container and per-environment visual differentiation; local/elevated status is already surfaced in the Development shell;
- container-shell launch and host/container context;
- long-running-command/background notifications;
- terminal graphics and protocol compatibility validation;
- extension and automation APIs;
- performance/latency/large-log/long-duration measurement;
- complete Glaze 1.6.0 consumer acceptance, including representative rendered, screen-reader, large-text/reflow, high-contrast, focus, adaptive, performance, and identity review; descriptive source-level labels for icon/glyph controls do not by themselves satisfy runtime accessibility acceptance;
- all nine Integral Platform System integrations/acceptance;
- Linux packaging, signed releases, rollback/recovery evidence, and lifecycle promotion.
