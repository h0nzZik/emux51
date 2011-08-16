#ifndef PORT_SELECTOR_H
#define PORT_SELECTOR_H

#include <glib.h>



G_BEGIN_DECLS

typedef struct
{
	GtkBox box;
	GtkWidget *buttons[4];
	guint port;
} PortSelector;

typedef struct
{
	GtkBoxClass parent_class;
	void (*port_select)(PortSelector *ps);
} PortSelectorClass;

/*	returns horizontal port selector	*/
GtkWidget *v_port_selector_new(void);
/*	returns vertical port selector		*/
GtkWidget *h_port_selector_new(void);
GtkWidget *port_selector_new(void);

guint port_selector_get_port(PortSelector *ps);
void port_selector_set_port(PortSelector *ps, guint port);


GType port_selector_get_type(void);
#define PORT_SELECTOR(obj) (G_TYPE_CHECK_INSTANCE_CAST(\
			(obj), port_selector_get_type(), PortSelector))

G_END_DECLS
#endif
