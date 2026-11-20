//! Integration smoke exercising a real local PTY, not a simulated shell.
//! The command is fixed and has no external side effects.
#[cfg(target_os = "linux")]
#[test]
fn shell_round_trip_over_real_pty() {
    use goreecloud_terminal::session::{self, Output};
    use std::time::{Duration, Instant};

    let mut shell = session::spawn().expect("spawn current user's local shell");
    // The echoed input contains the format expression, never this rendered marker.
    shell.send(b"printf 'GC_PTY_%s_READY' 7429\r")
        .expect("send fixed printf over PTY");
    let marker = b"GC_PTY_7429_READY";
    let mut output = Vec::new();
    let deadline = Instant::now() + Duration::from_secs(15);

    while Instant::now() < deadline {
        match shell.output.recv_timeout(Duration::from_millis(500)) {
            Ok(Output::Bytes(bytes)) => {
                output.extend_from_slice(&bytes);
                if output.windows(marker.len()).any(|w| w == marker) {
                    return;
                }
                assert!(output.len() < 256 * 1024, "unexpected excessive shell output");
            }
            Ok(Output::Closed) => panic!("shell closed before receiving the known marker"),
            Err(std::sync::mpsc::RecvTimeoutError::Disconnected) => panic!("PTY reader disconnected"),
            Err(std::sync::mpsc::RecvTimeoutError::Timeout) => {}
        }
    }
    panic!("timed out waiting for the fixed shell PTY response");
}
