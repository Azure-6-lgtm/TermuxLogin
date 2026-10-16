// Build: gcc login.c -o login -lncursesw
#define _XOPEN_SOURCE_EXTENDED 1
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void panel(int y, int x, int h, int w, const char *title,
                  const char *text, int cursor) {
    WINDOW *win = newwin(h, w, y, x);
    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);
    if (title) mvwprintw(win, 0, 1, "%s", title);
    mvwprintw(win, 1, 1, "%.*s", w - 2 > 0 ? w - 2 : 0, text);
    if (cursor >= 0) {
        if (cursor > w - 3) cursor = w - 3;
        wmove(win, 1, 1 + cursor);
    }
    wrefresh(win);
    delwin(win);
}

int main(void) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "You are homeless\n");
        return 1;
    }

    char path[1024];
    snprintf(path, sizeof path, "%s/.password.txt", home);

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to read password file\n");
        return 1;
    }
    char password[MAX_INPUT] = {0};
    if (!fgets(password, sizeof password, f)) password[0] = '\0';
    fclose(f);

    // trim whitespace on both ends
    size_t len = strlen(password);
    while (len > 0 && (password[len - 1] == '\n' || password[len - 1] == '\r' ||
                       password[len - 1] == ' ' || password[len - 1] == '\t'))
        password[--len] = '\0';
    char *start = password;
    while (*start == ' ' || *start == '\t') start++;
    if (start != password) memmove(password, start, strlen(start) + 1);

    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(25);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);

    char input[MAX_INPUT] = {0};
    int n = 0;          // input length
    int cursor_pos = 0;

    for (;;) {
        erase();
        refresh();

        int rows = LINES, cols = COLS;
        int mid_h = rows - 6;
        if (mid_h < 0) mid_h = 0;

        panel(0, 0, 3, cols, NULL, "Termux Login interface", -1);
        panel(rows - 3, 0, 3, cols, NULL,
              " Enter password \xE2\x80\xA2 ESC to clear current input ", -1);

        int in_y = 3 + mid_h * 40 / 100;
        int in_x = cols * 25 / 100;
        int in_w = cols * 50 / 100;

        char masked[MAX_INPUT];
        memset(masked, '*', n);
        masked[n] = '\0';
        panel(in_y, in_x, 3, in_w, " Enter Password ", masked, cursor_pos);

        int ch = getch();
        switch (ch) {
        case KEY_RESIZE:
            break;
        case KEY_BACKSPACE:
        case 127:
        case 8:
            if (cursor_pos > 0) {
                memmove(&input[cursor_pos - 1], &input[cursor_pos],
                        n - cursor_pos + 1);
                cursor_pos--;
                n--;
            }
            break;
        case KEY_LEFT:
            if (cursor_pos > 0) cursor_pos--;
            break;
        case KEY_RIGHT:
            if (cursor_pos < n) cursor_pos++;
            break;
        case '\n':
        case '\r':
        case KEY_ENTER:
            if (strcmp(input, password) == 0) goto done;
            memset(input, 0, sizeof input);
            n = cursor_pos = 0;
            break;
        case 27: // ESC
            memset(input, 0, sizeof input);
            n = cursor_pos = 0;
            break;
        default:
            if (ch >= 32 && ch < 127 && n < MAX_INPUT - 1) {
                memmove(&input[cursor_pos + 1], &input[cursor_pos],
                        n - cursor_pos + 1);
                input[cursor_pos++] = (char)ch;
                n++;
            }
        }
    }

done:
    endwin();
    return 0;
}
