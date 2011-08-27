#include <gtk/gtk.h>
#include <module.h>

#include <widgets/7seg.h>
#include <widgets/port_selector.h>

typedef struct {
	int id;
	GtkWidget *window;
	GtkWidget *sseg;
	int port_no;
	int invert;
} instance;

void import_segments(instance *self)
{
	char segments;
	read_port(self, self->port_no, &segments);

	if (self->invert)
		segments=~segments;
	seven_seg_set_segments(self->sseg, segments);
	
}

void module_read(instance *self, int port)
{
	if (port != self->port_no)
		return;
	import_segments(self);
}


static void port_select(PortSelector *ps, instance *self)
{
	self->port_no=port_selector_get_port(ps);
	import_segments(self);
}


void *module_init(instance *self)
{
	/*	init module	*/
	GtkWidget *select;
	GtkWidget *vbox;
	GtkWidget *ibox;

	self->invert=1;


	vbox=gtk_hbox_new(FALSE, 0);
	ibox=gtk_vbox_new(FALSE, 0);

	select=v_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), self);
	gtk_box_pack_start(GTK_BOX(vbox), select, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), ibox, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(ibox), 10);
	self->sseg=seven_seg_new();
	gtk_box_pack_start(GTK_BOX(ibox), self->sseg, FALSE, FALSE, 0);

	gtk_widget_show_all(vbox);
	self->window=gui_add(vbox, self, "7 segmented display");

	return 0;
}
void module_exit(instance *self, const char *str)
{
	printf("[7seg:%d]\texiting because of %s\n",self->id, str);
	gui_remove(self->window);
}

module_info_t module_info={
	"7 segmented display",
	M_SPACE_SIZE	(sizeof(instance)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	M_PORT_CHANGED	(module_read),
};
