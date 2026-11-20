# Dependencies, licenses and provenance

This repo contains original GoreeCloud Rust/Slint application source. Third-party code remains referenced as dependencies, **not copied or relicensed as GoreeCloud-owned source**.

| Dependency | Source | Purpose | License / review |
| --- | --- | --- | --- |
| Slint 1.19.0 | https://github.com/slint-ui/slint | Native Linux UI, compiled Slint markup | Used under GPL-3.0-only (repository distributed under GPL-3.0-only). |
| portable-pty 0.9.0 | https://github.com/wez/wezterm (now maintained as independent crate) | Unix PTY process I/O | MIT; verify resolved source and transitive licenses before release. |
| vt100 0.18.1 | https://github.com/doy/vt100-rust | Escape parser, visible cell state, scrollback | MIT; verify resolved source and transitive licenses before release. |
| anyhow 1.x | https://github.com/dtolnay/anyhow | Error context | MIT OR Apache-2.0. |

Slint has GPL-3.0-only, royalty-free and commercial options; this free/open-source application intentionally selects GPLv3. These findings are initial public-source licensing research and are **not** a third-party code audit, dependency scan, license opinion or security certification. Upstream notices and attribution must remain correct for packaged outputs.
