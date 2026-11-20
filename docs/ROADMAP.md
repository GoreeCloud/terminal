# Delivery roadmap (evidence-based)

## Implemented in this preview source
- Rust package and native Slint window; no web runtime or server.
- Linux PTY shell creation, terminal screen parser, basic keyboard translation.
- Bounded output queue, capped scrollback, live text grid, status row.
- Unit tests, GitHub Actions Linux CI workflow, application desktop entry.
- GPLv3 license and upstream dependency documentation.

## Next engineering priorities (NOT implemented yet)
1. **P0:** Get green `cargo fmt`, `cargo test`, `cargo clippy`, `cargo build` on GitHub Actions and at least one real Linux device; resolve all bugs; verify the committed Cargo.lock remains reproducible.
2. **P0:** Security review of dependencies and terminal escape handling, child lifecycle, input correctness, device availability, high-contrast and keyboard accessibility.
3. **P1:** Real cell painting with foreground/background colors, cursor shape, bold/underline, wide characters, selection and clipboard.
4. **P1:** PTY resize and window geometry updates; Wayland/X11 DPI qualification, font preferences, scrollback interactions.
5. **P1:** Tabs, splits, search, command palette, copy/paste, configurable shell and persistent profiles.
6. **P2:** Mouse reporting, bracketed paste, IME, hyperlinks with consent, Kitty keyboard/graphics compatibility, GPU performance profiling.
7. **P2:** GoreeCloud UI native Window Command Nodes implementation with verified minimize/maximize/close semantics and founder visual acceptance.
8. **P2:** Packaging (Flatpak/AppImage/distro formats), SBOM, secure update checks, release signing and supported-desktop acceptance.

**Release gates:** build + real device test + accessibility review + licensing/provenance + branch protection + dependency audit + founder visual approval. A merged PR is not a release or deployment.
