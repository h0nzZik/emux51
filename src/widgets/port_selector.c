/*
 * widgets/port_selector.c - implementation of PortSelector widget.
 */
#include <gtk/gtk.h>
#include <widgets/port_selector.h>

enum {
	SELECT_SIGNAL,
	LAST_SIGNAL
};

static guint port_selector_signals[LAST_SIGNAL]={0};



static void port_selector_toggled(GtkWidget *button, PortSelector *ps)
{
	int i;

	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
		return;

	/*	locate our button	*/
	for(i=0; i<4; i++) {
		if (ps->buttons[i] == button) {
			ps->port=i;
			g_signal_emit(ps, port_selector_signals[SELECT_SIGNAL],
					0);
			return;
		}
	}
	/*	wtf?	*/
}


static void port_selector_init(PortSelector *ps)
{
	int i;
	char buff[20];
	GSList *group;

	gtk_container_set_border_width(GTK_CONTAINER(ps), 3);
	gtk_box_set_homogeneous(GTK_BOX(ps), 1);
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
				G_CALLBACK(port_selector_toggled), ps);
		gtk_widget_show(ps->buttons[i]);
	}
	ps->port=0;
}


static void port_selector_class_init(PortSelectorClass *c)
{
	port_selector_signals[SELECT_SIGNAL]=
		g_signal_new (	"port-select",
				G_TYPE_FROM_CLASS(c),
				G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
				G_STRUCT_OFFSET(PortSelectorClass, port_select),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

GType port_selector_get_type(void)
{
	GType ps_type=0;


	ps_type=g_type_from_name("emux51-port_selector");
	/*	first call?	*/
	if(ps_type == 0) {
		const GTypeInfo ps_info =
		{
			sizeof(PortSelectorClass),
			NULL,	/* base init function	*/
			NULL,	/* base final function	*/
			(GClassInitFunc) port_selector_class_init,
			NULL,	/* class final func	*/
			NULL,	/* class data ptr	*/
			sizeof(PortSelector),
			0,	/* n preallocs	*/
			(GInstanceInitFunc) port_selector_init,
		};
		ps_type=g_type_register_static(GTK_TYPE_VBOX,
					"emux51-port_selector",
					&ps_info,0);
	}
	return ps_type;
}


/*	default: vertical orientation	*/
GtkWidget *port_selector_new(void)
{
	return GTK_WIDGET(g_object_new(port_selector_get_type(), NULL));
}

/*	change to horizontal	*/
GtkWidget *h_port_selector_new(void)
{
	int i;
	char buff[20];
	PortSelector *selector;

	/*	new selector	*/
	selector=PORT_SELECTOR(port_selector_new());
	/*	change orientation	*/
	gtk_orientable_set_orientation(GTK_ORIENTABLE(selector),
					GTK_ORIENTATION_HORIZONTAL);
	/*	and rename labels	*/
	for (i=0; i<4; i++) {
		sprintf(buff, "P%d", i);
		gtk_button_set_label(GTK_BUTTON(selector->buttons[i]), buff);
	}
	return GTK_WIDGET(selector);
}
/*	only other name	*/
GtkWidget *v_port_selector_new(void)
{
	return port_selector_new();
}



guint port_selector_get_port(PortSelector *ps)
{
	return(ps->port);
}
void port_selector_set_port(PortSelector *ps, guint port)
{
	port%=4;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ps->buttons[port]), TRUE);
}
