#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include <module.h>
#include <widgets/port_selector.h>

typedef struct {
	int id;

	int port;
	int ext_state;
	GtkWidget *window;
	GtkWidget *check_buttons[8];
} inst_t;

static void toggled(GtkToggleButton *button, inst_t *self)
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


static void port_select(PortSelector *ps, inst_t *self)
{
	char port;
	int i;

	port=port_selector_get_port(ps);
	if (port == self->port)
		return;

	printf("[switch:%d]\tport %d was selected\n", self->id, port);
	if (alloc_bits(self, port, 0xFF)){
		printf("[switch:%d]\tcan't allocate port %d\n", self->id, port);
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

int module_init(inst_t *self)
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
			printf("[switch:%d]\tusing port %d\n", self->id, j);
			self->port=j;
			port_selector_set_port(PORT_SELECTOR(select), j);
			break;
		}
	}
	if (j == 4) {
		fprintf(stderr, "[switch:%d]\tno empty port found\n", self->id);
		return -1;
	}
	gtk_widget_show_all(main_box);
	self->window=gui_add(main_box, self, "switch panel");
	return 0;
}

void module_exit(inst_t *self, const char *str)
{
	printf("[switch:%d] exiting because of %s\n", self->id, str);

	gui_remove(self->window);
}

module_info_t module_info={
	"switch panel",
	M_SPACE_SIZE	(sizeof(inst_t)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	NULL,
};
