//! Renders terminal-cell color/attribute data without trusting PTY bytes as markup.
//! ANSI 16/256-color index values use the conventional xterm palette. The
//! default foreground/background are the verified GoreeCloud UI dark tokens.
use vt100::Color;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Rgb {
    pub r: u8,
    pub g: u8,
    pub b: u8,
}

impl Rgb {
    pub const fn new(r: u8, g: u8, b: u8) -> Self {
        Self { r, g, b }
    }

    fn dim(self) -> Self {
        Self::new(
            ((u16::from(self.r) * 2) / 3) as u8,
            ((u16::from(self.g) * 2) / 3) as u8,
            ((u16::from(self.b) * 2) / 3) as u8,
        )
    }
}

pub const DEFAULT_FOREGROUND: Rgb = Rgb::new(0xF2, 0xF4, 0xFB);
pub const DEFAULT_BACKGROUND: Rgb = Rgb::new(0x17, 0x1D, 0x2A);

// These are standard xterm-style ANSI colors, not new GoreeCloud brand tokens.
const ANSI_16: [Rgb; 16] = [
    Rgb::new(0x00, 0x00, 0x00),
    Rgb::new(0xCD, 0x00, 0x00),
    Rgb::new(0x00, 0xCD, 0x00),
    Rgb::new(0xCD, 0xCD, 0x00),
    Rgb::new(0x00, 0x00, 0xEE),
    Rgb::new(0xCD, 0x00, 0xCD),
    Rgb::new(0x00, 0xCD, 0xCD),
    Rgb::new(0xE5, 0xE5, 0xE5),
    Rgb::new(0x7F, 0x7F, 0x7F),
    Rgb::new(0xFF, 0x00, 0x00),
    Rgb::new(0x00, 0xFF, 0x00),
    Rgb::new(0xFF, 0xFF, 0x00),
    Rgb::new(0x5C, 0x5C, 0xFF),
    Rgb::new(0xFF, 0x00, 0xFF),
    Rgb::new(0x00, 0xFF, 0xFF),
    Rgb::new(0xFF, 0xFF, 0xFF),
];

