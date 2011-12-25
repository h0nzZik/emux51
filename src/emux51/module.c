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

int get_id(void *space)
{
	int id;
	id=*(int *)space;
	if (id >= MODULE_CNT)
		return -1;
	return id;
}



int alloc_bits(void *space, int port, char mask)
{
	int id;
	id=get_id(space);
	if(id<0) {
		fprintf(stderr, "[emux51]\tBad module ID\n");
		return (-1);
	}
	
	/*	any bits are alloced	*/
	if (port_usage[port]&mask){
		return (-2);
	}

	port_usage[port]|=mask;
	modules[id].mask[port]|=mask;

	return 0;
}


int free_bits(void *space, int port, char mask)
{
	int id;

	id=get_id(space);
	if(id<0) {
		fprintf(stderr, "[emux51]\tBad module ID\n");
		return (-1);
	}

	mask&=modules[id].mask[port];
	/*	reset port bits		*/
	port_externals[port]|=mask;
	update_port(port);

	/*	free alloced bits	*/
	port_usage[port]&=~mask;
	modules[id].mask[port]&=~mask;
	return 0;
}


int write_port(void *space, int port, char data)
{
	char new;
	int id;

	id=get_id(space);
	if(id<0) {
		fprintf(stderr, "[emux51]\tBad module ID\n");
		return (-1);
	}

	new=port_externals[port];
	
	new|=data&modules[id].mask[port];
	new&=data|~modules[id].mask[port];


	port_externals[port]=new;
	update_port(port);
	return 0;
}

int read_port(void *space, int port, char *data)
{
	int id;
	id=get_id(space);
	if(id<0) {
		fprintf(stderr, "[emux51]\tBad module ID\n");
		return (-1);
	}

	*data=port_collectors[port];	
	return 0;
}


int add_to_queue(unsigned x, dlist_t **queue, void (*f)(void *, void *),
							int idx, void *data)
{
	dlist_t *new;
	dlist_t *entry;
	dlist_t *prev;

	/*	create node	*/
	new=g_malloc(sizeof(dlist_t));
	new->f=f;
	new->idx=idx;
	new->data=data;

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
	void *space;
	dlist_t *entry;
	dlist_t *tmp;

	entry=*queue;
	while (entry) {
		if (entry->dt < steps) {
			/*	perform the operation	*/
			if (entry->f){
				space=modules[entry->idx].space;
				entry->f(space, entry->data);
			}
			steps-=entry->dt;

			/*	free current node	*/
			tmp=entry->next;
			g_free(entry);
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
void rmid_queue(dlist_t **queue, int id)
{
	dlist_t *entry;
	dlist_t *prev;

	entry=*queue;
	prev=NULL;
	while (entry) {
		if(entry->idx == id) {
			if (prev) {
				prev->next=entry->next;
			} else {
				*queue=entry->next;
			}
			if (entry->next) {
				entry->next->dt+=entry->dt;
			}
			g_free(entry);
		}
		prev=entry;
		entry=entry->next;
	}
}

int cycle_queue_add(void *space, unsigned cycles,
			void (*f)(void *space, void *data), void *data)
{
	int rv;
	int id;

	id=get_id(space);
	if(id<0) {
		fprintf(stderr, "[emux51]\tBad module ID\n");
		return (-1);
	}

	rv=add_to_queue(cycles, &inst_queue, f, id, data);
	return rv;
}
int time_queue_add(void *space, unsigned ms,
			void (*f)(void *space, void *data), void *data)
{
	int rv;
	int cycles;
	int id;

	id=get_id(space);
	if(id<0) {
		fprintf(stderr, "[emux51]\tBad module ID\n");
		return (-1);
	}

	cycles=ms*SYNC_FREQ/1000;
	rv=add_to_queue(cycles, &time_queue, f, id, data);
	return rv;
}


void cycle_queue_perform(int steps)
{
	perform_queue(steps, &inst_queue);
}
void time_queue_perform(void)
{
	perform_queue(1, &time_queue);
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
		if (mod->handle && mod->info->port_changed){
			mod->info->port_changed(mod->space, port);
		}
	}
}


static int module_load(char *path, module_t *mod)
{
	module_info_t *info;
	void **p;

	memset(mod, 0, sizeof(module_t));
	mod->handle=load_lib(path);
	if (mod->handle == NULL) {
		fprintf(stderr, "[emux]\tcannot open file %s\n", path);
		return (-1);
	}
	info=load_sym(mod->handle, "module_info");

	if(info == NULL) {
		fprintf(stderr, "[emux]\t%s is bad module\n", path);
		close_lib(mod->handle);
		return -2;		
	}
	mod->info=info;

	p=load_sym(mod->handle, "read_port");
	if (p)
		*p=read_port;
	p=load_sym(mod->handle, "write_port");
	if (p)
		*p=write_port;
	p=load_sym(mod->handle, "alloc_bits");
	if (p)
		*p=alloc_bits;
	p=load_sym(mod->handle, "free_bits");
	if (p)
		*p=free_bits;
	p=load_sym(mod->handle, "time_queue_add");
	if (p)
		*p=time_queue_add;
	p=load_sym(mod->handle, "cycle_queue_add");
	if (p)
		*p=cycle_queue_add;
	p=load_sym(mod->handle, "gui_add");
	if (p)
		*p=gui_add;
	p=load_sym(mod->handle, "gui_remove");
	if (p)
		*p=gui_remove;

	printf("ok...\n");
	return 0;
}

int module_new(char *path)
{
	int i;
	module_t *mod;
	void *space;

	if (path == NULL) {
		return -1;
	}

	for(i=0; i<MODULE_CNT; i++) {
		if (modules[i].handle == NULL) {
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
	mod->id=i;

	space=g_malloc0(mod->info->space_size);
	mod->space=space;
	*(int *)space=mod->id;
	i=mod->info->init(space);
	if (i) {
		module_destroy(mod->space, "can't initialize\n");
		return -1;
	}
	return 0;
}

int module_destroy(void *space, const char *reason)
{
	module_t *mod;
	int i;

	mod=&modules[get_id(space)];
	if (mod == NULL) {
		printf("[emux]\ttrying to kill NULL\n");
		return -1;
	}

	rmid_queue(&time_queue, get_id(space));
	rmid_queue(&inst_queue, get_id(space));



	if (mod->info == NULL) {
		printf("wtf?\n");
	}
	if (mod->info->exit){
		mod->info->exit(mod->space, reason);
	}

/*		free alloced bits	*/
	for(i=0; i<4; i++) {
//		port_usage[i]|=mod->mask[i];
		free_bits(space, i,0xFF);
	}
	g_free(mod->space);
	close_lib(mod->handle);
	memset(mod, 0, sizeof(module_t));	

	printf("[emux]\tmodule killed.\n");
	return 0;
}


void module_destroy_all(const char *reason)
{
	int i;

	for (i=0; i<MODULE_CNT; i++) {
		if (modules[i].handle){
			printf("destroying %d\n", i);
			module_destroy(modules[i].space, reason);
		}
	}
}




/*	called from main while starting program	*/
int modules_init(void)
{
	memset(modules, 0, MODULE_CNT*sizeof(module_t));
	memset(port_usage, 0, sizeof(port_usage));
	return 0;
}


/*			</emulator API>				*/
