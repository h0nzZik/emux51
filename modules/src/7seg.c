#include <gtk/gtk.h>
#include <module.h>

modid_t id;
emuf_t *f;

GtkWidget *vbox;
GtkWidget *da;

GdkColor color;
GdkPoint horizontal_points[6];
GdkPoint vertical_points[6];
GdkPoint offset;


#define SEG_SGLEN 30
#define SEG_OBESITY 10
#define SEG_TRLEN SEG_OBESITY/2

int invert=1;

unsigned char port_no=0;
unsigned char lighting_segments=0x64;

void import_segments(void)
{
	lighting_segments=f->read_port(id, port_no);
	if (invert)
		lighting_segments=~lighting_segments;
}

int seg(unsigned x)
{
	return (lighting_segments>>x)&1;
}

void clear(GtkWidget *da)
{
	GdkColor color;
	GdkGC *gc;

	color.red=0xFFFF;
	color.green=0xFFFF;
	color.blue=0xFFFF;

	gc=gdk_gc_new(da->window);
	gdk_gc_set_rgb_fg_color(gc, &color);

	gdk_draw_rectangle(da->window, gc, 1, 0, 0,
			/* x size */	1*SEG_SGLEN+SEG_OBESITY+offset.x,
			/* y size */	2*SEG_SGLEN+SEG_OBESITY+offset.y);
	g_object_unref(gc);
}

void draw_segment(GtkWidget *da, GdkPoint *seg_points, int x, int y, int filled)
{
	int i;
	GdkGC *gc;
	GdkPoint points[6];
	GdkColor black;
	black.red=black.green=black.blue=0;

	for(i=0; i<6; i++) {
		points[i].x=seg_points[i].x+x+offset.x/2;
		points[i].y=seg_points[i].y+y+offset.y/2;
	}

	/*	fill	*/
	if (filled) {
		gc=gdk_gc_new(da->window);
		gdk_gc_set_rgb_fg_color(gc, &color);	
		gdk_draw_polygon(da->window, gc, TRUE, points, 6);	
		g_object_unref(gc);
	}

	/*	contour	*/
	gc=gdk_gc_new(da->window);
	gdk_gc_set_rgb_fg_color(gc, &black);	
	gdk_draw_polygon(da->window, gc, FALSE, points, 6);	
	g_object_unref(gc);
}

void redraw(void)
{
	clear(da);
	draw_segment(da, horizontal_points, SEG_OBESITY/2, 0, seg(0));
	draw_segment(da, vertical_points, SEG_SGLEN, SEG_TRLEN, seg(1));
	draw_segment(da, vertical_points, SEG_SGLEN, SEG_SGLEN+SEG_TRLEN, seg(2));
	draw_segment(da, horizontal_points, SEG_OBESITY/2, 2*SEG_SGLEN, seg(3));
	draw_segment(da, vertical_points, 0, SEG_SGLEN+SEG_TRLEN, seg(4));
	draw_segment(da, vertical_points, 0, SEG_TRLEN, seg(5));
	draw_segment(da, horizontal_points, SEG_OBESITY/2, SEG_SGLEN, seg(6));
}

gboolean da_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	import_segments();
	redraw();
}


void module_read(int port)
{
	if (port != port_no)
		return;
	import_segments();
	redraw();	
		
}

int ports[]={0, 1, 2, 3};

static void port_selection(GtkWidget *button, int *no)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
		printf("[7seg]\tport %d was set as active\n", *no);
		port_no=*no;
		import_segments();
		redraw();
	}
}

void *module_init(modid_t modid, emuf_t *funcs)
{
	/*	init module	*/
	int i;
	char buff[80];

	GtkWidget *button;
	GtkWidget *expander;
	GtkWidget *ibox;	/*	box inside expander	*/

	f=funcs;
	id=modid;

	if (f->handle_event(id, "read", module_read)){
		printf("error 'handling'");
		return NULL;
	}

	/*	init gui	*/
	vbox=gtk_hbox_new(FALSE, 0);


	/*	port selection	*/
/*	expander=gtk_expander_new("Port");
	gtk_box_pack_start(GTK_BOX(vbox), expander, FALSE, FALSE, 0);*/
	ibox=gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), ibox, FALSE, FALSE, 0);
/*	gtk_container_add(GTK_CONTAINER(expander), ibox);*/


	button=gtk_radio_button_new_with_label(NULL, "P0");
	g_signal_connect(button, "toggled", G_CALLBACK(port_selection),
				(gpointer)(ports+0));
	gtk_box_pack_start(GTK_BOX(ibox), button, FALSE, FALSE, 0);

	button=gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(button), "P1");
	g_signal_connect(button, "toggled", G_CALLBACK(port_selection),
				(gpointer)(ports+1));
	gtk_box_pack_start(GTK_BOX(ibox), button, FALSE, FALSE, 0);

	button=gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(button), "P2");
	g_signal_connect(button, "toggled", G_CALLBACK(port_selection),
				(gpointer)(ports+2));
	gtk_box_pack_start(GTK_BOX(ibox), button, FALSE, FALSE, 0);

	button=gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(button), "P3");
	g_signal_connect(button, "toggled", G_CALLBACK(port_selection),
				(gpointer)(ports+3));
	gtk_box_pack_start(GTK_BOX(ibox), button, FALSE, FALSE, 0);

	/*	drawing area	*/
	da=gtk_drawing_area_new();
	g_signal_connect(da, "expose_event", G_CALLBACK(da_expose), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), da, TRUE, TRUE, 0);

	vertical_points[0].x=SEG_OBESITY/2;
	vertical_points[0].y=0;
	vertical_points[1].x=SEG_OBESITY;
	vertical_points[1].y=SEG_TRLEN;
	vertical_points[2].x=SEG_OBESITY;
	vertical_points[2].y=SEG_SGLEN-SEG_TRLEN;
	vertical_points[3].x=SEG_OBESITY/2;
	vertical_points[3].y=SEG_SGLEN;
	vertical_points[4].x=0;
	vertical_points[4].y=SEG_SGLEN-SEG_TRLEN;
	vertical_points[5].x=0;
	vertical_points[5].y=SEG_TRLEN;

	horizontal_points[0].x=0;
	horizontal_points[0].y=SEG_OBESITY/2;
	horizontal_points[1].x=SEG_TRLEN;
	horizontal_points[1].y=0;
	horizontal_points[2].x=SEG_SGLEN-SEG_TRLEN;
	horizontal_points[2].y=0;
	horizontal_points[3].x=SEG_SGLEN;
	horizontal_points[3].y=SEG_OBESITY/2;
	horizontal_points[4].x=SEG_SGLEN-SEG_TRLEN;
	horizontal_points[4].y=SEG_OBESITY;
	horizontal_points[5].x=SEG_TRLEN;
	horizontal_points[5].y=SEG_OBESITY;

	offset.x=offset.y=40;

	gtk_widget_set_size_request(da, offset.x+1*SEG_SGLEN+SEG_OBESITY,
					offset.y+2*SEG_SGLEN+SEG_OBESITY);

	color.red=0xFFFF;
	color.green=0;
	color.blue=0;



	gtk_widget_show_all(vbox);

	return vbox;
}
void module_exit(const char *str)
{
	printf("[7seg]\texiting because of %s\n", str);
}
