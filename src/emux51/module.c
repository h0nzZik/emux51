/*
 * module.c - module support.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <gtk/gtk.h>

#include <emux51.h>
#include <module.h>
#include <gui.h>
#include <arch.h>


/*	delta list variables	*/
dlist_t *inst_queue=NULL;
dlist_t *time_queue=NULL;


char port_usage[PORTS_CNT];
module_t modules[MODULE_CNT];



static inline int test_module_id(modid_t modid)
{
	if (modid.id >= MODULE_CNT)
		return -1;
	if (modules[modid.id].id.id != modid.id)
		return -1;
	if (modules[modid.id].id.handle !=modid.handle)
		return -1;
	return 0;
}



static int module_load(char *path, module_t *mod)
{
	memset(mod, 0, sizeof(module_t));
	mod->id.handle=load_lib(path);
	if (mod->id.handle == NULL) {
		fprintf(stderr, "[emux]\tcannot open file %s:\n", path);
		return (-1);
	}
	mod->f.init=load_sym (mod->id.handle,	"module_init" );
	mod->f.exit=load_sym (mod->id.handle,	"module_exit" );

	if (!(mod->f.init && mod->f.exit)){
		fprintf(stderr, "[emux]\t%s is bad module\n", path);
		close_lib(mod->id.handle);
		return -2;
	}

	return 0;
}

/*******************************************************************************
 *		<API for modules>
 *
 *	API for modules is based on this functions:
 *	*	module_alloc_bits,
 *	*	module_free_bits,
 *	*	module_read_port
 *	*	module_write_port.
 *
 ******************************************************************************/




int module_alloc_bits(modid_t modid, int port, char mask)
{
	if (port >= PORTS_CNT || test_module_id(modid) || exporting) {
		return -1;
	}
	/*	any bits are alloced	*/
	if (port_usage[port]&mask){
		return (port_usage[port]&mask);
	}
	port_usage[port]|=mask;
	modules[modid.id].mask[port]|=mask;
	return 0;
}

int module_free_bits(modid_t modid, int port, char mask)
{
	if (port >= PORTS_CNT || test_module_id(modid) || exporting) {
		return -1;
	}
	mask&=modules[modid.id].mask[port];
	port_usage[port]&=~mask;
	modules[modid.id].mask[port]&=~mask;
	return 0;
}


int module_write_port(modid_t modid, int port, char data)
{
	char new;
	
	if (port >= PORTS_CNT || test_module_id(modid) || exporting) {
		fprintf(stderr, "somebody is cheating\n");
		return -1;
	}
	new=port_externals[port];
	
	new|=data&modules[modid.id].mask[port];
	new&=data|~modules[modid.id].mask[port];

	printf("writing 0x%2x to port\n", new);
//	write_port(port, new);
	port_externals[port]=new;
	update_port(port);
	return 0;
}
char module_read_port(modid_t modid, int port)
{
	if (port >= PORTS_CNT || test_module_id(modid)) {
		fprintf(stderr, "somebody is cheating\n");
		return -1;
	}

	return(port_collectors[port]);	
}

int module_set_space(modid_t modid, void *space)
{
	if(test_module_id(modid))
		return -1;

	modules[modid.id].space=space;
	return 0;
}


/* <queue> */

int add_to_queue(unsigned x, dlist_t **queue, void (*f)(void *space), void *space)
{
	dlist_t *new;
	dlist_t *entry;
	dlist_t *prev;

	/*	create node	*/
	new=malloc(sizeof(dlist_t));
	if (new == NULL) {
		fprintf(stderr, "[emux]\tOOM while queueing\n");
		return -2;
	}
	new->f=f;
	new->space=space;

	if (*queue == NULL) {
		new->next=NULL;
		new->dt=x;
		*queue=new;
		return 0;
	}
	
	entry=*queue;
	prev=NULL;
	while (entry) {
		if (entry->dt <= x) {
			x-=entry->dt;
			prev=entry;
			entry=entry->next;
		} else {
			new->next=entry;
			new->dt=x;
			entry->dt-=x;
			if (prev)
				prev->next=new;
			else {
				*queue=new;
			}
		}
	}
	prev->next=new;
	new->dt=x;
	new->next=NULL;
	return 0;
}

void perform_queue(int steps, dlist_t **queue)
{
	int i=0;

	dlist_t *entry;
	dlist_t *tmp;

	entry=*queue;
	while (entry) {
		i++;
		if (entry->dt < steps) {
			/*	perform the operation	*/
			if (entry->f)
				entry->f(entry->space);
			steps-=entry->dt;

			/*	free current node	*/
			tmp=entry->next;
			free(entry);
			/*	next node	*/
			entry=tmp;
			*queue=entry;
		}
		else {
			entry->dt-=steps;
			return;
		}
	}
	/*	nothing to do	*/
	return;
}


