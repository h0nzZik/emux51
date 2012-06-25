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


#ifndef MODULE_REGEX
	#define MODULE_REGEX "^[a-zA-Z0-9].*(.so)$"
#endif


/*	delta list variables	*/
dlist_t *inst_queue=NULL;
dlist_t *time_queue=NULL;


char port_usage[PORTS_CNT];
GList *port_handlers[PORTS_CNT];
module_t modules[MODULE_CNT];

GList *module_name_list;
extern GtkListStore *module_name_list_store;
 
/*******************************************************************************
 *		<API for modules>
 *
 *	API for modules is based on these functions:
 *	*	alloc_bits(),
 *	*	free_bits(),
 *	*	watch_port(),
 *	*	unwatch_port(),
 *	*	read_port(),
 *	*	write_port().
 *
 ******************************************************************************/

int get_id(void *space)
{
	int id;
	id=*(int *)space;
	if (id >= MODULE_CNT){
		fprintf(stderr, "[emux51]\tbad module ID: %d\n", id);
		return -1;
	}
	return id;
}

module_t * get_module(void *space)
{
	int id;
	id=get_id(space);
	if (id < 0)
		return NULL;
	return (&modules[id]);
}

int do_alloc_bits(void *space, int port, char mask)
{
	module_t *module;

	module=get_module(space);
	if (port_usage[port]&mask || module == NULL)
		return -1;

	port_usage[port]|=mask;
	module->mask[port]|=mask;
	return 0;
}

int do_free_bits(void *space, int port, char mask)
{
	module_t *module;

	module=get_module(space);
	if(module == NULL)
		return -1;
	
	mask&=module->mask[port];
	/*	reset port bits		*/
	port_externals[port]|=mask;
	update_port(port);

	/*	free alloced bits	*/
	port_usage[port]&=~mask;
	module->mask[port]&=~mask;
	return 0;
}


int do_write_port(void *space, int port, char data)
{
	char new;
	module_t *module;

	module=get_module(space);
	if(module == NULL)
		return -1;

	new=port_externals[port];
	
	new|=data&module->mask[port];
	new&=data|~module->mask[port];


	port_externals[port]=new;
	update_port(port);
	return 0;
}

int do_read_port(void *space, int port, char *data)
{
	int id;
	id=get_id(space);
	if(id<0)
		return -1;

	*data=port_collectors[port];	
	return 0;
}

/*
 *	Adds calling module to list, asociated with given port. At the moment, in which the port data is changed, module's 'port_changed' handler is called.
 *	If module is already in list, nothing happens.
 */
int do_watch_port(void *self, unsigned int port)
{
	module_t *module;

	module=get_module(self);
	if (port >= PORTS_CNT || module == NULL){
		return(-1);
	}

	if (g_list_find(port_handlers[port], module) != NULL) {
		return 0;
	}

	port_handlers[port]=g_list_prepend(port_handlers[port], module);
	printf("watch: new start == %p\n", port_handlers[port]);
	return 0;
}

/*
 *	Remove calling module from given port's list.
 *	If module is not included in list, nothing happens.
 */
int do_unwatch_port(void *self, unsigned int port)
{
	module_t *module;

	module=get_module(self);
	if (port >= PORTS_CNT || module == NULL){
		return(-1);
	}

	while (g_list_find(port_handlers[port], module) != NULL)
	{
		port_handlers[port]=g_list_remove(port_handlers[port], module);
	}
	printf("unwatch: new start == %p\n", port_handlers[port]);
	return 0;
}

