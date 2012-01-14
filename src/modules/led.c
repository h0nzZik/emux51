#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>


#include <module.h>
#include <widgets/port_selector.h>
#include <widgets/led.h>

typedef struct {
	int id;

	GtkWidget *window;
	GtkWidget *leds[8];
	int invert;
	char port_data;
	char port;
}instance_t;


static void update_leds(instance_t *self)
{
	int i;
	char data;
	if (self->invert)
		data=~self->port_data;
	else
		data=self->port_data;


	for (i=0; i<8; i++) {
		led_set_active(self->leds[i], (data>>i)&1);
	}
}

static void module_read(instance_t *self, int port)
{
	if (port != self->port)
		return;

	if (read_port(self, port, &self->port_data)) {
		printf("[led]\tcan't read port\n");
	}
	update_leds(self);
}

static void port_select(PortSelector *ps, instance_t *self)
{
	self->port=port_selector_get_port(ps);

	if (read_port(self, self->port, &self->port_data)) {
		printf("[led]\tcan't read port\n");
	}
	update_leds(self);
}


void module_exit(instance_t *self, char *reason)
{
	int i;
	printf("[led:%d]\texiting: %s.\n",self->id, reason);
	for (i=0; i<8; i++) {
		gtk_widget_destroy(self->leds[i]);
	}
	gui_remove(self->window);
}

int module_init(instance_t *self)
{
	int i;
	float c[3];
	GtkWidget *vbox;
	GtkWidget *led_box;
	GtkWidget *select;



	self->invert=1;


	/*	create main box with port selector..	*/
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	select=h_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), self);
	gtk_box_pack_start(GTK_BOX(vbox), select, FALSE, FALSE, 0);

	/*	randomize colors	*/
	i=rand()%3;
	c[(i+0)%3]=(rand()%0x100)/(double)0x100;
	if (c[(i+0)%3]<0.7)
		c[(i+1)%3]=(0x80+(rand()%0x80))/(double)0x80;
	else
		c[(i+1)%3]=0;
	c[(i+2)%3]=0;


	/*	.. and lots of LEDs	*/
	led_box=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), led_box, FALSE, FALSE, 0);
	for (i=0; i<8; i++) {
		self->leds[i]=led_new(12, 2);
		led_set_rgb(self->leds[i], c[0], c[1], c[2]);
		gtk_box_pack_end(GTK_BOX(led_box),
				self->leds[i], FALSE, FALSE, 0);
	}
	/*	display port 0	*/
	self->port=0;
	if (read_port(self, self->port, &self->port_data)) {
		printf("[led]\tcan't read port\n");
	}
	update_leds(self);
	

	gtk_widget_show_all(vbox);
	self->window=gui_add(vbox, self, "LED panel");

	


	return 0;
}


module_info_t module_info={
	"LED panel",
	M_SPACE_SIZE	(sizeof(instance_t)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	M_PORT_CHANGED	(module_read),
};

