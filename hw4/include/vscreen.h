#ifndef VSCREEN_H
#define VSCREEN_H

/*
 * Data structure maintaining information about a virtual screen.
 */

#include <ncurses.h>

typedef struct vscreen VSCREEN;

extern WINDOW *main_screen;
extern WINDOW *right_screen;  //right screen for the split mode
extern int num_screen; //number of screens either 1 or 2

VSCREEN *vscreen_init(void);
void vscreen_show(VSCREEN *vscreen);
void vscreen_sync(VSCREEN *vscreen);
void vscreen_putc(VSCREEN *vscreen, char c);
void vscreen_fini(VSCREEN *vscreen);

void vscreen_resize(VSCREEN *vscreen); //resize the specified vscreen when type split
void vscreen_show_right(VSCREEN *vscreen); //show virtual screen on a specific screen

#endif
