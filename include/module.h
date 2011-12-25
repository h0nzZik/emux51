#ifndef MODULE_H
#define MODULE_H

#define MODULE_CNT 32
#include <emux51.h>
//#include <gui.h>

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
//	modid_t id;
	void *handle;
	int id;
	char mask[PORTS_CNT];
	void *space;
	module_info_t *info;
//	emuf_t callbacks;
//	modf_t f;

//	void *window;
//	const char *name;

} module_t;

/*	delta list type	*/
typedef struct DLIST {
	void (*f)(void *space, void *data);
	unsigned dt;
	int idx;
	void *data;
	struct DLIST *next;
	
} dlist_t;

#ifdef BUILDING_MODULE
int (*read_port)(void *space, int port, char *data)=NULL;
int (*write_port)(void *space, int port, char data)=NULL;

int (*alloc_bits)(void *space, int port, char mask)=NULL;
int (*free_bits)(void *space, int port, char mask)=NULL;

int (*time_queue_add)(void *space, unsigned ms,
			void (*f)(void *space, void *data), void *data)=NULL;
int (*cycle_queue_add)(void *space, unsigned cycles,
			void (*f)(void *space, void *data), void *data)=NULL;
void * (*gui_add)(void *object, void *delete_data, const char *title)=NULL;
void (*gui_remove)(void *window);
#else

int module_new(char *path);
int module_destroy(void *space, const char *reason);

void module_export_port(int port);

void cycle_queue_perform(int steps);
void time_queue_perform(void);


int modules_init(void);
void module_destroy_all(const char *reason);
#endif

#endif
