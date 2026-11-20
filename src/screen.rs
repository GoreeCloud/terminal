//! VT parser and bounded snapshot of the visible text grid.
//! This first preview intentionally renders monochrome text. Truecolor, complex
//! width/grapheme layout, mouse reporting, IME, and selection are roadmap work.
pub const ROWS: u16 = 24;
pub const COLS: u16 = 80;
const SCROLLBACK: usize = 2_000;

pub struct Screen {
    parser: vt100::Parser,
    rows: u16,
    cols: u16,
}

impl Screen {
    pub fn new() -> Self {
        Self {
            parser: vt100::Parser::new(ROWS, COLS, SCROLLBACK),
            rows: ROWS,
            cols: COLS,
        }
    }

    pub fn process(&mut self, bytes: &[u8]) {
        self.parser.process(bytes);
    }

    /// A bounded, UI-safe plain-text snapshot. ANSI is handled by the parser,
    /// never passed to the UI as markup.
    pub fn visible_rows(&self) -> Vec<String> {
        let screen = self.parser.screen();
        let (cursor_row, cursor_col) = screen.cursor_position();
        screen
            .rows(0, self.cols)
            .enumerate()
            .take(self.rows as usize)
            .map(|(idx, raw)| {
                let mut chars: Vec<char> = raw.chars().collect();
                // The indicator is a preview fallback, not a fully correct grapheme-cell cursor.
                if idx == usize::from(cursor_row) && !screen.hide_cursor() {
                    let pos = usize::from(cursor_col).min(usize::from(self.cols.saturating_sub(1)));
                    let padding = pos.saturating_sub(chars.len()).min(self.cols as usize);
                    chars.extend(std::iter::repeat(' ').take(padding));
                    chars.insert(pos.min(chars.len()), '▌');
                }
                chars.into_iter().collect::<String>()
            })
            .collect()
    }

    pub fn dimensions(&self) -> (u16, u16) {
        (self.rows, self.cols)
    }
    pub fn resize(&mut self, rows: u16, cols: u16) {
        let rows = rows.clamp(5, 200);
        let cols = cols.clamp(20, 320);
        self.parser.set_size(rows, cols);
        self.rows = rows;
        self.cols = cols;
    }
}

impl Default for Screen {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn ansi_codes_not_visible_as_raw_escape_text() {
        let mut screen = Screen::new();
        screen.process(b"\x1b[31mhello\x1b[0m\r\nworld");
        let view = screen.visible_rows().join("\n");
        assert!(view.contains("hello"));
        assert!(view.contains("world"));
        assert!(!view.contains("\x1b"));
    }
    #[test]
    fn bounded_resize() {
        let mut screen = Screen::new();
        screen.resize(0, 65535);
        assert_eq!(screen.dimensions(), (5, 320));
    }
}

[executed on device: personal-laptop-ideapad3-01 (f760ad53-00b1-412e-9f38-92d15b9aa95c)]