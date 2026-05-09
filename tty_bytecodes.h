BYTECODE(TTY_TEST, "tty-test", 0, 0, 0, {
    tty_test();
})

BYTECODE(SYS_WRITE, "sys-write", 2, 0, 0, {
    int len = (int)POP();
    const char* str = (char*)POP();

    sys_write(str, len);
})

BYTECODE(TTY_CLEAR_SCREEN, "tty-clear-screen", 0, 0, 0, {
    tty_clear_screen();
})

BYTECODE(SYS_READ_KEY, "sys-read-key", 0, 0, 0, {
    int key = sys_read_key();
    PUSH((cell)key);
})

BYTECODE(SYS_FLUSH, "tty-flush", 0, 0, 0, {
    fflush(stdout);
})


BYTECODE(TTY_ENABLE_RAW_MODE, "tty-enable-raw-mode", 0, 0, 0, {
    tty_enable_raw_mode();
})

BYTECODE(TTY_DISABLE_RAW_MODE, "tty-disable-raw-mode", 0, 0, 0, {
    tty_disable_raw_mode();
})

BYTECODE(TTY_READ_KEY, "tty-read-key", 0, 0, 0, {
    int key = tty_read_key();
    PUSH((cell)key);
})

BYTECODE(TTY_GET_WINDOW_SIZE, "tty-get-window-size", 0, 0, 0, {
    int rows;
    int cols;
    tty_get_window_size(&rows, &cols);
    PUSH((cell)rows);
    PUSH((cell)cols);
})