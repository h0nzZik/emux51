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
} Display7seg;

void import_segments(Display7seg *self)
{
	char segments;
	read_port(self, self->port_no, &segments);

	if (self->invert)
		segments=~segments;
	seven_seg_set_segments(self->sseg, segments);
	
}

void module_read(Display7seg *self, int port)
{
	if (port != self->port_no)
		return;
	import_segments(self);
}


static void port_select(PortSelector *ps, Display7seg *self)
{
	unwatch_port(self, self->port_no);
	self->port_no=port_selector_get_port(ps);
	watch_port(self, self->port_no);
	import_segments(self);
}


void *module_init(Display7seg *self)
{
	/*	init module	*/
	GtkWidget *select;
	GtkWidget *vbox;
	GtkWidget *ibox;

	self->invert=1;

	watch_port(self, 0);

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
void module_exit(Display7seg *self, const char *str)
{
	printf("[7seg:%d]\texiting because of %s\n",self->id, str);
	unwatch_port(self, self->port_no);
	gui_remove(self->window);
}

module_info_t modules[]=
{
	{ "7 segmented display",
		M_SPACE_SIZE	(sizeof(Display7seg)),
		M_INIT		(module_init),
		M_EXIT		(module_exit),
		M_PORT_CHANGED	(module_read),
		M_RESET		(NULL),
	},
	{ NULL }
};
