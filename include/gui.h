#ifndef GUI_H
#define GUI_H

int gui_run(int *argc, char **argv[]);
int gui_destroy(void);

int gui_add(void *object);


void refresh_dump(void);

void gui_callback(void);

#endif
