use anyhow::{anyhow, Result};
use goreecloud_terminal::{keys, paste, screen::Screen, session};
use slint::{Color, ComponentHandle, ModelRc, SharedString, Timer, TimerMode, VecModel};
use std::{cell::RefCell, rc::Rc, sync::mpsc::TryRecvError, time::Duration};

slint::include_modules!();

const MAX_TABS: usize = 8;

struct Tab {
    number: usize,
    screen: Screen,
    session: session::Session,
    stopped: bool,
}

struct State {
    tabs: Vec<Tab>,
    active: usize,
    next_number: usize,
    clipboard: Option<arboard::Clipboard>,
    notice: String,
}

impl State {
    fn new() -> Result<Self> {
        let mut state = Self {
            tabs: Vec::new(),
            active: 0,
            next_number: 1,
            clipboard: None,
            notice: String::new(),
        };
        state.add_tab()?;
        Ok(state)
    }

    fn add_tab(&mut self) -> Result<()> {
        if self.tabs.len() >= MAX_TABS {
            return Err(anyhow!("Maximum of {MAX_TABS} tabs reached"));
        }
        let session = session::spawn()?;
        self.tabs.push(Tab {
            number: self.next_number,
            screen: Screen::new(),
            session,
            stopped: false,
        });
        self.next_number += 1;
        self.active = self.tabs.len() - 1;
        self.notice.clear();
        Ok(())
    }

    fn close_current(&mut self) {
        if self.tabs.is_empty() {
            return;
        }
        // Dropping Session closes the PTY and stops its shell child.
        self.tabs.remove(self.active);
        if self.tabs.is_empty() {
            if let Err(err) = self.add_tab() {
                self.notice = format!("Could not open replacement shell: {err}");
            }
        } else {
            self.active = self.active.min(self.tabs.len() - 1);
            self.notice.clear();
        }
    }

    fn active_tab(&self) -> Option<&Tab> {
        self.tabs.get(self.active)
    }

    fn active_tab_mut(&mut self) -> Option<&mut Tab> {
        self.tabs.get_mut(self.active)
    }

    fn active_clipboard(&mut self) -> Result<&mut arboard::Clipboard> {
        if self.clipboard.is_none() {
            self.clipboard = Some(arboard::Clipboard::new()?);
        }
        Ok(self.clipboard.as_mut().expect("clipboard initialized"))
    }

    fn copy_visible(&mut self) -> Result<()> {
        let text = self
            .active_tab()
            .ok_or_else(|| anyhow!("No active shell"))?
            .screen
            .copy_visible_text();
        if text.len() > 128 * 1024 {
            return Err(anyhow!("Visible output exceeds clipboard safety limit"));
        }
        self.active_clipboard()?.set_text(text)?;
        self.notice = "Visible screen copied to system clipboard".to_string();
        Ok(())
    }

    fn paste(&mut self) -> Result<()> {
        // Only read the clipboard in response to explicit user input.
        let text = self.active_clipboard()?.get_text()?;
        let bracketed = self
            .active_tab()
            .ok_or_else(|| anyhow!("No active shell"))?
            .screen
            .bracketed_paste();
        let input = paste::encode_paste(&text, bracketed)
            .map_err(|err| anyhow!("Paste blocked ({err:?}); no input was sent"))?;
        self.active_tab_mut()
            .ok_or_else(|| anyhow!("No active shell"))?
            .session
            .send(&input)?;
        self.notice = "Clipboard pasted into local shell".to_string();
        Ok(())
    }

    fn send_key(&mut self, text: &str, control: bool, alt: bool, shift: bool) -> Result<()> {
        let tab = self
            .active_tab_mut()
            .ok_or_else(|| anyhow!("No active shell"))?;
        if tab.stopped {
            return Err(anyhow!("Shell has exited; open a new tab"));
        }
        if let Some(bytes) =
            keys::encode_key_for_mode(text, control, alt, shift, tab.screen.application_cursor())
        {
            tab.screen.scroll_to_bottom();
            tab.session.send(&bytes)?;
        }
        Ok(())
    }

