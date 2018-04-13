/*
 * Ecran: A program that (if properly completed) supports the multiplexing
 * of multiple virtual terminal sessions onto a single physical terminal.
 */

#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/signal.h>
#include <sys/select.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "ecran.h"

static void initialize();
static void curses_init(void);
static void curses_fini(void);
static void finalize(void);


int main(int argc, char *argv[]) {
    char option;
    for (int i=0; i<optind; i++) {
        if((option=getopt(argc, argv, "o:")) != -1) {
            switch(option) {
                //debug helper
                case 'o': {
                    char* file_name = optarg;
                    //allow read and write
                    //mode_t mode = S_IROTH | S_IWOTH;
                    mode_t mode = S_IRWXU;
                    int monitor_fdp = open(file_name, O_RDWR | O_CREAT | O_TRUNC, mode); //
                    // dup to stderr
                    dup2(monitor_fdp, 2); close(monitor_fdp);
                }
            }
        }
    }
    initialize();
    mainloop(); //keep loop waiting for user; write the user input tu foreground session
    // NOT REACHED
}

/*
 * Initialize the program and launch a single session to run the
 * default shell.
 */
static void initialize() {
    curses_init();
    char *path = getenv("SHELL");
    if(path == NULL)
	   path = "/bin/bash";
    char *argv[2] = { " (ecran session)", NULL };
    session_init(path, argv); //init the session
}

/*
 * Cleanly terminate the program.  All existing sessions should be killed,
 * all pty file descriptors should be closed, all memory should be deallocated,
 * and the original screen contents should be restored before terminating
 * normally.  Note that the current implementation only handles restoring
 * the original screen contents and terminating normally; the rest is left
 * to be done.
 */
static void finalize(void) {
    // REST TO BE FILLED IN
    fg_session = NULL;
    for (int i; i<MAX_SESSIONS; i++) {
        if (sessions[i] != NULL) {
            session_fini(sessions[i]);
            sessions[i] = NULL;
        }
    }
    curses_fini();
    exit(EXIT_SUCCESS);
}

/*
 * Helper method to initialize the screen for use with curses processing.
 * You can find documentation of the "ncurses" package at:
 * https://invisible-island.net/ncurses/man/ncurses.3x.html
 */
static void curses_init(void) {
    initscr();
    raw();                       // Don't generate signals, and make typein
                                 // immediately available.
    noecho();                    // Don't echo -- let the pty handle it.
    //main_screen = stdscr;
    main_screen = newwin(LINES-1, COLS, 0, 0);

    scrollok(main_screen, TRUE); // allow scrolling
    //keypad(main_screen, TRUE); // capture special keystrokes

    nodelay(main_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(main_screen);         // Clear the screen.
    refresh();                   // Make changes visible.

    //Arrange for the bottom line of the terminal window to be reserved as a status line
    status_line = newwin(1, COLS, LINES-1, 0);
    //char *test = "Hello world";
    //set_status(test);
}

/*
 * Helper method to finalize curses processing and restore the original
 * screen contents.
 */
void curses_fini(void) {
    endwin();
}

/*
 * Function to read and process a command from the terminal.
 * This function is called from mainloop(), which arranges for non-blocking
 * I/O to be disabled before calling and restored upon return.
 * So within this function terminal input can be collected one character
 * at a time by calling getch(), which will block until input is available.
 * Note that while blocked in getch(), no updates to virtual screens can
 * occur.
 */
void do_command() {
    int c = wgetch(main_screen);
    SESSION *session_specified; //specified session in the command
    int session_id; //specifeid session id in the command
    // Quit command: terminates the program cleanly
    //if(wgetch(main_screen) == 'q')
    if (c == 'q') {
	   finalize();
    }
    //create a new pty session
    else if(c == 'n') {
        //set_status("Try to create new session");
        char *path = getenv("SHELL");
        if(path == NULL)
           path = "/bin/bash";
        //char *argv[2] = { " (ecran session)", NULL };
        char *argv[2] = { " (new session)", NULL };
        //session_init has already set the new session to the foreground
        SESSION *new_session = session_init(path, argv);
        //check if a new session is created
        if (new_session != NULL) {
            //display the new virtual screen
            vscreen_show(new_session->vscreen);
            fprintf(stderr, "%s\n", "new session created");
            fprintf(stderr, "The session process id is: %d\n", new_session->pid);
        }else {
            //no more session can be created
            set_status("Session table is full");
            flash();
        }
    }
    //switch between sessions
    else if(c >= '0' && c <= '9') {
        //set_status("Try to swith session to");
        session_id = c - '0';
        session_specified = sessions[session_id];
        if (session_specified != NULL) {
            //set_status("Session not NULL");
            session_setfg(session_specified);
            //vscreen_show(session_specified->vscreen);
            vscreen_show(fg_session->vscreen);
            fprintf(stderr, "%s\n", "session switched");
            //VSCREEN *fg_vscreen = fg_session->vscreen;
            //set_status("Try to swith session to");
        }else {
            set_status("No such session");
            flash();
        }
    }
    // kill session
    else if(c == 'k') {
        session_id = wgetch(main_screen);
        if(session_id >= '0' && session_id <= '9') {
            session_id = session_id - '0';
            session_specified = sessions[session_id];
            if (session_specified != NULL) {
                /*
                check if it is the foreground session
                if foreground session was killed
                then put the session with the minimus sid to foreground
                if no other sessions then quit the program
                */
                int fg_sid = fg_session->sid;
                int specifeid_sid = session_specified->sid;
                int bg_sid;
                SESSION *bg_session;
                pid_t pid_specified = session_specified->pid;
                if (fg_sid == specifeid_sid) {
                    fprintf(stderr, "Try to kill a foreground session with process id: %d\n", pid_specified);
                    //if no background session then quite the program
                    if ((bg_sid = find_bg_session(fg_sid)) != -1) {
                        bg_session = sessions[bg_sid];
                        session_setfg(bg_session);
                        vscreen_show(bg_session->vscreen);
                    }else {
                        //all sessions have been killed except the foreground session
                        //finish all the mess in the finalize
                        finalize();
                    }
                }
                session_kill(session_specified);
                /*
                fprintf(stderr, "Try to kill a session with process id: %d\n", pid_specified);
                kill(pid_specified, SIGKILL);
                //after kill should set session in list to null and free it
                //try to wrap a function to do it
                session_fini(session_specified);
                sessions[session_id] = NULL;
                */
            }else {
                set_status("No such session"); //session specified hasn't been created yet
                flash();
            }
        }else {
            set_status("No such session"); //session id not in 0-9
            flash();
        }
    }
    else {
        set_status("No such command");
        // OTHER COMMANDS TO BE IMPLEMENTED
        flash();
    }
}

/*
 * Function called from mainloop(), whose purpose is do any other processing
 * that has to be taken care of.  An example of such processing would be
 * to deal with sessions that have terminated.
 */
void do_other_processing() {
    // TO BE FILLED IN

}


//set the contents of the status line
void set_status(char *status) {
    wclear(status_line);          // Clear the screen.
    waddstr(status_line, status); // add a string to the status line
    wrefresh(status_line);        // Make changes visible by refresh
}

//should not use a wait; it will go infinitly loop
//set handler for SIGCHLD
void sigchld_handler(int sig) {
    while(waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}