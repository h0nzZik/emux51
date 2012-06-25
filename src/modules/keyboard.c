#include <stdio.h>
#include <gtk/gtk.h>
#include <widgets/port_selector.h>
#include <module.h>

typedef struct {
	int id;
	int port_no;
	int map[12];
	char last_port_value;
	int writing;

	GtkWidget *window;
	GtkWidget *buttons[12];
} Key3x4;

char *values[]={"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};


static void key_update_port(Key3x4 *self)
{
	int i,j;
	char port_value;
	char data=0xFF;

	/*	read bits 5..7	*/
	read_port(self, self->port_no, &port_value);
	port_value&=0xE0;

	/*
	 * If there is zero on bit (5..7) and corresponding key is pressed,
	 * some of bits (0..3) must be cleared. See schematics in doc.
	 */
	for (i=0; i<3; i++) {
		if(((port_value>>(5+i)) & 1) == 0) {
			for (j=0; j<4; j++) {
				if (self->map[3*j+i]) {
					data&=~(8>>j);
				}
			}
		}
	}	


	self->writing=1;
	write_port(self, self->port_no, data);
	self->writing=0;
}

static void select_port(PortSelector *ps, Key3x4 *self)
{
	int port;

	port=port_selector_get_port(ps);
	if (port == self->port_no)
		return;
	unwatch_port(self, self->port_no);
	watch_port(self, port);

	if (alloc_bits(self, port, 0xFF)){
		printf("[kb3x4:%d]\tcan't allocate port %d\n", self->id, port);
		port_selector_set_port(ps, self->port_no);
		return;
	}
	printf("[kb3x4:%d]\tport %d was selected\n", self->id, port);

	write_port(self, self->port_no, 0xFF);
	free_bits(self, self->port_no, 0xFF);
	self->port_no=port;
	key_update_port(self);
}





static void port_changed(Key3x4 *self, int port)
{
	if (port != self->port_no)
		return;

	if (self->writing)
		return;

//	putchar('.');
	key_update_port(self);
}

static void keyboard_pressed(GtkWidget *button, Key3x4 *self)
{
	int i;
	for(i=0; i<12; i++)
		if(button == self->buttons[i])
			break;
	printf("[kb3x4:%d]\t%s pressed\n", self->id, values[i]);
	self->map[i]=1;	
	key_update_port(self);
}
static void keyboard_released(GtkWidget *button, Key3x4 *self)
{
	int i;
	for(i=0; i<12; i++)
		if(button == self->buttons[i])
			break;
	printf("[kb3x4:%d]\t%s released\n", self->id, values[i]);
	self->map[i]=0;
	key_update_port(self);
}


int keyboard_init(Key3x4 *self)
{
	int i;

	GtkWidget *vbox;
	GtkWidget *selector;
	GtkWidget *table;
	GtkWidget *button;

	watch_port(self, self->port_no);

	vbox=gtk_vbox_new(FALSE, 2);
	selector=h_port_selector_new();
	g_signal_connect(selector, "port-select", G_CALLBACK(select_port), self);
	gtk_box_pack_start(GTK_BOX(vbox), selector, FALSE, FALSE, 0);

	table=gtk_table_new(4, 3, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	for (i=0; i<12; i++) {
		button=gtk_button_new_with_label(values[i]);
		self->buttons[i]=button;
		gtk_table_attach_defaults(GTK_TABLE(table), button,
					i%3, i%3+1, i/3, 1+i/3);
		g_signal_connect(button, "pressed",
				G_CALLBACK(keyboard_pressed), self);
		g_signal_connect(button, "released",
				G_CALLBACK(keyboard_released), self);
	}


	/*	try to find empty port	*/
	for(i=0; i<4; i++) {
		if (alloc_bits(self, i, 0xFF) == 0){
			printf("[kb3x4:%d]\tusing port %d\n", self->id, i);
			self->port_no=i;
			port_selector_set_port(PORT_SELECTOR(selector), i);
			break;
		}
	}
	if (i == 4) {
		fprintf(stderr, "[kb3x4:%d]\tno empty port found\n", self->id);
		return -1;
	}
	
	gtk_widget_show_all(vbox);
	self->window=gui_add(vbox, self, "Keyboard");
	return 0;	
}


int keyboard_exit(Key3x4 *self, const char *str)
{
	printf("[kb3x4:%d] exiting because of %s\n", self->id, str);

	unwatch_port(self, self->port_no);
	gui_remove(self->window);
	return 0;
}



module_info_t modules[]=
{
	{ "Keyboard 3x4 buttons",
		M_SPACE_SIZE	(sizeof(Key3x4)),
		M_INIT		(keyboard_init),
		M_EXIT		(keyboard_exit),
		M_PORT_CHANGED	(port_changed),
		M_RESET		(NULL),
	},
	{ NULL }
};

