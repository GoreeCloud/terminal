//! VT state and bounded plain-text/colored-cell snapshots.
//! Rendering uses the VT parser's actual cells, not plaintext char indexes.
//! Mouse reporting, pixel-accurate font measurement and IME remain future work.
use std::num::NonZeroU16;

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
            parser: vt100::Parser::new(
                NonZeroU16::new(ROWS).expect("terminal rows nonzero"),
                NonZeroU16::new(COLS).expect("terminal columns nonzero"),
                SCROLLBACK,
            ),
            rows: ROWS,
            cols: COLS,
        }
    }

    pub fn process(&mut self, bytes: &[u8]) {
        self.parser.process(bytes);
    }
    /// Snapshot of visible VT cells including xterm colors, attributes and wide cell spans.
    pub fn render_cells(&self) -> Vec<crate::render::RenderCell> {
        crate::render::snapshot(self.parser.screen(), self.rows, self.cols)
    }

    /// Browse the local VT parser's bounded scrollback (not shell history).
    pub fn scroll_up(&mut self, lines: usize) {
        if self.parser.screen().alternate_screen() {
            return;
        }
        let current = self.parser.screen().scrollback();
        self.parser
            .screen_mut()
            .set_scrollback(current.saturating_add(lines));
    }
    pub fn scroll_down(&mut self, lines: usize) {
        let current = self.parser.screen().scrollback();
        self.parser
            .screen_mut()
            .set_scrollback(current.saturating_sub(lines));
    }
    pub fn scroll_to_bottom(&mut self) {
        self.parser.screen_mut().set_scrollback(0);
    }
    pub fn scrollback_offset(&self) -> usize {
        self.parser.screen().scrollback()
    }
    pub fn application_cursor(&self) -> bool {
        self.parser.screen().application_cursor()
    }
    pub fn bracketed_paste(&self) -> bool {
        self.parser.screen().bracketed_paste()
    }
    /// Plain text only, with no raw escape codes or UI cursor overlay.
    pub fn copy_visible_text(&self) -> String {
        self.parser.screen().contents()
    }
    /// Approximate cells for the fixed 15px monospace font and current Slint chrome.
    pub fn grid_for_window(width: f32, height: f32) -> (u16, u16) {
        let cols = ((width.max(0.0) - 60.0) / 9.2).floor() as u16;
        let rows = ((height.max(0.0) - 216.0) / 22.0).floor() as u16;
        (rows.clamp(5, 200), cols.clamp(20, 320))
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
                if idx == usize::from(cursor_row)
                    && !screen.hide_cursor()
                    && screen.scrollback() == 0
                {
                    let pos = usize::from(cursor_col).min(usize::from(self.cols.saturating_sub(1)));
                    let padding = pos.saturating_sub(chars.len()).min(self.cols as usize);
                    chars.extend(std::iter::repeat_n(' ', padding));
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
        self.parser.set_size(
            NonZeroU16::new(rows).expect("clamped rows nonzero"),
            NonZeroU16::new(cols).expect("clamped columns nonzero"),
        );
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
    #[test]
    fn window_grid_is_bounded() {
        assert_eq!(Screen::grid_for_window(0.0, 0.0), (5, 20));
        let (rows, cols) = Screen::grid_for_window(1030.0, 650.0);
        assert!(rows >= 18 && cols >= 100);
        assert_eq!(Screen::grid_for_window(99999.0, 99999.0), (200, 320));
    }
    #[test]
    fn scrollback_can_be_navigated_safely() {
        let mut screen = Screen::new();
        for i in 0..100 {
            screen.process(format!("line {i}\r\n").as_bytes());
        }
        screen.scroll_up(1000);
        assert!(screen.scrollback_offset() > 0);
        screen.scroll_down(1000);
        assert_eq!(screen.scrollback_offset(), 0);
    }
    #[test]
    fn parser_tracks_application_input_modes() {
        let mut screen = Screen::new();
        screen.process(b"\x1b[?1h\x1b[?2004h");
        assert!(screen.application_cursor());
        assert!(screen.bracketed_paste());
        screen.process(b"\x1b[?1l\x1b[?2004l");
        assert!(!screen.application_cursor());
        assert!(!screen.bracketed_paste());
    }
}