int module_op_queue_add(modid_t modid, unsigned cycles, void (*f)(void * space))
{
	int rv;
	if (test_module_id(modid)) {
		return -1;
	}
	rv=add_to_queue(cycles, &inst_queue, f, modules[modid.id].space);
	return rv;
}
int module_time_queue_add(modid_t modid, unsigned cycles, void (*f)(void * space))
{
	int rv;

	if (test_module_id(modid)) {
		return -1;
	}
	rv=add_to_queue(cycles, &time_queue, f, modules[modid.id].space);
	return rv;
}

void module_op_queue_perform(int steps)
{
	perform_queue(steps, &inst_queue);
}
void module_time_queue_perform(void)
{
	perform_queue(1, &time_queue);
}

/* </queue> */


/*	function for registering event handlers	*/
int handle_event(modid_t modid, const char *event, void (*handle)())
{
	module_t *mod;

	if (test_module_id(modid)) {
		return -1;
	}
	mod=&modules[modid.id];

	if (!strcmp(event, "read"))
		mod->f.write=handle;	/*	;)	*/
	else
		return -1;
	return 0;
}


int emulator_state(void)
{
	return running;
}

/*******************************************************************************
 *		</API for modules>
 ******************************************************************************/
 
void module_export_port(int port)
{
	int i;
	module_t *mod;

	for(i=0; i<MODULE_CNT; i++) {
		mod=&modules[i];
		if (mod->id.handle && mod->f.write){
			mod->f.write(mod->space, port);
		}
	}
}


/*	TODO:	*/
void module_crash(modid_t id, const char *reason)
{
	if(test_module_id(id)){
		printf("fuck\n");
		return;
	}
	fprintf(stderr, "module %d crashed: %s\n", id.id, reason);
	module_destroy(&modules[id.id], reason);
}

void module_set_name(modid_t id, const char *name)
{
	modules[id.id].name=name;
	if (modules[id.id].window)
		gui_set_window_title(modules[id.id].window, name);
}

int module_new(char *path)
{
	int i;
	module_t *mod;

	void *mod_rval;

	if (path == NULL) {
		return -1;
	}

	for(i=0; i<MODULE_CNT; i++) {
		if (modules[i].id.handle == NULL) {
			mod=&modules[i];
			break;
		}
	}
	if (i == MODULE_CNT) {
		fprintf(stderr, "[emux]\tcan not load module because of limit\n");
		return -1;
	}
	if (module_load(path, mod)){
		return -2;
	}

	mod->id.id=i;

	mod->callbacks.alloc_bits=module_alloc_bits;
	mod->callbacks.free_bits=module_free_bits;
	mod->callbacks.read_port=module_read_port;
	mod->callbacks.write_port=module_write_port;
	mod->callbacks.handle_event=handle_event;
	mod->callbacks.cycle_queue_add=module_op_queue_add;
	mod->callbacks.time_queue_add=module_time_queue_add;
	mod->callbacks.set_space=module_set_space;
	mod->callbacks.set_name=module_set_name;
	mod->callbacks.crash=module_crash;
	
	mod_rval=mod->f.init(mod->id, &mod->callbacks);
	if (mod_rval != NULL)
		mod->window=gui_add(mod_rval, mod);

	if (mod->window)
		gui_set_window_title(mod->window, mod->name);
	else
		gui_set_window_title(mod->window, "unnamed");
	return mod->id.id;
}



int module_destroy(module_t *mod, const char *reason)
{
	int i;
	if (mod == NULL) {
		printf("[emux]\ttrying to kill NULL\n");
		return -1;
	}
	printf("->exit\n");
	mod->f.exit(mod->space, reason);
	printf("->rm\n");
	if(mod->window)
		gui_remove(mod->window);
	printf("ok\n");
/*		CHECK: free alloced bits	*/
	for(i=0; i<4; i++) {
		port_usage[i]|=mod->mask[i];
	}

	close_lib(mod->id.handle);
	memset(mod, 0, sizeof(module_t));	

	printf("[emux]\tmodule killed\n");
	return 0;
}


void module_destroy_all(const char *reason)
{
	int i;

	for (i=0; i<MODULE_CNT; i++) {
		if (modules[i].id.handle)
			module_destroy(modules+i, reason);
	}
}




/*	called from main while starting program	*/
int modules_init(void)
{
	memset(modules, 0, MODULE_CNT*sizeof(module_t));
	return 0;
}


/*			</emulator API>				*/
