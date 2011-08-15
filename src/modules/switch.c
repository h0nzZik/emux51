#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <module.h>
#include <widgets/port_selection.h>

typedef struct {
	modid_t id;
	emuf_t *f;

	int port;
	GtkWidget *check_buttons[8];
	char ext_state;
} inst_t;

static void toggled(GtkToggleButton *button, inst_t *in)
{
	int active;
	int bit;
	int i;

	/* looking for our bit	*/
	bit=0;
	for (i=0; i<8; i++){
		if ((void *)in->check_buttons[i] == (void *)button) {
			bit=8-i;
			break;
		}
	}

	active=gtk_toggle_button_get_active(button);
	if (active) {
		in->ext_state|=1<<bit;
	} else {
		in->ext_state&=~(1<<bit);
	}
	printf("there is now 0x%x\n", in->ext_state);
	in->f->write_port(in->id, in->port, in->ext_state);
}


static void port_select(PortSelection *ps, inst_t *self)
{
	char port;
	int i;

	port=port_selection_get_port(ps);
	if (self->f->alloc_bits(self->id, port, 0xFF)){
		printf("cann't alloc port\n");
		return;
	}
	/* reset port */
	self->f->write_port(self->id, self->port, 0xFF);
	self->f->free_bits(self->id, self->port, 0xFF);
	/* reset buttons */
	for (i=0; i<8; i++) {
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(self->check_buttons[i]), 1);
	}
	/* new port */
	self->port=port;
	
}

void * module_init(modid_t modid, void *cbs)
{
	int j;

	GtkWidget *main_box;
	GtkWidget *button;
	GtkWidget *button_box;
	GtkWidget *select;
	inst_t *in;

	printf("tady dobry\n");
	in=g_malloc0(sizeof(inst_t));	
	printf("tady dobry\n");
	printf("tady dobry\n");

	in->id=modid;
	in->f=cbs;

	in->ext_state=0xFF;



	main_box=gtk_vbox_new(FALSE, 0);
	select=h_port_selection_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), in);
	gtk_box_pack_start(GTK_BOX(main_box), select, FALSE, FALSE, 0);

	/*	buttons	*/
	button_box=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 0);

	for(j=0; j<8; j++) {
		button=gtk_check_button_new();
		g_signal_connect(button, "toggled",
			G_CALLBACK(toggled), in);
		gtk_box_pack_start(GTK_BOX(button_box), button,	FALSE, FALSE, 0);
		in->check_buttons[j]=button;
	}


	gtk_widget_show_all(main_box);
	printf("tady taky\n");
	in->f->set_space(in->id, in);
	return main_box;
}

void module_exit(inst_t *self, const char *str)
{
	printf("[switch:%d] exiting because of %s\n", self->id.id, str);

	self->f->write_port(self->id, self->port, 0xFF);
	self->f->free_bits(self->id, self->port, 0xFF);

	g_free(self);
}