    fn handle_key(&mut self, text: &str, control: bool, alt: bool, shift: bool) -> bool {
        if control && shift && !alt {
            if text.eq_ignore_ascii_case("t") {
                if let Err(err) = self.add_tab() {
                    self.notice = err.to_string();
                }
                return true;
            }
            if text.eq_ignore_ascii_case("w") {
                self.close_current();
                return true;
            }
            if text.eq_ignore_ascii_case("c") {
                if let Err(err) = self.copy_visible() {
                    self.notice = format!("Copy unavailable: {err}");
                }
                return true;
            }
            if text.eq_ignore_ascii_case("v") {
                if let Err(err) = self.paste() {
                    self.notice = format!("Paste unavailable: {err}");
                }
                return true;
            }
            if text == "\u{f72c}" {
                if let Some(tab) = self.active_tab_mut() {
                    tab.screen.scroll_up(10);
                }
                return true;
            }
            if text == "\u{f72d}" {
                if let Some(tab) = self.active_tab_mut() {
                    tab.screen.scroll_down(10);
                }
                return true;
            }
        }
        if let Err(err) = self.send_key(text, control, alt, shift) {
            self.notice = format!("Input unavailable: {err}");
            return true;
        }
        false
    }

    fn poll(&mut self, rows: u16, cols: u16) -> bool {
        let mut refresh = false;
        for tab in &mut self.tabs {
            if tab.screen.dimensions() != (rows, cols) && !tab.stopped {
                match tab.session.resize(rows, cols) {
                    Ok(()) => {
                        tab.screen.resize(rows, cols);
                        refresh = true;
                    }
                    Err(err) => {
                        self.notice = format!("Shell resize failed: {err}");
                        refresh = true;
                    }
                }
            }
            if tab.stopped {
                continue;
            }
            // Work limits apply per tab so an output flood cannot starve the UI.
            for _ in 0..32 {
                match tab.session.output.try_recv() {
                    Ok(session::Output::Bytes(chunk)) => {
                        tab.screen.process(&chunk);
                        refresh = true;
                    }
                    Ok(session::Output::Closed) | Err(TryRecvError::Disconnected) => {
                        tab.stopped = true;
                        refresh = true;
                        break;
                    }
                    Err(TryRecvError::Empty) => break,
                }
            }
        }
        refresh
    }

    fn update_view(&self, app: &TerminalWindow) {
        let tabs: Vec<SharedString> = self
            .tabs
            .iter()
            .map(|tab| {
                if tab.stopped {
                    format!("Shell {} (ended)", tab.number).into()
                } else {
                    format!("Shell {}", tab.number).into()
                }
            })
            .collect();
        app.set_tabs(ModelRc::new(VecModel::from(tabs)));
        app.set_active_tab(self.active as i32);

        if let Some(tab) = self.active_tab() {
            app.set_cells(cells_model(tab.screen.render_cells()));
            let (rows, cols) = tab.screen.dimensions();
            let status = if !self.notice.is_empty() {
                self.notice.clone()
            } else if tab.stopped {
                "SHELL EXITED · Open a new tab or close this tab".to_string()
            } else {
                let offset = tab.screen.scrollback_offset();
                if offset > 0 {
                    format!("LOCAL · {cols} × {rows} · Scrollback {offset} lines")
                } else {
                    format!("LOCAL · {cols} × {rows} · Ctrl+Shift+T new · Ctrl+Shift+C copy")
                }
            };
            app.set_status(status.into());
        } else {
            app.set_cells(cells_model(Vec::new()));
            app.set_status(self.notice.clone().into());
        }
    }
}

fn cells_model(cells: Vec<goreecloud_terminal::render::RenderCell>) -> ModelRc<TerminalCell> {
    ModelRc::new(VecModel::from(
        cells
            .into_iter()
            .map(|cell| TerminalCell {
                text: cell.text.into(),
                foreground: Color::from_rgb_u8(
                    cell.foreground.r,
                    cell.foreground.g,
                    cell.foreground.b,
                ),
                background: Color::from_rgb_u8(
                    cell.background.r,
                    cell.background.g,
                    cell.background.b,
                ),
                bold: cell.bold,
                italic: cell.italic,
                underline: cell.underline,
                row: i32::from(cell.row),
                column: i32::from(cell.column),
                span: i32::from(cell.span),
            })
            .collect::<Vec<_>>(),
    ))
}

