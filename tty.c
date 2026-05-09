#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "platform.h"
#include "tty.h"

#ifdef IS_WINDOWS
    // Windows console state
    #include <windows.h>

    typedef struct {
        // HANDLE hStdin;
        // HANDLE hStdout;
        // DWORD originalInputMode;
        // DWORD originalOutputMode;
        // int rawModeEnabled;
        HANDLE  hStdin;
        HANDLE  hStdout;
        DWORD   savedOutputMode;
        DWORD   savedInputMode;
        int     savedOutputValid;
        int     savedInputValid;
    } terminal_state_t;
#else
    // Unix terminal state
    typedef struct {
        int orig_termios_valid;
        struct termios orig_termios;
        // int rawModeEnabled;
    } terminal_state_t;
#endif

// ========== EDITOR KEY CODES ==========
#define CTRL_KEY(k) ((k) & 0x1f)
#define KEY_ESC     '\x1b'
#define KEY_UP      1000
#define KEY_DOWN    1001
#define KEY_RIGHT   1002
#define KEY_LEFT    1003

struct editor_config
{
    int screenrows;
    int screencols;
    terminal_state_t terminal_state;
};

struct editor_config TTY;
int counter = 0;

void tty_test(void) { printf("test!\r\n"); }

void die(const char* str) 
{
    tty_clear_screen();
    perror(str);
    exit(1);
}

/* sys_ is a prefix for lower level OS abstractions */
void sys_write(const char *buf, int len) 
{
    #ifdef _WIN32
        DWORD written;
        WriteConsoleA(TTY.terminal_state.hStdout, buf, len, &written, NULL);
    #else
        write(STDOUT_FILENO, buf, len);
    #endif
}

// Blocking read - returns raw key code
int sys_read_key(void) // read byte
{
    char c;
    
    #ifdef _WIN32
        DWORD numEvents;
        INPUT_RECORD record;
        
        while (1) {
            if (!PeekConsoleInput(TTY.terminal_state.hStdin, &record, 1, &numEvents)) {
                return -1;
            }
            
            if (numEvents == 0) {
                continue;  // No events, loop immediately
            }
            
            if (!ReadConsoleInput(TTY.terminal_state.hStdin, &record, 1, &numEvents)) {
                return -1;
            }
            
            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
                c = record.Event.KeyEvent.uChar.AsciiChar;
                if (c == 0) {
                    // Special key - return virtual key code
                    return record.Event.KeyEvent.wVirtualKeyCode;
                }
                return c;
            }
            // Ignore key up, mouse, window events
        }
    #else
        int nread;
        while (1) {
            nread = read(STDIN_FILENO, &c, 1);
            if (nread == 1) return c;
            if (nread == -1 && errno != EAGAIN) {
                perror("read");
                exit(1);
            }
            // EAGAIN: no key yet, loop immediately
        }
    #endif
}


void tty_disable_raw_mode(void) 
{
    #ifdef _WIN32
        printf("\x1b[0m");
        fflush(stdout);
        if (TTY.terminal_state.savedOutputValid) SetConsoleMode(TTY.terminal_state.hStdout, TTY.terminal_state.savedOutputMode);
        if (TTY.terminal_state.savedInputValid)  SetConsoleMode(TTY.terminal_state.hStdin,  TTY.terminal_state.savedInputMode);
    #else
        if (TTY.terminal_state.orig_termios_valid) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &TTY.terminal_state.orig_termios);
        }
    #endif
}

void tty_enable_raw_mode(void) 
{
    #ifdef _WIN32
        atexit(tty_disable_raw_mode);
        
        TTY.terminal_state.hStdin  = GetStdHandle(STD_INPUT_HANDLE);
        TTY.terminal_state.hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        if (TTY.terminal_state.hStdin == INVALID_HANDLE_VALUE || TTY.terminal_state.hStdout == INVALID_HANDLE_VALUE)
            die("GetStdHandle");
        
        // Save output mode
        if (!GetConsoleMode(TTY.terminal_state.hStdout, &TTY.terminal_state.savedOutputMode)) die("GetConsoleMode");
        TTY.terminal_state.savedOutputValid = 1;
        DWORD newMode = TTY.terminal_state.savedOutputMode;
        newMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        newMode &= ~ENABLE_WRAP_AT_EOL_OUTPUT;
        if (!SetConsoleMode(TTY.terminal_state.hStdout, newMode)) die("SetConsoleMode");
        
        // Save input mode
        if (!GetConsoleMode(TTY.terminal_state.hStdin, &TTY.terminal_state.savedInputMode)) die("GetConsoleMode");
        TTY.terminal_state.savedInputValid = 1;
        newMode = TTY.terminal_state.savedInputMode;
        newMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
        newMode &= ~(ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
        newMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        if (!SetConsoleMode(TTY.terminal_state.hStdin, newMode)) die("SetConsoleMode");
        
        // Flush any pending events
        FlushConsoleInputBuffer(TTY.terminal_state.hStdin);
        
    #else
        atexit(tty_disable_raw_mode);
        
        if (tcgetattr(STDIN_FILENO, &TTY.terminal_state.orig_termios) == -1) die("tcgetattr");
        TTY.terminal_state.orig_termios_valid = 1;
        
        struct termios raw = TTY.terminal_state.orig_termios;
        raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
    #endif
}

void tty_clear_screen(void)
{ 
    /*  
        todo: is there a more reliable way to do this? 
        adding another write shows that it kinda goes out of order 
    */
    sys_write("\x1b[2J]",   4);
    sys_write("\x1b[H",     3);
}

int tty_read_key(void)
{
    int c = sys_read_key();
    if (c != KEY_ESC) return c;
    
    // Check for escape sequence
    c = sys_read_key();
    if (c != '[') return KEY_ESC;
    
    c = sys_read_key();
    switch (c) {
        case 'A': return KEY_UP;
        case 'B': return KEY_DOWN;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        default: return KEY_ESC;
    }
}

int tty_get_window_size(int* rows, int* cols) 
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(TTY.terminal_state.hStdout, &csbi)) {
        *cols = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
        return 0;
    }
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col != 0) {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

    // fallback: move cursor to bottom-right, then query position
    sys_write("\x1b[999C\x1b[999B", 12);
    
    // query cursor position
    sys_write("\x1b[6n", 4);
    
    char buf[32];
    int i = 0;
    while (i < (int)sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    
    // response is \x1b[rows;colsR
    if (buf[0] != '\x1b' || buf[1] != '[') {
        *cols = 80; *rows = 24;
        return -1;
    }
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        *cols = 80; *rows = 24;
        return -1;
    }
#endif
    return 0;
}