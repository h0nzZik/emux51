#include <gtk/gtk.h>
#include <module.h>

/*	libemux_widgets provides some usefull widgets 	*/
#include <widgets/port_selector.h>

/*	module class	*/
typedef struct
{
	int id;			/* required: module ID. Set by emulator	*/
	GtkWidget *window;	/* our window	*/
} instance_t;


void module_exit(instance_t *self, char *reason)
{
	printf("[hello world:%d]\texiting: %s.\n",self->id, reason);

	/*	remove our window	*/
	gui_remove(self->window);
}

void module_read(instance_t *self, int port)
{
	char value=0;

	/*	read changed port, data will be stored in 'value'	*/
	read_port(self, port, &value); 

	printf("Hello, there is new value on port %d: %x\n", port, 0xFF&value);
}


void *module_init(instance_t *self)
{
	GtkWidget *vbox;
	GtkWidget *label;

	printf("hello world\n");

	/*	if you don't understand this, see GTK Tutorial	*/
	vbox=gtk_vbox_new(FALSE, 0);
	label=gtk_label_new("Hello, world!");

	gtk_container_set_border_width(GTK_CONTAINER(vbox), 2);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);

	/*	create window and add 'vbox' into it	*/
	self->window=gui_add(vbox, self, "First emux51 module");

	return 0;
}

/*	Module information structure	*/
module_info_t module_info={
	"hello world",				/* module name		*/
	M_SPACE_SIZE	(sizeof(instance_t)),	/* size of module class	*/
	M_INIT		(module_init),		/* init function	*/
	M_EXIT		(module_exit),		/* exit function	*/
	M_PORT_CHANGED	(module_read),		/* port change function	*/
};
