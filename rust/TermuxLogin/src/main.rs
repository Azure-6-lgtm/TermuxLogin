use std::io; 
use std::fs;
use std::env;

use crossterm::{
    event::{self, Event, KeyCode},
    execute,
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
};

use ratatui::{
    backend::CrosstermBackend,
    layout::{Constraint, Direction, Layout},
    style::{Color, Style},
    widgets::{Block, Borders, Paragraph},
    Terminal,
};


fn main() -> Result<(), io::Error> {
    enable_raw_mode()?;
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen)?;

    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;

    let mut input = String::new();
    let mut cursor_pos = 0; 

    let home = env::var("HOME").expect("You are homeless"); 
    let path = format!("{}/.password.txt", home);
    let password = fs::read_to_string(path)
    .expect("Failed to read password file")
    .trim()
    .to_string();

    loop {
        terminal.draw(|f| {
            let size = f.size();

            // main layout
            let layout = Layout::default()
                .direction(Direction::Vertical)
                .constraints([
                    Constraint::Length(3),
                    Constraint::Min(0),
                    Constraint::Length(3),
                ])
                .split(size);

            // GREEN STYLE
            let green = Style::default().fg(Color::Green);

            // HEADER
            let header = Paragraph::new("Termux Login interface")
                .style(green)
                .block(
                    Block::default()
                        .borders(Borders::ALL)
                        .style(green)
                );
            f.render_widget(header, layout[0]);

            // FOOTER
            let footer = Paragraph::new(" Enter password • ESC to clear current input ")
                .style(green)
                .block(
                    Block::default()
                        .borders(Borders::ALL)
                        .style(green)
                );
            f.render_widget(footer, layout[2]);

            // center vertically
            let vertical = Layout::default()
                .direction(Direction::Vertical)
                .constraints([
                    Constraint::Percentage(40),
                    Constraint::Length(3),
                    Constraint::Percentage(40),
                ])
                .split(layout[1]);

            // center horizontally
            let horizontal = Layout::default()
                .direction(Direction::Horizontal)
                .constraints([
                    Constraint::Percentage(25),
                    Constraint::Percentage(50),
                    Constraint::Percentage(25),
                ])
                .split(vertical[1]);

            let center_area = horizontal[1];

            // mask password
            let masked = "*".repeat(input.len());

            let input_box = Paragraph::new(masked)
                .style(green)
                .block(
                    Block::default()
                        .title(" Enter Password ")
                        .borders(Borders::ALL)
                        .style(green)
                );

            f.render_widget(input_box, center_area);

            // cursor
            f.set_cursor(
                center_area.x + cursor_pos as u16 + 1,
                center_area.y + 1,
            );
        })?;

        // input handling
        if let Event::Key(key) = event::read()? {
            match key.code {
                KeyCode::Char(c) => {
                    input.insert(cursor_pos, c);
                    cursor_pos += 1;
                }
                KeyCode::Backspace => {
                    if cursor_pos > 0 {
                        input.remove(cursor_pos - 1);
                        cursor_pos -= 1;
                    }
                }
                KeyCode::Left => {
                    if cursor_pos > 0 {
                        cursor_pos -= 1;
                    }
                }
                KeyCode::Right => {
                    if cursor_pos < input.len() {
                        cursor_pos += 1;
                    }
                }
                KeyCode::Enter => {
                    if input == password {
                        break;
                    } else {
                        input.clear();
                        cursor_pos = 0;
                    }
                }
                KeyCode::Esc => {
                    input.clear();
                    cursor_pos = 0;
                },
                _ => {}
            }
        }
    }

    disable_raw_mode()?;
    execute!(terminal.backend_mut(), LeaveAlternateScreen)?;
    Ok(())
}
