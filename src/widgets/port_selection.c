/*
 * widgets/port_selection.c - implementation of PortSelection widget.
 */
#include <gtk/gtk.h>
#include <widgets/port_selection.h>

enum {
	SELECT_SIGNAL,
	LAST_SIGNAL
};

static guint port_selection_signals[LAST_SIGNAL]={0};



static void port_selection_toggled(GtkWidget *button, PortSelection *ps)
{
	int i;

	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
		return;

	/*	locate our button	*/
	for(i=0; i<4; i++) {
		if (ps->buttons[i] == button) {
			ps->port=i;
			g_signal_emit(ps, port_selection_signals[SELECT_SIGNAL],
					0);
			return;
		}
	}
	/*	wtf?	*/
	printf("[port_selection]\twtf?\n");
	
}

static void port_selection_init(PortSelection *ps)
{
	int i;
	char buff[20];
	GSList *group;

	gtk_container_set_border_width(GTK_CONTAINER(ps), 3);
	group=NULL;
	for(i=0; i<4; i++) {
		/*	create button	*/
		sprintf(buff, "Port %d", i);
		ps->buttons[i]=gtk_radio_button_new_with_label(group, buff);
		group=gtk_radio_button_get_group
			(GTK_RADIO_BUTTON (ps->buttons[i]));
		/*	add it to box	*/
		gtk_box_pack_start(GTK_BOX(ps), ps->buttons[i], FALSE, FALSE, 0);
		/*	and connect signal	*/
		g_signal_connect(ps->buttons[i], "toggled",
				G_CALLBACK(port_selection_toggled), ps);
		gtk_widget_show(ps->buttons[i]);
	}


	ps->port=0;

}

static void port_selection_class_init(PortSelectionClass *c)
{
	port_selection_signals[SELECT_SIGNAL]=
		g_signal_new (	"port-select",
				G_TYPE_FROM_CLASS(c),
				G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
				G_STRUCT_OFFSET(PortSelectionClass, port_select),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


GType port_selection_get_type(void)
{
	static GType ps_type=0;

	/*	first call?	*/
	if(ps_type == 0) {
		const GTypeInfo ps_info =
		{
			sizeof(PortSelectionClass),
			NULL,	/* base init function	*/
			NULL,	/* base final function	*/
			(GClassInitFunc) port_selection_class_init,
			NULL,	/* class final func	*/
			NULL,	/* class data ptr	*/
			sizeof(PortSelection),
			0,	/* n preallocs	*/
			(GInstanceInitFunc) port_selection_init,
		};
		ps_type=g_type_register_static(GTK_TYPE_VBOX,
					"port-selector",
					&ps_info,0);
	}
	return ps_type;
}


GtkWidget *port_selection_new(void)
{
	return GTK_WIDGET(g_object_new(port_selection_get_type(), NULL));
}

guint port_selection_get_port(PortSelection *ps)
{
	return(ps->port);
}
