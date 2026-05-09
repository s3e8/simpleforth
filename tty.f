." tty.f" cr

: raw-cr  13 emit  10 emit ;

27    constant KEY_ESC
1000  constant KEY_UP
1001  constant KEY_DOWN
1002  constant KEY_RIGHT
1003  constant KEY_LEFT

\ variable counter

variable cursor-x 
variable cursor-y 

0 constant mode-blocking
1 constant mode-nonblocking

: new-page-buffer   ;
: new-block-buffer  ;
: new-line-buffer   ;




: read-key ( -- key )
    sys-read-key
    dup KEY_ESC <> if exit then
    drop
    sys-read-key
    dup [ char [ ] literal <> if drop KEY_ESC exit then
    drop
    sys-read-key
    dup [ char A ] literal = if drop KEY_UP    exit then
    dup [ char B ] literal = if drop KEY_DOWN  exit then
    dup [ char C ] literal = if drop KEY_RIGHT exit then
    dup [ char D ] literal = if drop KEY_LEFT  exit then
    drop KEY_ESC
;

: update-screen-size ( -- )
    tty-get-window-size
    ." cursor-y: " dup . raw-cr 
    cursor-y !
    ." cursor-x: " dup . raw-cr
    cursor-x !
;






: tty-loop
    tty-enable-raw-mode
    begin
        tty-clear-screen
        update-screen-size
        raw-cr
        read-key
        raw-cr
        dup .
        dup emit

        KEY_ESC =
    until
    tty-disable-raw-mode
;

tty-loop \ todo: rename tty-quit?

