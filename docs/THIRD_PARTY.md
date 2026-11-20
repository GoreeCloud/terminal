# Third-party sources, licenses and provenance

GoreeCloud Terminal contains original Rust/Slint application code; it references third-party crates via Cargo. It does not copy Ghostty source or claim ownership of upstream components.

| Library | Version | Origin / function | License |
| --- | --- | --- | --- |
| [Slint](https://github.com/slint-ui/slint) | 1.19.0 | Native desktop UI, Winit/FemtoVG rendering | GPL-3.0-only usage path; GoreeCloud Terminal source is GPL-3.0-only |
| [portable-pty](https://github.com/wez/wezterm) | 0.9.0 | Unix pseudoterminal sessions | MIT (verify packaged notices) |
| [vt100](https://github.com/doy/vt100-rust) | 0.18.1 | VT screen/escape-state parser | MIT (verify packaged notices) |
| [arboard](https://github.com/1Password/arboard) | 3.7.0 | User-initiated system clipboard operations | MIT OR Apache-2.0 |
| [anyhow](https://github.com/dtolnay/anyhow) | 1.x | Rust error context | MIT OR Apache-2.0 |

The standard library and Cargo-resolved transitive dependencies remain bound by their individual licenses. Cargo.lock records resolved versions, not legal clearance, a supply-chain scan or malware certification. Slint supports multiple licensing paths; this repository intentionally uses the GPLv3 path and includes the full LICENSE. Do not relicense vendored upstream code as original GoreeCloud work.

Prior to stable distribution: audit full dependency tree, package appropriate third-party notices, check security advisories, verify exact dependency source and provenance, and validate compatible license obligations.
