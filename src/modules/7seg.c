#include <gtk/gtk.h>
#include <module.h>

#include <widgets/7seg.h>
#include <widgets/port_selection.h>

typedef struct {
	modid_t id;
	emuf_t *f;
	GtkWidget *sseg;
	unsigned char port_no;
	int invert;
} mi_t;

void import_segments(mi_t *in)
{
	unsigned char segments;
	segments=in->f->read_port(in->id, in->port_no);
	if (in->invert)
		segments=~segments;
	seven_seg_set_segments(in->sseg, segments);
	
}

void module_read(mi_t *in, int port)
{
	if (port != in->port_no)
		return;
	import_segments(in);
}


static void port_select(PortSelection *ps, gpointer data)
{
	mi_t *in;
	in=data;
	in->port_no=port_selection_get_port(ps);
	import_segments(in);
}

void *module_init(modid_t modid, emuf_t *funcs)
{
	/*	init module	*/
	int i;
	char buff[80];

	GtkWidget *select;
	GtkWidget *vbox;
	GtkWidget *ibox;

	mi_t *in;
	in=g_malloc0(sizeof(mi_t));
	in->f=funcs;
	in->id=modid;
	in->invert=1;
	if (in->f->handle_event(in->id, "read", module_read)){
		in->f->crash(in->id, "handle");
	}

	vbox=gtk_hbox_new(FALSE, 0);
	ibox=gtk_vbox_new(FALSE, 0);

	select=port_selection_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), in);
	gtk_box_pack_start(GTK_BOX(vbox), select, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), ibox, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(ibox), 10);
	in->sseg=seven_seg_new();
	gtk_box_pack_start(GTK_BOX(ibox), in->sseg, FALSE, FALSE, 0);


	gtk_widget_show_all(vbox);
	in->f->set_space(in->id, in);
	in->f->set_name(in->id, "7 segmented display");

	return vbox;
}
void module_exit(mi_t *in, const char *str)
{
	printf("[7seg:%d]\texiting because of %s\n",in->id.id, str);
	g_free(in);
}
