//! Local shell startup with a real Linux PTY. No shell string evaluation.
use anyhow::{anyhow, Context, Result};
use portable_pty::{native_pty_system, CommandBuilder, MasterPty, PtySize};
use std::{
    io::{Read, Write},
    path::PathBuf,
    sync::mpsc::{sync_channel, Receiver},
    thread,
};

pub enum Output {
    Bytes(Vec<u8>),
    Closed,
}

pub struct Session {
    pub master: Box<dyn MasterPty + Send>,
    pub writer: Box<dyn Write + Send>,
    pub output: Receiver<Output>,
    // Keep the child alive until the UI closes.
    pub _child: Box<dyn portable_pty::Child + Send + Sync>,
}
impl Session {
    pub fn resize(&self, rows: u16, cols: u16) -> Result<()> {
        self.master.resize(PtySize { rows, cols, pixel_width: 0, pixel_height: 0 }).context("resize shell PTY")
    }
    pub fn send(&mut self, data: &[u8]) -> Result<()> {
        self.writer.write_all(data).context("write PTY")?;
        self.writer.flush().context("flush PTY")
    }
}
impl Drop for Session {
    fn drop(&mut self) {
        // A closed tab must not orphan its direct shell child.
        let _ = self._child.kill();
    }
}

pub fn default_shell() -> PathBuf {
    #[cfg(unix)]
    fn is_valid(p: &std::path::Path) -> bool {
        use std::os::unix::fs::PermissionsExt;
        p.is_absolute()
            && std::fs::metadata(p)
                .map(|m| m.is_file() && m.permissions().mode() & 0o111 != 0)
                .unwrap_or(false)
    }
    let requested = std::env::var_os("SHELL").map(PathBuf::from);
    requested
        .filter(|p| is_valid(p))
        .unwrap_or_else(|| PathBuf::from("/bin/sh"))
}

pub fn spawn() -> Result<Session> {
    let pair = native_pty_system()
        .openpty(PtySize {
            rows: crate::screen::ROWS,
            cols: crate::screen::COLS,
            pixel_width: 0,
            pixel_height: 0,
        })
        .context("create local Linux pseudoterminal")?;

    let shell = default_shell();
    if !shell.exists() {
        return Err(anyhow!("no usable shell is installed"));
    }
    let mut command = CommandBuilder::new(shell.as_os_str());
    command.env("TERM", "xterm-256color");
    // Omit COLORTERM=truecolor until the native cell renderer supports it.
    let child = pair
        .slave
        .spawn_command(command)
        .context("spawn shell in pseudoterminal")?;
    drop(pair.slave);

    let mut reader = pair
        .master
        .try_clone_reader()
        .context("clone PTY output reader")?;
    let writer = pair.master.take_writer().context("open PTY input writer")?;
    // Bounded queue protects the app from unlimited output buffering.
    let (sender, output) = sync_channel(128);
    thread::Builder::new()
        .name("goreecloud-pty-reader".into())
        .spawn(move || {
            let mut buf = [0u8; 8192];
            loop {
                match reader.read(&mut buf) {
                    Ok(0) | Err(_) => break,
                    Ok(n) => {
                        if sender.send(Output::Bytes(buf[..n].to_vec())).is_err() {
                            return;
                        }
                    }
                }
            }
            let _ = sender.send(Output::Closed);
        })
        .context("start PTY reader thread")?;
    Ok(Session {
        master: pair.master,
        writer,
        output,
        _child: child,
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn fallback_shell_is_absolute_and_executable() {
        assert!(default_shell().is_absolute());
        assert!(default_shell().exists());
    }
}
