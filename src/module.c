#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>

#include <emux51.h>
#include <module.h>
#include <gui.h>
#include <arch.h>


/*	delta list variables	*/
dlist_t *dlist_first=NULL;
dlist_t *dlist_last=NULL;


char port_usage[PORTS_CNT];
module_t modules[MODULE_CNT];



inline int test_module_id(modid_t modid)
{
	if (modid.id >= MODULE_CNT)
		return -1;
	if (modules[modid.id].id.id != modid.id)
		return -1;
	if (modules[modid.id].id.handle !=modid.handle)
		return -1;
	return 0;
}



int module_load(char *path, module_t *mod)
{
	mod->id.handle=load_lib(path);
	if (mod->id.handle == NULL) {
		fprintf(stderr, "cannot load module from %s:\n", path);
		return (-1);
	}
	mod->f.init=load_sym (mod->id.handle,	"module_init" );
	mod->f.exit=load_sym (mod->id.handle,	"module_exit" );

	if (!(mod->f.init && mod->f.exit)){
		fprintf(stderr, "%s is bad module\n", path);
		close_lib(mod->id.handle);
		return -2;
	}

	memset(mod->mask, 0, PORTS_CNT);

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
	/*	somebody is cheating	*/
	if (port >= PORTS_CNT || test_module_id(modid) || exporting) {
/*		fprintf(stderr, "somebody is cheating\n");*/
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
/*		fprintf(stderr, "somebody is cheating\n");*/
		return -1;
	}
	mask&=modules[modid.id].mask[port];
	port_usage[port]&=~mask;
	modules[modid.id].mask[port]&=~mask;
	return 0;
}


int module_write_port(modid_t modid, int port, char data)
{
	char old;
	char new;
	
	if (port >= PORTS_CNT || test_module_id(modid) || exporting) {
/*		fprintf(stderr, "somebody is cheating\n");*/
		return -1;
	}
	old=read_port(port);
	new=old;
	new|=data& modules[modid.id].mask[port];
	new&=data|~modules[modid.id].mask[port];
	write_port(port, new);
	return 0;
}

char module_read_port(modid_t modid, int port)
{
	if (port >= PORTS_CNT || test_module_id(modid)) {
/*		fprintf(stderr, "somebody is cheating\n");*/
		return -1;
	}
	return(read_port(port));	
}

/*	I hope this is all right ;-)	*/
int module_queue_add(modid_t modid, unsigned cycles, void (*f)(void))
{
	dlist_t *entry;
	dlist_t *new;

	if (test_module_id(modid)) {
/*		fprintf(stderr, "somebody is cheating\n");*/
		return -1;
	}
	new=malloc(sizeof(dlist_t));
	if (new == NULL) {
		fprintf(stderr, "OOM while queueing\n");
		return -2;
	}
	new->f=f;

	entry=dlist_first;
	while (entry) {
		if (entry->dt <= cycles) {
			cycles-=entry->dt;
			entry=entry->next;
		}
		else {
			new->next=entry->next;
			entry->next=new;
			new->dt=cycles;
			if (new->next){
				new->next->dt-=new->dt;
			}
			else {
				dlist_last=new;
			}
			return 0;
		}
	}
	dlist_first=dlist_last=new;
	new->next=NULL;
	new->dt=cycles;

	return 0;
}

void module_queue_perform(int steps)
{
	dlist_t *entry;
	dlist_t *tmp;

	entry=dlist_first;
	int i=0;
	while (entry) {
		i++;
		if (entry->dt < steps) {
			/*	perform the operation	*/
			if (entry->f)
				entry->f();
/*			else fprintf(stderr, "empty f?, i == %d\n", i);*/
			steps-=entry->dt;

			/*	free current node	*/
			tmp=entry->next;
			free(entry);
			entry=tmp;
			dlist_first=entry;
		}
		else {
			entry->dt-=steps;
			return;
		}
	}
	/*	nothing to do	*/
	return;
}

/*	function for registering event handlers	*/
int handle_event(modid_t modid, const char *event, void (*handle)())
{
	module_t *mod;

	if (test_module_id(modid)) {
/*		fprintf(stderr, "somebody is cheating\n");*/
		return -1;
	}
	mod=&modules[modid.id];

	if (!strcmp(event, "read"))
		mod->f.write=handle;	/*	;)	*/
	else
	if (!strcmp(event, "write"))
		mod->f.read=handle;
	else
		return -1;
	return 0;
}

/*******************************************************************************
 *		</API for modules>
 ******************************************************************************/
 


/*			<emulator API>				*/

void module_import_port(int port)
{
	int i;
	module_t *mod;

	for (i=0; i<MODULE_CNT; i++) {
		mod=&modules[i];
		if(mod->mask[port] && mod->f.read)
			mod->f.read(port);
	}
}

void module_export_port(int port)
{
	int i;
	module_t *mod;

	for(i=0; i<MODULE_CNT; i++) {
		mod=&modules[i];
		if (mod->id.handle && mod->f.write){
			mod->f.write(port);
		}
	}
}

/*	called from commands	*/
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
		fprintf(stderr, "module_new failed: there's no space 4U\n");
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
	mod->callbacks.queue_add=module_queue_add;

	
	mod_rval=mod->f.init(mod->id, &mod->callbacks);

	if (mod_rval != NULL)
		gui_add(mod_rval);

	return mod->id.id;
}

int module_destroy(unsigned int id)
{
	if (id<0 || id>=MODULE_CNT) {
		return -1;
	}
	modules[id].f.exit("users choice");
	close_lib(modules[id].id.handle);
	return 0;
}

/*	called from main while starting program	*/
int modules_init(void)
{
	memset(modules, 0, MODULE_CNT*sizeof(module_t));
	return 0;

}

/*			</emulator API>				*/