fn main() -> Result<()> {
    let app = TerminalWindow::new()?;
    let state = Rc::new(RefCell::new(State::new()?));
    state.borrow().update_view(&app);

    let key_state = Rc::clone(&state);
    let key_ui = app.as_weak();
    app.on_typed(move |text, control, alt, shift| {
        let changed = key_state
            .borrow_mut()
            .handle_key(text.as_str(), control, alt, shift);
        if changed {
            if let Some(ui) = key_ui.upgrade() {
                key_state.borrow().update_view(&ui);
            }
        }
    });

    let new_state = Rc::clone(&state);
    let new_ui = app.as_weak();
    app.on_new_tab(move || {
        {
            let mut s = new_state.borrow_mut();
            if let Err(err) = s.add_tab() {
                s.notice = err.to_string();
            }
        }
        if let Some(ui) = new_ui.upgrade() {
            new_state.borrow().update_view(&ui);
        }
    });

    let close_state = Rc::clone(&state);
    let close_ui = app.as_weak();
    app.on_close_tab(move || {
        close_state.borrow_mut().close_current();
        if let Some(ui) = close_ui.upgrade() {
            close_state.borrow().update_view(&ui);
        }
    });

    let select_state = Rc::clone(&state);
    let select_ui = app.as_weak();
    app.on_select_tab(move |index| {
        {
            let mut s = select_state.borrow_mut();
            if let Ok(index) = usize::try_from(index) {
                if index < s.tabs.len() {
                    s.active = index;
                    s.notice.clear();
                }
            }
        }
        if let Some(ui) = select_ui.upgrade() {
            select_state.borrow().update_view(&ui);
        }
    });

    let scroll_state = Rc::clone(&state);
    let scroll_ui = app.as_weak();
    app.on_scroll(move |amount| {
        if let Some(tab) = scroll_state.borrow_mut().active_tab_mut() {
            if amount < 0 {
                tab.screen.scroll_up(amount.unsigned_abs() as usize);
            } else {
                tab.screen.scroll_down(amount as usize);
            }
        }
        if let Some(ui) = scroll_ui.upgrade() {
            scroll_state.borrow().update_view(&ui);
        }
    });

    let copy_state = Rc::clone(&state);
    let copy_ui = app.as_weak();
    app.on_copy_visible(move || {
        {
            let mut s = copy_state.borrow_mut();
            if let Err(err) = s.copy_visible() {
                s.notice = format!("Copy unavailable: {err}");
            }
        }
        if let Some(ui) = copy_ui.upgrade() {
            copy_state.borrow().update_view(&ui);
        }
    });

    let paste_state = Rc::clone(&state);
    let paste_ui = app.as_weak();
    app.on_paste(move || {
        {
            let mut s = paste_state.borrow_mut();
            if let Err(err) = s.paste() {
                s.notice = format!("Paste unavailable: {err}");
            }
        }
        if let Some(ui) = paste_ui.upgrade() {
            paste_state.borrow().update_view(&ui);
        }
    });

    let tick_state = Rc::clone(&state);
    let weak = app.as_weak();
    let timer = Timer::default();
    timer.start(TimerMode::Repeated, Duration::from_millis(33), move || {
        if let Some(ui) = weak.upgrade() {
            let size = ui.window().size();
            let scale = ui.window().scale_factor().max(0.1);
            if size.width < 100 || size.height < 100 {
                return;
            }
            let (rows, cols) =
                Screen::grid_for_window(size.width as f32 / scale, size.height as f32 / scale);
            let changed = tick_state.borrow_mut().poll(rows, cols);
            if changed {
                tick_state.borrow().update_view(&ui);
            }
        }
    });

    app.run()?;
    // All sessions are killed when their State is dropped.
    drop(timer);
    drop(state);
    Ok(())
}
