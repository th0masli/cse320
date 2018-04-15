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
#include <string.h>
#include <time.h>

#include "ecran.h"

static void initialize();
static void curses_init(void);
static void curses_fini(void);
static void finalize(void);

static char *init_cmd = " (ecran session)"; //initial command for starting the pty
volatile sig_atomic_t set_flag = 0; //alarm handler flag

int main(int argc, char *argv[]) {
    char *file_name;
    char *extra_cmd = NULL;
    //there is -o filename
    if (argc > 2 && strcmp(argv[1], "-o") == 0) {
        //do -o file
        file_name = argv[2];
        //allow read and write
        //mode_t mode = S_IROTH | S_IWOTH;
        mode_t mode = S_IRWXU;
        int monitor_fdp = open(file_name, O_RDWR | O_CREAT | O_TRUNC, mode);
        // dup to stderr
        dup2(monitor_fdp, 2); close(monitor_fdp);
        //concate extra commnad; starting from arg[3]
        if (argv[3]) {
            extra_cmd = argv[3];
            for (int i=4; i<argc; i++) {
                extra_cmd = concat_str_space(extra_cmd, argv[i]);
            }
        }
    }
    //no -o filename
    else {
        //concate extra commnad; starting from arg[1]
        if (argv[1]) {
            extra_cmd = argv[1];
            for (int j=2; j<argc; j++) {
                extra_cmd = concat_str_space(extra_cmd, argv[j]);
            }
        }
    }
    //check if the init command is NULL
    if (extra_cmd != NULL)
        init_cmd = extra_cmd;
    //fprintf(stderr, "The initial command is: %s\n", init_cmd);
    //fprintf(stderr, "The extra command is: %s\n", extra_cmd);
    initialize();
    //for the time display
    signal(SIGALRM, sigalrm_handler); // install alarm signal handler
    alarm(1); // alarm schedule is 1s
    mainloop(); //keep loop waiting for user; write the user input to foreground session
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
    //char *argv[2] = { " (ecran session)", NULL };
    char *argv[2] = { init_cmd, NULL };
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
    int i;
    fg_session = NULL;
    right_session = NULL;
    for (i=0; i<MAX_SESSIONS; i++) {
        if (sessions[i] != NULL) {
            session_fini(sessions[i]);
            sessions[i] = NULL;
        }
    }
    curses_fini();
    exit(EXIT_SUCCESS);
}

/*
//maybe need a finalize for the EXIT_FAILURE
void failure_fini(void) {
    int i;
    fg_session = NULL;
    for (i=0; i<MAX_SESSIONS; i++) {
        if (sessions[i] != NULL) {
            session_fini(sessions[i]);
            sessions[i] = NULL;
        }
    }
    curses_fini();
    exit(EXIT_FAILURE);
}
*/

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
    //main_screen = newwin(LINES-1, COLS/2, 0, 0);
    if (main_screen == NULL)
        exit(EXIT_FAILURE);

    scrollok(main_screen, TRUE); // allow scrolling
    //keypad(main_screen, TRUE); // capture special keystrokes

    nodelay(main_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(main_screen);         // Clear the screen.
    //refresh();                   // Make changes visible
    wrefresh(main_screen);

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
    //delete the windows
    /*
    delwin(main_screen);
    delwin(status_line);
    endwin();
    */
    //if (delwin(main_screen) == ERR || delwin(status_line) == ERR || endwin() == ERR)
      //  exit(EXIT_FAILURE);
    if (num_screen == 2 && right_screen != NULL)
        delwin(right_screen);
    if (delwin(main_screen) == ERR)
        exit(EXIT_FAILURE);
    else if(delwin(status_line) == ERR)
        exit(EXIT_FAILURE);
    else if(endwin() == ERR)
        exit(EXIT_FAILURE);
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
            }else {
                set_status("No such session"); //session specified hasn't been created yet
                flash();
            }
        }else {
            set_status("No such session"); //session id not in 0-9
            flash();
        }
    }
    //split screen
    else if(c == 's') {
        split_screen();
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
    //reap the zombies
    signal(SIGCHLD, sigchld_handler);
    //display time in the bottom right corner of status line
    set_session_num();
    display_time();
    if (num_screen == 2 && right_screen != NULL && right_session != NULL) {
        //need to syncronize the virtual screen corresponding to the right screen
        vscreen_show_right(right_session->vscreen);
    }
    wrefresh(main_screen);
}


//set the contents of the status line
void set_status(char *status) {
    wclear(status_line);          // Clear the screen.
    waddstr(status_line, status); // add a string to the status line
    wrefresh(status_line);        // Make changes visible by refresh
}

