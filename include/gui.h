#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

#define FILE_LABEL_LEN 20

int gui_run(int *argc, char **argv[]);
int gui_destroy(void);

void * do_gui_add(void *object, void *module, const char *title);
void do_gui_remove(void *window);


void refresh_dump(void);

void gui_callback(void);


#endif
