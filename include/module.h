#ifndef MODULE_H
#define MODULE_H

#define MODULE_CNT 32

#include <emux51.h>


typedef struct {
/*	handle for dynamic linker	*/
	void *handle;
/*	now only idx to array		*/
	int id;
} modid_t;


/*
 *	Functions, provided by emulator for module.
 */

typedef struct {
	char (*read_port)   (modid_t id, int port);
	int  (*write_port)  (modid_t id, int port, char data);
	int  (*alloc_bits)  (modid_t id, int port, char mask);
	int  (*free_bits)   (modid_t id, int port, char mask);
	int  (*handle_event)
		(modid_t id, const char *event, void (*handle)());
	int  (*cycle_queue_add)
		    (modid_t id, unsigned cycles, void (*event)(void * space));
	int  (*time_queue_add)
		    (modid_t id, unsigned time, void (*event)(void * space));
	int  (*set_space)   (modid_t id, void *space);
	void  (*set_name)    (modid_t id, const char *name);
	void (*crash)	    (modid_t id, const char *reason);
	

} emuf_t;


/*
 *	Functions, provided by the module itself.
 *
 *	See that 'read' function has type 'void'.
 *	These calls are only notifications for module.
 *	Module itself perform own operation.
 *
 *	Note that if emulator call modf_t::read,
 *	module will perform 'write' operation.
 */

typedef struct {
	/*	only these functions are necessary	*/
	void *(*init) (modid_t id, emuf_t *callbacks);
	void (*exit) (void *space, const char *reason);	
	/*		*/
	void (*write)(void *space, int port);
	void (*read) (void *space, int port);
} modf_t;



typedef struct {
	modid_t id;
	char mask[PORTS_CNT];
	emuf_t callbacks;
	modf_t f;
	void *space;
	void *window;
	const char *name;

} module_t;

/*	delta list type	*/
typedef struct DLIST {
	void (*f)(void *space);
	unsigned dt;
	void *space;
	struct DLIST *next;
	
} dlist_t;


int module_new(char *path);
int module_destroy(module_t *mod, const char *reason);

void module_import_port(int port);
void module_export_port(int port);


void module_op_queue_perform(int steps);
void module_time_queue_perform(void);


int modules_init(void);
void module_destroy_all(const char *reason);


#endif