void * do_timer_event_alloc(void *space, void (*f)(void *space, void *data), void *data)
{
	if(get_id(space)<0)
		return NULL;

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
 
void module_export_port_to_module(module_t *module, gpointer ptr)
{
	int port;

	port=(int)ptr;
	if (module->module && module->info.port_changed)
		module->info.port_changed(module->space, port);
}


void module_export_port(int port)
{
	g_list_foreach(port_handlers[port], (GFunc)module_export_port_to_module, (gpointer)port);
}


module_t *module_alloc()
{
	int i;
	module_t *mod;

	for(i=0; i<MODULE_CNT; i++) {
		if (modules[i].module == NULL) {
			mod=&modules[i];
			break;
		}
	}
	if (i == MODULE_CNT) {
		fprintf(stderr, "[emux]\tmodule_alloc() failed\n");
		return NULL;
	}
	
	memset(mod, 0, sizeof(module_t));
	mod->id=i;


	return mod;
}

int module_set_symbols(module_t *mod)
{
	void **p;

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

	if (g_module_symbol(mod->module, "watch_port", (gpointer *)&p) == TRUE)
		*p=do_watch_port;

	if (g_module_symbol(mod->module, "unwatch_port", (gpointer *)&p) == TRUE)
		*p=do_unwatch_port;
	
}

#if 0
static int module_load(const char *path, module_t *mod)
{
	module_info_t *info;

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
#if 0
	printf("loaded module: \"%s\"\n"
		"\tspace size:\t%d\n"
		"\tinit function:\t%p\n"
		"\texit function:\t%p\n"
		"\tport change function:\t%p\n",
		info->name, info->space_size, info->init,
		info->exit, info->port_changed);	
#endif	
	memcpy(&(mod->info), info, sizeof(*info));
	module_set_symbols(mod);
	return 0;
}
#endif
static int module_load_module_from_file(const char *module_name, const char *path, module_t *mod)
{
//	module_info_t *info=NULL;
	int i;
	module_info_t *modules;

	mod->module=g_module_open(path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);

	if (mod->module == NULL) {
		fprintf(stderr, "[emux]\tcannot open file %s\n", path);
		return (-1);
	}

	if (g_module_symbol(mod->module, "modules",(gpointer *)&modules) == FALSE){
//		|| info == NULL || g_strcmp0(info->name, module_name) != 0) {
		fprintf(stderr, "[emux]\t%s is bad module\n", path);
		g_module_close(mod->module);
		return -2;		
	}
	for (i=0; modules[i].name != NULL; i++) {
		if (g_strcmp0(modules[i].name, module_name) == 0) {
			g_print("[emux51]\tLoading %s @ \"%s\"[%d]\n", module_name, path, i);
			memcpy(&(mod->info), &modules[i], sizeof(modules[i]));
			module_set_symbols(mod);
			return 0;
		}
	}
/*
	memcpy(&(mod->info), info, sizeof(*info));
	module_set_symbols(mod);
*/
	return 0;
	
}


int module_new(const char *name, const char *path)
{
	module_t *mod;
	void *space;

	if (path == NULL || name == NULL) {
		return -1;
	}
	mod=module_alloc();
	if (mod == NULL)
		return -2;
	if (module_load_module_from_file(name, path, mod)){
		return -3;
	}

	space=g_malloc0(mod->info.space_size);
	mod->space=space;
	/*	write base */
	module_base_t *base;

	base=(module_base_t *)space;
	base->id=mod->id;
//	base->module=mod;

	if (mod->info.init(space)) {
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

	mod=get_module(instance);

	printf("[emux51]\tkilling module %d @ %p\n", get_id(instance), mod);
	if (mod == NULL) {
		printf("[emux]\ttrying to kill NULL\n");
		return -1;
	}
	if (mod->info.exit) {
		mod->info.exit(mod->space, reason);
	}
	/*	unwatch all ports before possibly write	*/
	for (i=0; i<4; i++) {
		do_unwatch_port(instance, i);
	}

	/*	free alloced bits	*/
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


int modname_cmp(const module_list_entry_t *a, const char *b)
{
	return (g_strcmp0(a->module_name, b));
}


int module_load_by_name(const char *module_name)
{
	module_list_entry_t *entry;
	GList *found;

	found=g_list_find_custom(module_name_list,
				(gconstpointer)module_name,
				(GCompareFunc)modname_cmp);

	if (found == NULL) {
		fprintf(stderr, "[modules]\tmodule '%s' was not found\n", module_name);
		return -1;
	}
	entry=found->data;
	printf("%s -> %s\n", entry->module_name, entry->path_name);
	module_new(entry->module_name, entry->path_name);
	return 0;
}


int module_name_list_add(const char *path, const char *name)
{
	char *module_name;
	char *path_name;
	module_list_entry_t *entry;

	module_name=g_strdup(name);
	path_name=g_strdup(path);
	entry=g_malloc(sizeof(module_list_entry_t));

	entry->path_name=path_name;
	entry->module_name=module_name;

	module_name_list=g_list_prepend(module_name_list, entry);

	GtkTreeIter iter;
	gtk_list_store_append(module_name_list_store, &iter);
	gtk_list_store_set(module_name_list_store, &iter, 0, name, -1);
	return 0;
}

int module_name_list_destroy(void)
{
	gtk_list_store_clear(module_name_list_store);
	g_list_free(module_name_list);
	module_name_list=NULL;
	return 0;
}

int module_lookup_file(const char *pathname)
{
	GModule *module;
	char *module_name;
	int i;
	module_info_t *modules;

	module=g_module_open(pathname, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if (module == NULL) {
		return -1;
	}
	if (g_module_symbol(module, "modules", (gpointer)&modules) != TRUE) {
		return -1;
	}

	for(i=0; modules[i].name != NULL ; i++) {
		g_print("[modules]\tFound \"%s\"\n", modules[i].name);
		module_name_list_add(pathname, modules[i].name);
	}
	g_module_close(module);
	return 0;
}

int module_lookup_directory(const char *dirname)
{
//	GError *error;
	GDir *dir;
	const char *fname;
	GRegex * file_regex;
	char *pathname;

	dir=g_dir_open(dirname, 0, NULL);
	if (dir == NULL) {
		fprintf(stderr, "[modules]\tcan't open module directory %s\n", dirname);
		return -1;
	}
	file_regex=g_regex_new(MODULE_REGEX, 0, 0, NULL);
	if (file_regex == NULL) {
		fprintf(stderr, "[modules]\tbad module regular expression\n");
		return -1;
	}

	while ((fname=g_dir_read_name(dir)) != NULL) {
		if (g_regex_match(file_regex, fname, 0, NULL)) {
			pathname=g_build_path(G_DIR_SEPARATOR_S, dirname, fname, NULL);
			module_lookup_file(pathname);
			g_free(pathname);
		}
		
	}
	g_dir_close(dir);
	printf("[modules]\tloaded directory %s\n", dirname);
	return 0;
}


int module_do_lookup()
{

//	module_lookup_directory("out/nixies/modules");
	module_lookup_directory(module_directory);
	return 0;

}

int gui_init_module(void);
/*	called from main while starting program	*/
int modules_init(void)
{
	memset(modules, 0, MODULE_CNT*sizeof(module_t));
	memset(port_usage, 0, sizeof(port_usage));
	memset(port_handlers, 0, sizeof(port_handlers));
	return 0;
}



