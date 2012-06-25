#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include <module.h>
#include <widgets/port_selector.h>

typedef struct {
	module_base_t base;

	int port;
	int ext_state;
	GtkWidget *window;
	GtkWidget *check_buttons[8];
} Switch;

static void toggled(GtkToggleButton *button, Switch *self)
{
	int active;
	int bit;
	int i;

	/* looking for our bit	*/
	bit=0;
	for (i=0; i<8; i++){
		if ((void *)self->check_buttons[i] == (void *)button) {
			bit=7-i;
			break;
		}
	}

	active=gtk_toggle_button_get_active(button);
	if (active) {
		self->ext_state|=1<<bit;
	} else {
		self->ext_state&=~(1<<bit);
	}
	write_port(self, self->port, self->ext_state);
}


static void port_select(PortSelector *ps, Switch *self)
{
	char port;
	int i;

	port=port_selector_get_port(ps);
	if (port == self->port)
		return;

	printf("[switch:%d]\tport %d was selected\n", *(int *)self, port);
	if (alloc_bits(self, port, 0xFF)){
		printf("[switch:%d]\tcan't allocate port %d\n", *(int *)self, port);
		port_selector_set_port(ps, self->port);
		return;
	}
	/* free port */
	free_bits(self, self->port, 0xFF);

	/* reset buttons */
	for (i=0; i<8; i++) {
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(self->check_buttons[i]), 1);
	}
	/* new port */
	self->port=port;
	
}

int switch_reset(Switch *self)
{
	int i;

	for (i=0; i<8; i++) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->check_buttons[i]), TRUE);
	}
	self->ext_state=0xFF;
	return 0;
}

int module_init(Switch *self)
{
	int j;

	GtkWidget *main_box;
	GtkWidget *button;
	GtkWidget *button_box;
	GtkWidget *select;

	self->ext_state=0xFF;



	main_box=gtk_vbox_new(FALSE, 0);
	select=h_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), self);
	gtk_box_pack_start(GTK_BOX(main_box), select, FALSE, FALSE, 0);

	/*	buttons	*/
	button_box=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 0);

	for(j=0; j<8; j++) {
		button=gtk_check_button_new();
		g_signal_connect(button, "toggled",
			G_CALLBACK(toggled), self);
		gtk_box_pack_start(GTK_BOX(button_box), button,	FALSE, FALSE, 0);
		self->check_buttons[j]=button;
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(self->check_buttons[j]), 1);
	}

	/*	try to find empty port	*/
	for(j=0; j<4; j++) {
		if (alloc_bits(self, j, 0xFF) == 0){
			printf("[switch:%d]\tusing port %d\n", *(int *)self, j);
			self->port=j;
			port_selector_set_port(PORT_SELECTOR(select), j);
			break;
		}
	}
	if (j == 4) {
		fprintf(stderr, "[switch:%d]\tno empty port found\n", *(int *)self);
		return -1;
	}
	gtk_widget_show_all(main_box);
	self->window=gui_add(main_box, self, "switch panel");
	return 0;
}

void module_exit(Switch *self, const char *str)
{
	printf("[switch:%d] exiting because of %s\n", *(int *)self, str);

	gui_remove(self->window);
}

module_info_t modules[]=
{
	{ "switch panel",
		M_SPACE_SIZE	(sizeof(Switch)),
		M_INIT		(module_init),
		M_EXIT		(module_exit),
		M_PORT_CHANGED	(NULL),
		M_RESET		(switch_reset),
	},
	{ NULL }
};
