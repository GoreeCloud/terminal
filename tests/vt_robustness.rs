//! Deterministic hostile-input resilience tests. These are regression tests,
//! not a security certification or a replacement for coverage-guided fuzzing.
use goreecloud_terminal::screen::Screen;

#[test]
fn arbitrary_bounded_pty_streams_are_safe_to_snapshot() {
    // Runs in-process and uses no external service, shell, network, or OS entropy.
    for seed in 1..=12_u64 {
        let mut screen = Screen::new();
        let mut state = seed;
        for block in 0..50 {
            let mut bytes = [0u8; 128];
            for byte in &mut bytes {
                state ^= state << 13;
                state ^= state >> 7;
                state ^= state << 17;
                *byte = state as u8;
            }
            if block % 7 == 0 {
                bytes[..14].copy_from_slice(b"\x1b[?1049h\x1b[31m");
            }
            screen.process(&bytes);
            let cells = screen.render_cells();
            let (rows, cols) = screen.dimensions();
            assert!(cells.len() <= usize::from(rows) * usize::from(cols));
            assert!(cells.iter().all(|c| c.row < rows && c.column < cols));
            assert!(cells.iter().all(|c| (1..=2).contains(&c.span)));
        }
    }
}

#[test]
fn clipboard_control_sequences_are_not_displayed_as_raw_ansi() {
    let mut screen = Screen::new();
    screen.process(b"before\x1b]52;c;YXR0ZW1wdA==\x07after");
    let text = screen.copy_visible_text();
    assert!(text.contains("before"));
    assert!(text.contains("after"));
    assert!(!text.contains("\x1b"));
    assert!(!text.contains("YXR0ZW1wdA=="));
}

#[test]
fn long_styled_repaint_does_not_exceed_grid_capacity() {
    let mut screen = Screen::new();
    for i in 0..5000 {
        screen.process(format!("\x1b[38;5;{}m{str}\r\n", i % 256, str = "彩色").as_bytes());
    }
    let (rows, cols) = screen.dimensions();
    assert!(screen.render_cells().len() <= usize::from(rows) * usize::from(cols));
}
