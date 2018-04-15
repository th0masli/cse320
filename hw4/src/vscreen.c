#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "vscreen.h"

/*
 * Functions to implement a virtual screen that can be multiplexed
 * onto a physical screen.  The current contents of a virtual screen
 * are maintained in an in-memory buffer, from which they can be
 * used to initialize the physical screen to correspond.
 */

WINDOW *main_screen;
//right screen for the split mode
WINDOW *right_screen = NULL;
//number of screens either 1 or 2
int num_screen = 1; //by default there is only 1 screen

struct vscreen {
    int num_lines;
    int num_cols;
    int cur_line;
    int cur_col;
    char **lines;
    char *line_changed;
};

static void update_line(VSCREEN *vscreen, int l);
static void update_line_right(VSCREEN *vscreen, int l); //update line for right screen

/*
 * Create a new virtual screen of the same size as the physical screen.
 */
VSCREEN *vscreen_init() {
    VSCREEN *vscreen = calloc(sizeof(VSCREEN), 1);
    //vscreen->num_lines = LINES;
    //reserve 1 line for the status line
    vscreen->num_lines = LINES-1;
    vscreen->num_cols = COLS;
    //halve the COLS when first type split
    //vscreen->num_cols = COLS/num_screen;
    vscreen->cur_line = 0;
    vscreen->cur_col = 0;
    vscreen->lines = calloc(sizeof(char *), vscreen->num_lines);
    vscreen->line_changed = calloc(sizeof(char), vscreen->num_lines);
    for(int i = 0; i < vscreen->num_lines; i++)
	   vscreen->lines[i] = calloc(sizeof(char), vscreen->num_cols);
    return vscreen;
}

/*
 * Erase the physical screen and show the current contents of a
 * specified virtual screen.
 */
void vscreen_show(VSCREEN *vscreen) {
    wclear(main_screen);
    //fprintf(stderr, "%s\n", *(vscreen->lines));
    for(int l = 0; l < vscreen->num_lines; l++) {
        //change every line when vscreen is called
        update_line(vscreen, l);
        vscreen->line_changed[l] = 0;
        /*
    	if(vscreen->line_changed[l]) {
    	    update_line(vscreen, l);
    	    vscreen->line_changed[l] = 0;
    	}
        */
    }
    //fprintf(stderr, "The current cols for a virtual screens is: %d\n", vscreen->num_cols);
    if (wmove(main_screen, vscreen->cur_line, vscreen->cur_col) == ERR)
        exit(EXIT_FAILURE);
    //if (refresh() == ERR)
    if (wrefresh(main_screen) == ERR)
        exit(EXIT_FAILURE);
}

/*
 * Function to be called after a series of state changes,
 * to cause changed lines on the physical screen to be refreshed
 * and the cursor position to be updated.
 * Although the same effect could be achieved by calling vscreen_show(),
 * the present function tries to be more economical about what is displayed,
 * by only rewriting the contents of lines that have changed.
 */
void vscreen_sync(VSCREEN *vscreen) {
    for(int l = 0; l < vscreen->num_lines; l++) {
    	if(vscreen->line_changed[l]) {
    	    update_line(vscreen, l);
    	    vscreen->line_changed[l] = 0;
    	}
    }
    if (wmove(main_screen, vscreen->cur_line, vscreen->cur_col) == ERR)
        exit(EXIT_FAILURE);
    //if (refresh() == ERR)
    if (wrefresh(main_screen) == ERR)
        exit(EXIT_FAILURE);
}

/*
 * Helper function to clear and rewrite a specified line of the screen.
 */
static void update_line(VSCREEN *vscreen, int l) {
    char *line = vscreen->lines[l];
    //fprintf(stderr, "%s\n", line);
    if (wmove(main_screen, l, 0) == ERR)
        exit(EXIT_FAILURE);
    wclrtoeol(main_screen);
    for(int c = 0; c < vscreen->num_cols; c++) {
    	char ch = line[c];
    	if(isprint(ch))
    	    waddch(main_screen, line[c]);
    }
    if (wmove(main_screen, vscreen->cur_line, vscreen->cur_col) == ERR)
        exit(EXIT_FAILURE);
    //if (refresh() == ERR)
    if (wrefresh(main_screen) == ERR)
        exit(EXIT_FAILURE);
}

