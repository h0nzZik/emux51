#include <gtk/gtk.h>
#include <module.h>


/*	module 'pseudo-class', contains instance-dependent data.	*/
typedef struct {
	/*	necessary data	*/
	modid_t id;	/* module ID structure */
	emuf_t *f;	/* functions, provided by emulator */

	/*	optional data	*/
	int port_nuber;	/* selected port number */
	int counter;	/* counter of 'port_changed' calls	*/

} instance_t;

/*	global, between instances shared data	*/
const char *name="Template module";



/*
 * When the user selects a module, emulator tries to find these functions:
 * (if not found, then module won't be loaded.)
 */ 	void *module_init(modid_t module_id, emuf_t *functions);
	void  module_exit(instance_t *self, const char *reason);




/*
 * This function may allocate memory for 'instance_t',
 * save module ID and functions, provided by emulator
 */
void *module_init(modid_t module_id, emuf_t *functions)
{
	instance_t *self;	/* instance */

/*
 * Allocate memory for current instance. No check is required.
 * It also clears allocated memory.
 */	self=g_malloc0(sizeof(instance_t));
/*
 * Save module ID and emulator-provided functions
 */	self->id=module_id;
	self->f=functions;
/*
 * Store pointer to instance in emulator internal structures
 * by calling 'set_space'from provided functions.
 */	self->f->set_space(self->id, self);

	


}
