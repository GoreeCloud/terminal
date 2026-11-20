# Delivery roadmap and evidence gates

## Currently committed development source

- Native Slint Linux window with optional GTK-independent graphics stack.
- Local shell PTY, independent tabs (maximum eight), client-side tab controls and shell-exit feedback.
- VT parser, application-cursor handling, bounded scrollback navigation, guarded clipboard access, and approximate PTY resize as the Slint window changes.
- Bounded reader queues, explicit session teardown and no telemetry/network service.
- Unit tests, Linux GitHub Actions CI, desktop entry, GPLv3 source license and attribution.

## Next acceptance priorities

1. **P0:** Keep CI (format, locked unit/integration tests, strict Clippy, release build) green, and validate real Linux graphical runtime on Wayland/X11 without consuming the user's production shell state.
2. **P0:** Verify branch protection/review gates, security, shell-group lifecycle, license notices, dependency advisories and offline operation.
3. **P1:** Replace approximate monochrome rows with a correct cell painter: ANSI 16/256/24-bit fg/bg colors, bold/underline/inverse, Unicode double-width/grapheme alignment, supported cursor shapes and measurable redraw performance.
4. **P1:** Measure actual monospaced font metrics for PTY resizing on varying DPI, font and desktop layouts; qualify scrollback mouse/trackpad behavior and copy/selection boundaries.
5. **P1:** Add genuine accessible selection, search, clipboard paste confirmation, configurable profiles/fonts and tab-management ergonomics; then splits and session recovery controls.
6. **P2:** Mouse/SGR reporting, IME, clipboard ownership across Wayland compositors, controlled hyperlinks, Kitty keyboard/graphics support and GPU render batching.
7. **P2:** Wire native GoreeCloud UI Window Command Nodes to real window operations and qualify visual/keyboard/screen-reader behavior with founder acceptance.
8. **P2:** Signed distribution packages (Flatpak/AppImage/distro packages), SBOM, upgrade/recovery policy, reproducibility and release notes.

## Release criteria

No Stable or production claims until passing CI **on the exact release commit**, manual native Linux shell/clipboard/resize interaction, device/accessibility qualification, complete license/advisory review, applicable GitHub branch protections, and founder design acceptance. A clean PR is not a release.