/*
 * Output a character to a virtual screen, updating the cursor position
 * accordingly.  Changes are not reflected to the physical screen until
 * vscreen_show() or vscreen_sync() is called.  The current version of
 * this function emulates a "very dumb" terminal.  Each printing character
 * output is placed at the cursor position and the cursor position is
 * advanced by one column.  If the cursor advances beyond the last column,
 * it is reset to the first column.  The only non-printing characters
 * handled are carriage return, which causes the cursor to return to the
 * beginning of the current line, and line feed, which causes the cursor
 * to advance to the next line and clear from the current column position
 * to the end of the line.  There is currently no scrolling: if the cursor
 * advances beyond the last line, it wraps to the first line.
 */
void vscreen_putc(VSCREEN *vscreen, char ch) {
    int l = vscreen->cur_line;
    int c = vscreen->cur_col;
    if(isprint(ch)) {
	vscreen->lines[l][c] = ch;
	if(vscreen->cur_col + 1 < vscreen->num_cols)
	    vscreen->cur_col++;
    } else if(ch == '\n') {
        //do scrolling
        if ((vscreen->cur_line + 1) / vscreen->num_lines > 0) {
            if (scroll(main_screen) == ERR)
                exit(EXIT_FAILURE);
            //keep the line at the bottom
            l = vscreen->cur_line = l;
        } else {
            //go to the next line
            l = vscreen->cur_line = (vscreen->cur_line + 1) % vscreen->num_lines;
        }
        //clear the line
        memset(vscreen->lines[l], 0, vscreen->num_cols);
    } else if(ch == '\r') {
	   vscreen->cur_col = 0;
    }
    vscreen->line_changed[l] = 1;
}

/*
 * Deallocate a virtual screen that is no longer in use.
 */
void vscreen_fini(VSCREEN *vscreen) {
    // TO BE FILLED IN
    //free all the calloc
    for(int i = 0; i < vscreen->num_lines; i++)
        free(vscreen->lines[i]);
    free(vscreen->lines);
    free(vscreen->line_changed);
    free(vscreen);

}


//resize the specified vscreen when type split
void vscreen_resize(VSCREEN *vscreen) {
    //resize the vscreens to fit the 2 screen mode
    if (num_screen == 2) {
        vscreen->num_cols = COLS/num_screen;
    }
    //resize the vscreens to fit the original main screen
    else if (num_screen == 1) {
        vscreen->num_cols = COLS;
    }
}

//show virtual screen on a specific screen
void vscreen_show_right(VSCREEN *vscreen) {
    wclear(right_screen);
    for(int l = 0; l < vscreen->num_lines; l++) {
        //change every line when vscreen is called
        update_line_right(vscreen, l);
        vscreen->line_changed[l] = 0;
    }
    //fprintf(stderr, "The current cols for a virtual screens is: %d\n", vscreen->num_cols);
    if (wmove(right_screen, vscreen->cur_line, vscreen->cur_col) == ERR)
        exit(EXIT_FAILURE);
    //if (refresh() == ERR)
    if (wrefresh(right_screen) == ERR)
        exit(EXIT_FAILURE);
}

//update line for right screen
static void update_line_right(VSCREEN *vscreen, int l) {
    char *line = vscreen->lines[l];
    //fprintf(stderr, "%s\n", line);
    if (wmove(right_screen, l, 0) == ERR)
        exit(EXIT_FAILURE);
    wclrtoeol(right_screen);
    for(int c = 0; c < vscreen->num_cols; c++) {
        char ch = line[c];
        if(isprint(ch))
            waddch(right_screen, line[c]);
    }
    if (wmove(right_screen, vscreen->cur_line, vscreen->cur_col) == ERR)
        exit(EXIT_FAILURE);
    //if (refresh() == ERR)
    if (wrefresh(right_screen) == ERR)
        exit(EXIT_FAILURE);
}