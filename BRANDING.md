# Branding

**Product:** GoreeCloud Terminal

The canonical first-party Terminal application icon is owned by `GoreeCloud/branding-assets` at:

- canonical vector: `products/terminal/app-icon.svg`;
- accepted branding blob: `fd28f49fc0dd67e2f3e31480942d555914e8fc5b`;
- canonical consumer repository: `GoreeCloud/terminal`.

This repository vendors the exact approved SVG bytes at `data/icons/hicolor/scalable/apps/com.goreecloud.Terminal.svg` because the Linux desktop package and offline application runtime require a local derivative. The vendored SVG is not an independent branding authority and must remain synchronized with the canonical branding repository.

The desktop entry uses the icon name `com.goreecloud.Terminal`, and the native header embeds the same approved SVG through a GLib resource so Development runs do not depend on the icon already being installed into the desktop theme.

The current identity communicates command-line work and GoreeCloud product family membership. Branding does not imply remote connectivity, security acceptance, production readiness, or Stable/Anchor lifecycle state.
