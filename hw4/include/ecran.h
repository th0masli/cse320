#include <ncurses.h>

#include "session.h"
#include "vscreen.h"

/*
 * The control character used to escape program commands,
 * so that they are not simply transferred as input to the
 * foreground session.
 */
#define COMMAND_ESCAPE 0x1   // CTRL-A

//bottom window for status line
WINDOW *status_line;

int mainloop(void);
void do_command(void);
void do_other_processing(void);

void set_status(char *status);
