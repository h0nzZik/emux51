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
#include <lists.h>

#include <gmodule.h>

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



int do_alloc_bits(void *space, int port, char mask)
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


int do_free_bits(void *space, int port, char mask)
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


int do_write_port(void *space, int port, char data)
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

int do_read_port(void *space, int port, char *data)
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


void * do_timer_event_alloc(void *space, void (*f)(void *space, void *data), void *data)
{
	if(get_id(space)<0) {
		fprintf(stderr, "[emux51]\tBad module ID\n");
		return (NULL);
	}

	return(dlist_alloc(f, space, data));
	
}


void do_usec_timer_add(void *new, unsigned useconds)
{
	unsigned cycles;

	cycles=(double)useconds/1000000*Fosc/12;
	inst_queue=dlist_link(inst_queue, new, cycles);
}

void do_sync_timer_add(void *new, unsigned ms)
{
	unsigned cycles;

	cycles=ms*SYNC_FREQ/1000;
	time_queue=dlist_link(time_queue, new, cycles);
}


void cycle_queue_perform(int steps)
{
	dlist_perform(&inst_queue, steps);
}
void time_queue_perform(void)
{
	dlist_perform(&time_queue, 1);
}


void do_sync_timer_unlink(void *entry)
{
	dlist_unlink(&time_queue, entry);
}

void do_usec_timer_unlink(void *entry)
{
	dlist_unlink(&inst_queue, entry);
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

		if (mod->module && mod->info.port_changed) {
			mod->info.port_changed(mod->space, port);
		}
	}
}


static int module_load(char *path, module_t *mod)
{
	module_info_t *info;
	void **p;

	memset(mod, 0, sizeof(module_t));

	mod->module=g_module_open(path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);

	if (mod->module == NULL) {
		fprintf(stderr, "[emux]\tcannot open file %s\n", path);
		return (-1);
	}

	if (g_module_symbol(mod->module, "module_info",(gpointer *)&info) == FALSE
		|| info == NULL) {
		
		fprintf(stderr, "[emux]\t%s is bad module\n", path);

		g_module_close(mod->module);
		return -2;		
	}

	printf("loaded module: \"%s\"\n"
		"\tspace size:\t%d\n"
		"\tinit function:\t%p\n"
		"\texit function:\t%p\n"
		"\tport change function:\t%p\n",
		info->name, info->space_size, info->init,
		info->exit, info->port_changed);	
	
	memcpy(&(mod->info), info, sizeof(*info));

	if (g_module_symbol(mod->module,"read_port",(gpointer *)&p) == TRUE)
		*p=do_read_port;

	if (g_module_symbol(mod->module,"write_port",(gpointer *)&p) == TRUE)
		*p=do_write_port;

	if (g_module_symbol(mod->module,"alloc_bits",(gpointer *)&p) == TRUE)
		*p=do_alloc_bits;

	if (g_module_symbol(mod->module,"free_bits",(gpointer *)&p) == TRUE)
		*p=do_free_bits;

	if (g_module_symbol(mod->module,"timer_event_alloc",(gpointer *)&p) == TRUE)
		*p=do_timer_event_alloc;

	if (g_module_symbol(mod->module,"sync_timer_add",(gpointer *)&p) == TRUE)
		*p=do_sync_timer_add;

	if (g_module_symbol(mod->module,"usec_timer_add",(gpointer *)&p) == TRUE)
		*p=do_usec_timer_add;

	if (g_module_symbol(mod->module,"gui_add",(gpointer *)&p) == TRUE)
		*p=do_gui_add;

	if (g_module_symbol(mod->module,"gui_remove",(gpointer *)&p) == TRUE)
		*p=do_gui_remove;

	if (g_module_symbol(mod->module,"clock_counter",(gpointer *)&p) == TRUE)
		*p=&cycle_counter;

	if (g_module_symbol(mod->module,"sync_timer_unlink",(gpointer *)&p) == TRUE)
		*p=do_sync_timer_unlink;

	if (g_module_symbol(mod->module,"usec_timer_unlink",(gpointer *)&p) == TRUE)
		*p=do_usec_timer_unlink;

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
		if (modules[i].module == NULL) {
			mod=&modules[i];
			break;
		}
	}
	if (i == MODULE_CNT) {
		fprintf(stderr, "[emux]\tcan not load module because of limit\n");
		return -1;
	}
	printf("[emux51]\tnew module with id %d @ %p\n", i, mod);
	if (module_load(path, mod)){
		return -2;
	}
	mod->id=i;

	space=g_malloc0(mod->info.space_size);
	mod->space=space;
	*(int *)space=mod->id;

	i=mod->info.init(space);	
	if (i) {
		module_destroy(mod->space, "can't initialize\n");
		return -1;
	}
	return 0;
}

void module_dump_all(void)
{
#if 0
	int i;

	printf("[emux51]\tdumping all modules\n");
	for (i=0; i<MODULE_CNT; i++) {
		if (modules[i].space) {
			printf("module id %d @ %p\n", i, modules[i].space);
			printf("\t->info\t%p\n", modules[i].info);
			printf("\t\t->port_changed\t%p\n", modules[i].info->port_changed);


			printf("...\n");
		}
	}
#endif
}


int module_destroy(void *instance, const char *reason)
{
	module_t *mod;
	int i;

	mod=&modules[get_id(instance)];
	printf("[emux51]\tkilling module %d @ %p\n", get_id(instance), mod);
	if (mod == NULL) {
		printf("[emux]\ttrying to kill NULL\n");
		return -1;
	}
	if (mod->info.exit) {
		mod->info.exit(mod->space, reason);
	}
	mod->info.port_changed=NULL;



/*		free alloced bits	*/
	for(i=0; i<4; i++) {
		do_free_bits(instance, i,0xFF);
	}
	g_free(mod->space);
	g_module_close(mod->module);
	memset(mod, 0, sizeof(module_t));	
	printf("[emux]\tmodule killed.\n");
	module_dump_all();
	return 0;
}



void module_destroy_all(const char *reason)
{
	int i;

	for (i=0; i<MODULE_CNT; i++) {
		if (modules[i].module){
			module_destroy(modules[i].space, reason);
		}
	}
}

void module_reset_all(void)
{
	int i;

	for (i=0; i<MODULE_CNT; i++) {
		if (modules[i].module && modules[i].info.reset) {
			modules[i].info.reset(modules[i].space);
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