//specify the flag WNOHANG
//set handler for SIGCHLD
void sigchld_handler(int sig) {
    while(waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}

//string contatenation with space between the strings
char *concat_str_space(char *str0, char *str1) {
    int i, j;

    i = 0;
    while (str0[i] != '\0') {
      i++;
    }
    //change the \0 to space
    str0[i] = ' ';
    i++;

    j = 0;
    while (str1[j] != '\0') {
      str0[i] = str1[j];
      i++;
      j++;
    }

    str0[i] = '\0';

    return str0;
}


//EXTRA CREDIT
//ANSI_EMULATION
void ansi_emulator() {

}


//SPLIT_SCREEN
void split_screen() {
    //flash();
    //split 1 main screen to 2
    if (num_screen == 1) {
        //delete the main screen first
        delwin(main_screen);
        //create the 2 sides terminal screen; and set left as main screen
        main_screen = newwin(LINES-1, COLS/2, 0, 0);
        right_screen = newwin(LINES-1, COLS/2, 0, COLS/2);
        //set left screen as main screen
        //main_screen = left_screen;
        if (main_screen == NULL || right_screen == NULL)
            exit(EXIT_FAILURE);

        scrollok(main_screen, TRUE); scrollok(right_screen, TRUE); // allow scrolling
        //keypad(main_screen, TRUE); // capture special keystrokes
        nodelay(main_screen, TRUE); nodelay(right_screen, TRUE);   // set non-blocking I/O on input.
        wclear(main_screen); wclear(right_screen);                 // clear the screen.
        wrefresh(main_screen); wrefresh(right_screen);
        //set global var num_screen to 2
        num_screen = 2;
        //resize the virtual screens
        resize_vscreens();
        //show the fg session on the left and right screen
        vscreen_show(fg_session->vscreen);
        SESSION *cur_session = sessions[fg_session->sid];
        session_set_right(cur_session);
        vscreen_show_right(right_session->vscreen);
    }
    //combine 2 splitted screen to 1 main screen
    else if (num_screen == 2) {
        //delete the 2 screens
        delwin(main_screen); delwin(right_screen);
        right_session = NULL;
        main_screen = newwin(LINES-1, COLS, 0, 0);
        if (main_screen == NULL)
            exit(EXIT_FAILURE);
        scrollok(main_screen, TRUE); // allow scrolling
        //keypad(main_screen, TRUE); // capture special keystrokes
        nodelay(main_screen, TRUE);  // Set non-blocking I/O on input.
        wclear(main_screen);         // Clear the screen.
        wrefresh(main_screen);       // Make changes visible
        //set global var num_screen to 1
        num_screen = 1;
        //resize the vscreens
        resize_vscreens();
        vscreen_show(fg_session->vscreen);
    }
}


//resize all the vscreen when type split
void resize_vscreens() {
    SESSION *cur_session;
    VSCREEN *cur_vscreen;
    for (int i=0; i<MAX_SESSIONS; i++) {
        cur_session = sessions[i];
        if (cur_session != NULL) {
            cur_vscreen = cur_session->vscreen;
            vscreen_resize(cur_vscreen);
        }
    }
}


//ENHANCED_STATUS
//display time
//alarm signal handler
void sigalrm_handler(int sig) {
    set_flag = 1;
}

//display time and update every second
void display_time() {
    time_t timer;
    char uniform_tm[10]; //HH:MM:SS
    struct tm *tm_info;
    if (set_flag) {
        time(&timer);
        tm_info = localtime(&timer);
        strftime(uniform_tm, 10, "%H:%M:%S", tm_info);
        //fprintf(stderr, "The current time is: %s\n", uniform_tm);
        mvwaddstr(status_line, 0, COLS-21, uniform_tm);
        wrefresh(status_line);        // Make changes visible by refresh
        set_flag = 0;
        alarm(1);
    }
}

//number of existing sessions
void set_session_num() {
    int session_num = 0;
    char session_num_str[6];
    //set the default text
    char *title = "#sessions:";
    for (int j=0; j<strlen(title); j++)
        mvwaddch(status_line, 0, COLS-12+j, title[j]);
    //get the current existing number of sessions
    for (int i=0; i<MAX_SESSIONS; i++) {
        if (sessions[i] != NULL)
            session_num++;
    }
    sprintf(session_num_str, "%d", session_num);
    //set_status(session_num_str);
    /*
    int num_len = strlen(session_num_str);
    for (int i=0; i<num_len; i++)
        mvwaddch(status_line, 0, COLS-2+i, session_num_str[i]);
    */
    mvwaddstr(status_line, 0, COLS-2, session_num_str);
    wrefresh(status_line);        // Make changes visible by refresh
}

//Help Screen