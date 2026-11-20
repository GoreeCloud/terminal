use anyhow::Result;
use goreecloud_terminal::{keys, screen::Screen, session};
use slint::{ComponentHandle, ModelRc, SharedString, Timer, TimerMode, VecModel};
use std::{cell::RefCell, io::Write, rc::Rc, sync::{Arc, Mutex}, time::Duration};

slint::include_modules!();

fn main() -> Result<()> {
    let app = TerminalWindow::new()?;
    let session = session::spawn()?;
    let writer = Arc::new(Mutex::new(session.writer));

    app.set_status("LOCAL · 80 × 24 · Shell session".into());
    let writer_for_keys = Arc::clone(&writer);
    app.on_typed(move |text, control, alt, shift| {
        if let Some(bytes) = keys::encode_key(text.as_str(), control, alt, shift) {
            if let Ok(mut sink) = writer_for_keys.lock() {
                let _ = sink.write_all(&bytes);
                let _ = sink.flush();
            }
        }
    });

    let parser = Rc::new(RefCell::new(Screen::new()));
    app.set_rows(rows_model(parser.borrow().visible_rows()));

    let weak = app.as_weak();
    let timer = Timer::default();
    let output = session.output;
    timer.start(TimerMode::Repeated, Duration::from_millis(33), move || {
        let mut changed = false;
        let mut closed = false;
        // Limit UI work per tick. PTY reader already has a bounded queue.
        let mut count = 0;
        while count < 64 {
            match output.try_recv() {
                Ok(session::Output::Bytes(bytes)) => {
                    parser.borrow_mut().process(&bytes);
                    changed = true;
                    count += 1;
                }
                Ok(session::Output::Closed) => { closed = true; break; }
                Err(_) => break,
            }
        }
        if let Some(app) = weak.upgrade() {
            if changed { app.set_rows(rows_model(parser.borrow().visible_rows())); }
            if closed { app.set_status("SHELL EXITED · Close this window".into()); }
        }
    });

    // The PTY master and its child stay owned by this function during app.run.
    let _pty_master = session.master;
    let _shell_process = session._child;
    app.run()?;
    Ok(())
}

fn rows_model(rows: Vec<String>) -> ModelRc<SharedString> {
    let strings = rows.into_iter().map(SharedString::from).collect::<Vec<_>>();
    ModelRc::new(VecModel::from(strings))
}
