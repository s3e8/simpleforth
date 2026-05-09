
void die(const char* str);

void sys_write(const char* buf, int len);
int sys_read_key(void);

void tty_test(void);
void tty_enable_raw_mode(void);
void tty_disable_raw_mode(void);
void tty_clear_screen(void);
int  tty_read_key(void);
int tty_get_window_size(int *rows, int *cols);