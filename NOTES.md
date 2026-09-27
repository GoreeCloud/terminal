# Notes

- Initial architecture: C17 + GTK 4 + VTE GTK4 + Meson/Ninja.
- VTE is a supporting terminal/PTY component, not the GoreeCloud product architecture.
- Ubuntu 24.04 is the first CI baseline.
- VTE 0.78 deprecates the legacy current-directory/title property signals used by the 0.76 Ubuntu baseline; migrate deliberately when the minimum VTE version is raised.
- Do not claim GPU acceleration, protocol completeness, long-duration stability, high throughput, or Glaze UI conformance without measured/accepted evidence.
