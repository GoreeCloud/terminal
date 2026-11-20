# Delivery roadmap and evidence gates

## Currently committed development source

- Native Slint Linux window with optional GTK-independent graphics stack.
- Local shell PTY, independent tabs (maximum eight), client-side tab controls and shell-exit feedback.
- VT parser, application-cursor handling, bounded scrollback navigation, guarded clipboard access, and approximate PTY resize as the Slint window changes.
- Development-stage native ANSI 16/256/RGB foreground/background colors, bold/italic/underline/inverse attributes, cursor inversion, and Unicode double-width cell placement from VT parser metadata. Font zoom (75%–175%) resizes PTY geometry using approximate metrics. All await real Linux visual validation.
- Repository Dependabot alerts and weekly Cargo/GitHub Actions update proposal configuration; updates remain subject to PR and CI gates.
- Rust CodeQL code scanning workflow on PRs, main, and a weekly schedule. Initial analysis completed successfully on commit `6a3d7bd8` (GitHub run 44824264806), with no then-open CodeQL alerts; current main-branch policy requires both the passing `linux` and `rust-codeql` jobs.
- Automated isolated Xvfb/Linux native-window smoke script added to Linux CI after the release build. Successful CI execution of this new gate is still pending on the updated commit.
- Bounded reader queues, explicit session teardown and no telemetry/network service.
- Unit tests, Linux GitHub Actions CI, desktop entry, GPLv3 source license and attribution.

## Next acceptance priorities

1. **P0:** Keep CI (format, locked tests, strict Clippy, release build, isolated X11 window smoke) green; qualify genuine Wayland and interactive Linux behavior without consuming the user's production shell state. An unsigned CI artifact on `6a3d7bd8` was checksum-verified on the authorized laptop; a private Xvfb server created a real 1030×650 X11 window and kept the process alive until the deliberate timeout (issue #3).
2. **P0:** Verify branch protection/review gates, security, shell-group lifecycle, license notices, dependency advisories and offline operation.
3. **P1:** Qualify and optimize the new native colored-cell painter: real Wayland/X11 visual checks, precise font-cell measurement, full Unicode grapheme/fallback shaping, all cursor shapes, styled output correctness, and incremental GPU-efficient diff rendering.
4. **P1:** Measure actual monospaced font metrics for PTY resizing on varying DPI, font and desktop layouts; qualify scrollback mouse/trackpad behavior and copy/selection boundaries.
5. **P1:** Add genuine accessible selection, search, clipboard paste confirmation, configurable profiles/fonts and tab-management ergonomics; then splits and session recovery controls.
6. **P2:** Mouse/SGR reporting, IME, clipboard ownership across Wayland compositors, controlled hyperlinks, Kitty keyboard/graphics support and GPU render batching.
7. **P2:** Wire native GoreeCloud UI Window Command Nodes to real window operations and qualify visual/keyboard/screen-reader behavior with founder acceptance.
8. **P2:** Signed distribution packages (Flatpak/AppImage/distro packages), SBOM, upgrade/recovery policy, reproducibility and release notes.

## Release criteria

No Stable or production claims until passing CI **on the exact release commit**, manual native Linux shell/clipboard/resize interaction, device/accessibility qualification, complete license/advisory review, applicable GitHub branch protections, and founder design acceptance. A clean PR is not a release.
