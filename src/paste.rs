//! User-triggered paste. Reject control-sequence injection and accidental
//! multiline execution when the child shell has not enabled bracketed paste.
pub const MAX_PASTE_BYTES: usize = 32 * 1024;
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PasteError { TooLarge, ControlCharacter, MultilineWithoutBrackets }
pub fn encode_paste(text: &str, bracketed: bool) -> Result<Vec<u8>, PasteError> {
    if text.len() > MAX_PASTE_BYTES { return Err(PasteError::TooLarge); }
    if !bracketed && (text.contains('\n') || text.contains('\r')) {
        return Err(PasteError::MultilineWithoutBrackets);
    }
    if text.chars().any(|c| (c.is_control() && c != '\n' && c != '\r' && c != '\t') || c == '\u{7f}') {
        return Err(PasteError::ControlCharacter);
    }
    let normalized = text.replace("\r\n", "\n").replace('\r', "\n");
    let mut bytes = Vec::with_capacity(normalized.len() + 16);
    if bracketed { bytes.extend_from_slice(b"\x1b[200~"); }
    bytes.extend_from_slice(normalized.replace('\n', "\r").as_bytes());
    if bracketed { bytes.extend_from_slice(b"\x1b[201~"); }
    Ok(bytes)
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test] fn single_line() { assert_eq!(encode_paste("ls -la", false).unwrap(), b"ls -la"); }
    #[test] fn multiline_is_guarded() {
        assert_eq!(encode_paste("echo hi\nexit", false), Err(PasteError::MultilineWithoutBrackets));
        assert_eq!(encode_paste("one\r\ntwo", true).unwrap(), b"\x1b[200~one\rtwo\x1b[201~");
    }
    #[test] fn control_and_large_payload_rejected() {
        assert_eq!(encode_paste("hi\x1b[201~", true), Err(PasteError::ControlCharacter));
        assert_eq!(encode_paste("a\0b", false), Err(PasteError::ControlCharacter));
        assert_eq!(encode_paste(&"A".repeat(MAX_PASTE_BYTES + 1), false), Err(PasteError::TooLarge));
    }
}
