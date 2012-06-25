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
#define M_RESET(x)		((int(*)(void *))x)


typedef struct {
	char *name;
	size_t space_size;
	int (*init)(void *self);
	int (*exit)(void *self, const char *reason);
	int (*port_changed)(void *self, int port);
	int (*reset)(void *self);
} module_info_t;


typedef struct {
	char *path_name;
	char *module_name;
} module_list_entry_t;

/*
 *	Base module structure.
 */
typedef struct {
	int id;
	void *module;
} module_base_t;




#ifdef BUILDING_MODULE
int (*read_port)(void *space, int port, char *data)=NULL;
int (*write_port)(void *space, int port, char data)=NULL;

int (*alloc_bits)(void *space, int port, char mask)=NULL;
int (*free_bits)(void *space, int port, char mask)=NULL;

int *clock_counter=NULL;

void * (*timer_event_alloc)(void *space, void (*f)(void *space, void *data), void *data)=NULL;

void (*sync_timer_add)(void *event, unsigned ms)=NULL;
void (*usec_timer_add)(void *event, unsigned us)=NULL;
void (*sync_timer_unlink)(void *entry)=NULL;
void (*usec_timer_unlink)(void *entry)=NULL;

void * (*gui_add)(void *object, void *delete_data, const char *title)=NULL;
void (*gui_remove)(void *window);


int (*watch_port)(void *self, unsigned int port);
int (*unwatch_port)(void *self, unsigned int port);

//#endif		//building module
#else

typedef struct {
	GModule *module;
	int id;
	char mask[PORTS_CNT];	
	void *space;
	module_info_t info;
} module_t;
int module_new(const char *name, const char *path);
int module_destroy(void *space, const char *reason);

int module_load_by_name(const char *module_name);

void module_export_port(int port);

void cycle_queue_perform(int steps);
void time_queue_perform(void);


int modules_init(void);
void module_destroy_all(const char *reason);
void module_reset_all(void);

int module_name_list_destroy(void);



#endif

#endif
