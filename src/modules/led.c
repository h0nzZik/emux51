/*	TODO: rewrite using LED widget (not implemented yet)	*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <time.h>


#include <module.h>
#include <widgets/port_selector.h>

typedef struct {
	int id;

	GtkWidget *window;
	GtkWidget *da;
	GtkWidget *label;
	char port_data;
	char port;
	GdkColor filler_c;
	GdkColor empty_c;
}my_t;


#define LED_SPACE 4
#define LED_COUNT 8
#define LED_RADIUS 24

/*		leds are numbered from left to right	*/
void led_set(GtkWidget *da, int led, GdkColor color)
{
	GdkColor black;
	GdkGC *gc;

	gc=gdk_gc_new(da->window);
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_arc(da->window, gc, 1,
			/* x */	LED_SPACE/2+led*(LED_SPACE+LED_RADIUS),
			/* y */ 0,
				LED_RADIUS, LED_RADIUS, 0, 64*360);
	g_object_unref(gc);

	black.red=black.green=black.blue=0;
	gc=gdk_gc_new(da->window);
	gdk_gc_set_rgb_fg_color(gc, &black);
	gdk_draw_arc(da->window, gc, 0,
		/* x */	LED_SPACE/2+led*(LED_SPACE+LED_RADIUS),
		/* y */ 0,
			LED_RADIUS, LED_RADIUS, 0, 64*360);
	g_object_unref(gc);
}

void clear_area(GtkWidget *da)
{
	GdkColor color;
	GdkGC *gc;

	color.red=0xFFFF;
	color.green=0xFFFF;
	color.blue=0xFFFF;

	gc=gdk_gc_new(da->window);
	gdk_gc_set_rgb_fg_color(gc, &color);

	gdk_draw_rectangle(da->window, gc, 1, 0, 0,
			/* x size */	LED_COUNT*(LED_RADIUS+LED_SPACE),
			/* y size */	LED_RADIUS);
}

void update_area(GtkWidget *da, char data, my_t *in)
{
	int i;

	for(i=0; i<LED_COUNT; i++) {
		led_set(da, 7-i, ((data>>i)&1)?in->filler_c:in->empty_c);
	}
}

static void da_expose(GtkWidget *widget, GdkEventExpose *event, my_t *in)
{
	/*	set size of drawing area	*/
	gtk_widget_set_size_request(widget,
				LED_COUNT*(LED_RADIUS+LED_SPACE), LED_RADIUS+4);

	update_area(widget, in->port_data, in);

}


void module_read(my_t *self, int port)
{
	if (port != self->port)
		return;

	if (read_port(self, port, &self->port_data)) {
		printf("[led]\tcan't read port\n");
	}
	update_area(self->da, self->port_data, self);
}

static void port_select(PortSelector *ps, my_t *self)
{
	self->port=port_selector_get_port(ps);

	if (read_port(self, self->port, &self->port_data)) {
		printf("[led]\tcan't read port\n");
	}
	update_area(self->da, self->port_data, self);
}


void module_exit(my_t *self, char *reason)
{
	printf("[led]\texiting because of %s.\n", reason?reason:"_what_?");
	gui_remove(self->window);
}

int module_init(my_t *self)
{
	GtkWidget *box;
	GtkWidget *select;

	self->port_data=0xFF;

	box=gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(box), 5);
	self->da=gtk_drawing_area_new();
	gtk_widget_set_size_request(self->da, 100, 10);
	g_signal_connect(self->da, "expose_event",
			G_CALLBACK(da_expose), self);
	select=h_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), self);
	gtk_box_pack_start(GTK_BOX(box), select, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), self->da, TRUE, TRUE, 0);


	/*	collor selection	*/
	memset(&self->empty_c, 0xFF, sizeof(GdkColor));
	memset(&self->filler_c, 0x00, sizeof(GdkColor));
	switch(rand()%3) {
		case 0:
			self->filler_c.red=0xFFFF;
			break;
		case 1:
			self->filler_c.green=0xFFFF;
			break;
		case 2:
			self->filler_c.blue=0xFFFF;
			break;
	}
	gtk_widget_show_all(box);

	self->window=gui_add(box, self, "LED panel");

	return 0;
}


module_info_t module_info={
	"LED panel",
	M_SPACE_SIZE	(sizeof(my_t)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	M_PORT_CHANGED	(module_read),
};

