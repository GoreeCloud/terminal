//! Translate logical Slint key events to bytes for the shell's PTY.
//! Does not interpret or execute shell commands.
pub fn encode_key(text: &str, control: bool, alt: bool, _shift: bool) -> Option<Vec<u8>> {
    encode_key_for_mode(text, control, alt, _shift, false)
}
pub fn encode_key_for_mode(
    text: &str,
    control: bool,
    alt: bool,
    _shift: bool,
    app_cursor: bool,
) -> Option<Vec<u8>> {
    let mut input = match text {
        "\u{f700}" if app_cursor => b"\x1bOA".to_vec(),
        "\u{f701}" if app_cursor => b"\x1bOB".to_vec(),
        "\u{f702}" if app_cursor => b"\x1bOD".to_vec(),
        "\u{f703}" if app_cursor => b"\x1bOC".to_vec(),
        "\u{000a}" | "\u{000d}" => vec![b'\r'],
        "\u{0008}" => vec![0x7f],
        "\u{0009}" => vec![b'\t'],
        "\u{001b}" => vec![0x1b],
        "\u{f700}" => b"\x1b[A".to_vec(),   // Up
        "\u{f701}" => b"\x1b[B".to_vec(),   // Down
        "\u{f702}" => b"\x1b[D".to_vec(),   // Left
        "\u{f703}" => b"\x1b[C".to_vec(),   // Right
        "\u{007f}" => b"\x1b[3~".to_vec(),  // Delete
        "\u{f727}" => b"\x1b[2~".to_vec(),  // Insert
        "\u{f729}" => b"\x1b[H".to_vec(),   // Home
        "\u{f72b}" => b"\x1b[F".to_vec(),   // End
        "\u{f72c}" => b"\x1b[5~".to_vec(),  // PageUp
        "\u{f72d}" => b"\x1b[6~".to_vec(),  // PageDown
        "\u{0019}" => b"\x1b[Z".to_vec(),   // Shift+Tab
        "\u{f704}" => b"\x1bOP".to_vec(),   // F1
        "\u{f705}" => b"\x1bOQ".to_vec(),   // F2
        "\u{f706}" => b"\x1bOR".to_vec(),   // F3
        "\u{f707}" => b"\x1bOS".to_vec(),   // F4
        "\u{f708}" => b"\x1b[15~".to_vec(), // F5
        "\u{f709}" => b"\x1b[17~".to_vec(), // F6
        "\u{f70a}" => b"\x1b[18~".to_vec(), // F7
        "\u{f70b}" => b"\x1b[19~".to_vec(), // F8
        "\u{f70c}" => b"\x1b[20~".to_vec(), // F9
        "\u{f70d}" => b"\x1b[21~".to_vec(), // F10
        "\u{f70e}" => b"\x1b[23~".to_vec(), // F11
        "\u{f70f}" => b"\x1b[24~".to_vec(), // F12
        _ if control => {
            if text == " " || text == "@" {
                vec![0]
            } else if text == "[" {
                vec![0x1b]
            } else if text == "\\" {
                vec![0x1c]
            } else if text == "]" {
                vec![0x1d]
            } else if text == "^" {
                vec![0x1e]
            } else if text == "_" {
                vec![0x1f]
            } else if text.chars().count() == 1 && text.as_bytes()[0].is_ascii_alphabetic() {
                vec![text.as_bytes()[0].to_ascii_uppercase() - b'A' + 1]
            } else {
                return None;
            }
        }
        _ => {
            // Modifier-only keys, other private-use keys and control codes are not shell text.
            if text.is_empty()
                || text
                    .chars()
                    .any(|c| c.is_control() || ('\u{e000}'..='\u{f8ff}').contains(&c))
            {
                return None;
            }
            text.as_bytes().to_vec()
        }
    };
    if alt && input != [0x1b] {
        input.insert(0, 0x1b);
    }
    Some(input)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn text_and_unicode() {
        assert_eq!(
            encode_key("hello", false, false, false),
            Some(b"hello".to_vec())
        );
        assert_eq!(
            encode_key("é", false, false, false),
            Some("é".as_bytes().to_vec())
        );
    }
    #[test]
    fn control_shortcuts() {
        assert_eq!(encode_key("c", true, false, false), Some(vec![3]));
        assert_eq!(encode_key("C", true, false, true), Some(vec![3]));
        assert_eq!(encode_key(" ", true, false, false), Some(vec![0]));
        assert_eq!(encode_key("!", true, false, false), None);
    }
    #[test]
    fn application_cursor_mode() {
        assert_eq!(
            encode_key_for_mode("\u{f700}", false, false, false, true),
            Some(b"\x1bOA".to_vec())
        );
        assert_eq!(
            encode_key_for_mode("\u{f702}", false, false, false, true),
            Some(b"\x1bOD".to_vec())
        );
    }
    #[test]
    fn escape_and_cursor_keys() {
        assert_eq!(
            encode_key("\u{f700}", false, false, false),
            Some(b"\x1b[A".to_vec())
        );
        assert_eq!(
            encode_key("\u{f700}", false, true, false),
            Some(b"\x1b\x1b[A".to_vec())
        );
        assert_eq!(encode_key("\u{0008}", false, false, false), Some(vec![127]));
        assert_eq!(encode_key("\u{000a}", false, false, false), Some(vec![13]));
        assert_eq!(
            encode_key("\u{f702}", false, false, false),
            Some(b"\x1b[D".to_vec())
        );
        assert_eq!(
            encode_key("\u{f703}", false, false, false),
            Some(b"\x1b[C".to_vec())
        );
        assert_eq!(
            encode_key("\u{007f}", false, false, false),
            Some(b"\x1b[3~".to_vec())
        );
        assert_eq!(
            encode_key("\u{f727}", false, false, false),
            Some(b"\x1b[2~".to_vec())
        );
        assert_eq!(
            encode_key("\u{f729}", false, false, false),
            Some(b"\x1b[H".to_vec())
        );
        assert_eq!(
            encode_key("\u{f72b}", false, false, false),
            Some(b"\x1b[F".to_vec())
        );
        assert_eq!(
            encode_key("\u{f70f}", false, false, false),
            Some(b"\x1b[24~".to_vec())
        );
        assert_eq!(encode_key("\u{f777}", false, false, false), None);
    }
}
