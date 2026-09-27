# Dependencies

## GTK 4

Purpose: native Linux application/window/widget/input/clipboard/accessibility toolkit.  
Build target: `gtk4`  
Initial minimum: 4.12  
Upstream license: LGPL-2.1-or-later.

## GNOME VTE

Purpose: terminal-emulation widget and PTY/rendering/input primitives.  
Build target: `vte-2.91-gtk4`  
API namespace: 3.91  
Initial minimum baseline: 0.76  
Upstream license: LGPL-3.0-or-later.

## GLib/GIO

Purpose: runtime utilities, actions/application model, environment/host helpers, and tests.  
Initial minimum: 2.76.

## PCRE2

Purpose: compile-time regex flags for VTE terminal match expressions used by validated link/path detection. VTE remains responsible for match execution.  
Build target: `libpcre2-8`  
Initial baseline: distribution-supported PCRE2 from Ubuntu 24.04.

## Build

Meson 1.0+, Ninja, pkg-config, C17 compiler.

Dependencies remain supporting components. Their presence does not establish product-feature acceptance.