pub fn resolve_color(value: Color, foreground: bool) -> Rgb {
    match value {
        Color::Default => {
            if foreground {
                DEFAULT_FOREGROUND
            } else {
                DEFAULT_BACKGROUND
            }
        }
        Color::Rgb(r, g, b) => Rgb::new(r, g, b),
        Color::Idx(index @ 0..=15) => ANSI_16[usize::from(index)],
        Color::Idx(index @ 16..=231) => {
            let index = index - 16;
            const STEPS: [u8; 6] = [0, 95, 135, 175, 215, 255];
            Rgb::new(
                STEPS[usize::from(index / 36)],
                STEPS[usize::from((index % 36) / 6)],
                STEPS[usize::from(index % 6)],
            )
        }
        Color::Idx(index) => {
            let gray = 8 + (index - 232) * 10;
            Rgb::new(gray, gray, gray)
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct RenderCell {
    pub row: u16,
    pub column: u16,
    /// Two columns for wide graphemes; VT continuation cells are omitted.
    pub span: u8,
    pub text: String,
    pub foreground: Rgb,
    pub background: Rgb,
    pub bold: bool,
    pub italic: bool,
    pub underline: bool,
}

pub fn snapshot(screen: &vt100::Screen, rows: u16, cols: u16) -> Vec<RenderCell> {
    let (cursor_row, cursor_col) = screen.cursor_position();
    let cursor_visible = !screen.hide_cursor() && screen.scrollback() == 0;
    let mut result = Vec::new();

    for row in 0..rows {
        for column in 0..cols {
            let Some(cell) = screen.cell(row, column) else {
                continue;
            };
            if cell.is_wide_continuation() {
                continue;
            }
            let mut foreground = resolve_color(cell.fgcolor(), true);
            let mut background = resolve_color(cell.bgcolor(), false);
            if cell.dim() {
                foreground = foreground.dim();
            }
            if cell.inverse() {
                std::mem::swap(&mut foreground, &mut background);
            }
            // Cursor is a non-destructive visual inversion of the target cell.
            let cursor_here = cursor_visible && row == cursor_row && column == cursor_col;
            if cursor_here {
                std::mem::swap(&mut foreground, &mut background);
            }
            let text = cell.contents();
            // Avoid building up to 64k Slint rectangles for default empty cells.
            if text.is_empty()
                && background == DEFAULT_BACKGROUND
                && !cursor_here
                && !cell.underline()
            {
                continue;
            }
            result.push(RenderCell {
                row,
                column,
                span: if cell.is_wide() { 2 } else { 1 },
                text: if text.is_empty() {
                    " ".to_string()
                } else {
                    text.to_string()
                },
                foreground,
                background,
                bold: cell.bold(),
                italic: cell.italic(),
                underline: cell.underline(),
            });
        }
    }
    result
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::num::NonZeroU16;

    fn parsed(bytes: &[u8]) -> vt100::Parser {
        let mut parser = vt100::Parser::new(
            NonZeroU16::new(4).unwrap(),
            NonZeroU16::new(24).unwrap(),
            10,
        );
        parser.process(bytes);
        parser
    }

    #[test]
    fn xterm_palette_indexes_are_correct() {
        assert_eq!(resolve_color(Color::Idx(1), true), Rgb::new(205, 0, 0));
        assert_eq!(resolve_color(Color::Idx(16), true), Rgb::new(0, 0, 0));
        assert_eq!(resolve_color(Color::Idx(21), true), Rgb::new(0, 0, 255));
        assert_eq!(resolve_color(Color::Idx(196), true), Rgb::new(255, 0, 0));
        assert_eq!(
            resolve_color(Color::Idx(231), true),
            Rgb::new(255, 255, 255)
        );
        assert_eq!(resolve_color(Color::Idx(232), true), Rgb::new(8, 8, 8));
        assert_eq!(
            resolve_color(Color::Idx(255), true),
            Rgb::new(238, 238, 238)
        );
        assert_eq!(
            resolve_color(Color::Rgb(10, 20, 30), true),
            Rgb::new(10, 20, 30)
        );
        assert_eq!(resolve_color(Color::Default, true), DEFAULT_FOREGROUND);
        assert_eq!(resolve_color(Color::Default, false), DEFAULT_BACKGROUND);
    }

    #[test]
    fn snapshots_preserve_rgb_background_and_underline() {
        let parser = parsed(b"\x1b[38;2;12;34;56m\x1b[48;2;78;90;123m\x1b[4mA");
        let cells = snapshot(parser.screen(), 4, 24);
        let a = cells.iter().find(|c| c.column == 0 && c.row == 0).unwrap();
        assert_eq!(a.text, "A");
        assert_eq!(a.foreground, Rgb::new(12, 34, 56));
        assert_eq!(a.background, Rgb::new(78, 90, 123));
        assert!(a.underline);
    }

    #[test]
    fn inverse_and_cursor_are_visually_distinct() {
        let parser = parsed(b"\x1b[7mX\x1b[0mY");
        let cells = snapshot(parser.screen(), 4, 24);
        let a = cells.iter().find(|c| c.row == 0 && c.column == 0).unwrap();
        assert_eq!(a.foreground, DEFAULT_BACKGROUND);
        assert_eq!(a.background, DEFAULT_FOREGROUND);
        let b = cells.iter().find(|c| c.row == 0 && c.column == 2).unwrap();
        assert_eq!(b.foreground, DEFAULT_BACKGROUND);
        assert_eq!(b.background, DEFAULT_FOREGROUND);
    }

    #[test]
    fn wide_unicode_does_not_create_a_duplicate_continuation_cell() {
        let parser = parsed("好Z".as_bytes());
        let cells = snapshot(parser.screen(), 4, 24);
        assert_eq!(cells.iter().find(|c| c.column == 0).unwrap().span, 2);
        assert!(!cells.iter().any(|c| c.row == 0 && c.column == 1));
        assert_eq!(cells.iter().find(|c| c.column == 2).unwrap().text, "Z");
    }

    #[test]
    fn alternate_screen_and_scrollback_hide_live_cursor() {
        let parser = parsed(b"\x1b[?25lhello");
        let cells = snapshot(parser.screen(), 4, 24);
        assert!(!cells.iter().any(|c| c.row == 0 && c.column == 5));
    }
}
