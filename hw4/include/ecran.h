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
//help screen mode
extern int help_mode;

int mainloop(void);
void do_command(void);
void do_other_processing(void);

void set_status(char *status);
void sigchld_handler(int sig);
char *concat_str_space(char *str0, char *str1);
void failure_fini(void);

//EXTRA CREDIT

//ANSI_EMULATION
void ansi_emulator();

//SPLIT_SCREEN
void split_screen();
//resize all the vscreen when type split
void resize_vscreens();

//ENHANCED_STATUS
void set_session_num();
void display_time();
//alarm signal handler
void sigalrm_handler(int sig);

//HELP_SCREEN
void display_help();
void active_sessions();


static char *help_msg[] = {"Terminal Multiplexer",
                    "by Yao Li",
                    "CTRL-A        COMMAND_ESCAPE",
                    "CTRL-A n      Create new virtual termin session",
                    "CTRL-A num    Switch between terminal sessions; num is the session number",
                    "CTRL-A k num  Kill terminal session with ID num",
                    "CTRL-A s      Split screen mode; type again to revert back",
                    "CTRL-A h      Display this help page",
                    "CTRL-A q      Exit the program",
                    "Active sessions: ",
                    "[ESC] exit this page"};