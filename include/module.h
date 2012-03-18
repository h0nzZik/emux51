#ifndef MODULE_H
#define MODULE_H

#define MODULE_CNT 32
#include <emux51.h>
#include <gmodule.h>

#define M_INIT(x)		((int (*)(void *))x)
#define M_EXIT(x)		((int(*)(void *, const char *))x)
#define M_PORT_CHANGED(x)	((int(*)(void *, int))x)
#define M_QUEUE(x)		((void (*)(void *, void *))x)
#define M_SPACE_SIZE(x)		((size_t)x)


typedef struct {
	char *name;
	size_t space_size;
	int (*init)(void *self);
	int (*exit)(void *self, const char *reason);
	int (*port_changed)(void *self, int port);
} module_info_t;


typedef struct {

	GModule *module;
	int id;
	char mask[PORTS_CNT];
	void *space;
	//module_info_t *info;
	module_info_t info;
} module_t;



#ifdef BUILDING_MODULE
int (*read_port)(void *space, int port, char *data)=NULL;
int (*write_port)(void *space, int port, char data)=NULL;

int (*alloc_bits)(void *space, int port, char mask)=NULL;
int (*free_bits)(void *space, int port, char mask)=NULL;

/*
int (*time_queue_add)(void *space, unsigned ms,
			void (*f)(void *space, void *data), void *data)=NULL;
*/

int *clock_counter=NULL;

void * (*timer_event_alloc)(void *space, void (*f)(void *space, void *data), void *data)=NULL;

void (*sync_timer_add)(void *event, unsigned ms)=NULL;

int (*usec_timer_add)(void *event, unsigned us)=NULL;
void (*sync_timer_unlink)(void *entry)=NULL;
void (*usec_timer_unlink)(void *entry)=NULL;

void * (*gui_add)(void *object, void *delete_data, const char *title)=NULL;
void (*gui_remove)(void *window);

int module_new(char *path);
int module_destroy(void *space, const char *reason);

void module_export_port(int port);

void cycle_queue_perform(int steps);
void time_queue_perform(void);


int modules_init(void);
void module_destroy_all(const char *reason);
#endif

#endif
