#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>


#include <module.h>

modid_t id;
emuf_t *f;

GtkWidget *main_box;
GtkWidget *boxes[4];
GtkWidget *draws[4];
GtkWidget *labels[4];
char port_data[4];



#define LED_SPACE 2
#define LED_COUNT 8
#define LED_RADIUS 16

/*		leds are numbered from left to right	*/
void led_set(GtkWidget *da, int led, int state)
{
	GdkColor color;
	GdkGC *gc;

	color.red=0xFFFF;
	color.green=0;
	color.blue=0;

	gc=gdk_gc_new(da->window);
	gdk_gc_set_rgb_fg_color(gc, &color);


	gdk_draw_arc(da->window, gc, state,
		/* x */	LED_SPACE/2+led*(LED_SPACE+LED_RADIUS),
		/* y */ 0,
			LED_RADIUS, LED_RADIUS, 0, 64*360);

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

void update_area(GtkWidget *da, char data)
{
	int i;

	clear_area(da);
	for(i=0; i<LED_COUNT; i++) {
		led_set(da, 7-i, (data>>i)&1);
	}
}

gboolean da_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	int i;

	/*	set size of drawing area	*/
	gtk_widget_set_size_request(widget,
				LED_COUNT*(LED_RADIUS+LED_SPACE), LED_RADIUS);

	i=((void *)data-(void *)draws)/sizeof(GtkWidget *);
	printf("[led]\tupdating area %d\n", i);

	update_area(widget, port_data[i]);

}


void module_read(int port)
{
	char data;

	data=f->read_port(id, port);
	printf("[led]\t0x%x on port %d\n", data, port);

	port_data[port]=data;

	update_area(draws[port], data);


}


void module_exit(char *reason)
{
	printf("[led]\tI must go, because of %s\n.\t\tBye.\n", reason?reason:"_what_?");

}

void * module_init(modid_t modid, void *cbs)
{
	int i;
	char buff[40];

	printf("[led]\tinitializing leds\n");

	memset(port_data, 0xFF, 4);

	main_box=gtk_vbox_new(FALSE, 0);

	for (i=0; i<4; i++) {
/*		port_data[i]=f->read_port(id, i);*/

		boxes[i]=gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(main_box), boxes[i], TRUE, TRUE, 0);

		draws[i]=gtk_drawing_area_new();
		gtk_widget_set_size_request(draws[i], 100, 10);
		g_signal_connect(draws[i], "expose_event",
				G_CALLBACK(da_expose),draws+i);
		
		sprintf(buff, "Port %d\t", i);
		labels[i]=gtk_label_new(buff);

		gtk_box_pack_start(GTK_BOX(boxes[i]), labels[i], TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(boxes[i]), draws[i], TRUE, TRUE, 0);
	}

	id=modid;
	f=cbs;

	if (f->handle_event(id, "read", module_read)){
		printf("error 'handling'");
		return NULL;
	}/*
	for(i=0; i<4; i++) {
		module_read(i);
	}
*/
	printf("[led]\tinit ok\n");

	gtk_widget_show_all(main_box);
	return (void *) main_box;
}
