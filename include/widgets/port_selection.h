#ifndef PORT_SELECTION_H
#define PORT_SELECTION_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _PortSelection
{
	GtkVBox box;
	GtkWidget *buttons[4];
	guint port;
} PortSelection;

typedef struct _PortSelectionClass
{
	GtkVBoxClass parent_class;
	void (*port_select)(PortSelection *ps);
} PortSelectionClass;


GtkWidget *port_selection_new(void);
guint port_selection_get_port(PortSelection *ps);


G_END_DECLS
#endif
