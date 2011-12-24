/*
 * widgets/7seg.c - SevenSeg widget.
 */

#include <gtk/gtk.h>
#include <widgets/7seg.h>
#include <stdio.h>


GdkPoint horizontal_segment[6]={{0, SEG_OBESITY/2},
				{SEG_TRLEN, 0},
				{SEG_SGLEN-SEG_TRLEN, 0},
				{SEG_SGLEN, SEG_OBESITY/2},
				{SEG_SGLEN-SEG_TRLEN, SEG_OBESITY},
				{SEG_TRLEN, SEG_OBESITY}};

GdkPoint vertical_segment[6]={	{SEG_OBESITY/2, 0},
				{SEG_OBESITY, SEG_TRLEN},
				{SEG_OBESITY, SEG_SGLEN-SEG_TRLEN},
				{SEG_OBESITY/2, SEG_SGLEN},
				{0, SEG_SGLEN-SEG_TRLEN},
				{0, SEG_TRLEN}};


void draw_segment_filler(cairo_t *cr, GdkPoint *seg_points,
			sseg_color c, int x, int y)
{
	int i;

	cairo_new_path(cr);
	for (i=0; i<6; i++) {
		cairo_line_to(cr, 1+x+seg_points[i].x, 1+y+seg_points[i].y);
	}
	cairo_close_path(cr);


	cairo_set_source_rgba(cr, c.red, c.green, c.blue, c.alpha);
	cairo_fill(cr);

}

static void draw_segment_contour(cairo_t *cr, GdkPoint *seg_points, 
				sseg_color c, int x, int y)
{
	int i;

	cairo_new_path(cr);
	for (i=0; i<6; i++) {
		cairo_line_to(cr, 1+x+seg_points[i].x, 1+y+seg_points[i].y);
	}
	cairo_close_path(cr);

	cairo_set_source_rgb(cr, c.red, c.green, c.blue);
	cairo_stroke(cr);
}


void redraw_seven_seg_fillers(GtkWidget *instance)
{
	int i;
	int bits[8];
	guchar segs;
	cairo_t *cr;
	sseg_color on, off;


		
	on=SEVEN_SEG(instance)->on;
	off=SEVEN_SEG(instance)->off;
	segs=SEVEN_SEG(instance)->lighting_segments;

	for (i=0;i<8; i++)
		bits[i]=(segs>>i)&1;


	cr=gdk_cairo_create(instance->window);
	cairo_set_line_width(cr, 0.1);
	draw_segment_filler(cr, horizontal_segment, bits[0]?on:off,
			SEG_OBESITY/2, 0);
	draw_segment_filler(cr, vertical_segment, bits[1]?on:off,
			SEG_SGLEN, SEG_TRLEN);
	draw_segment_filler(cr, vertical_segment, bits[2]?on:off,
			SEG_SGLEN, SEG_SGLEN+SEG_TRLEN);
	draw_segment_filler(cr, horizontal_segment, bits[3]?on:off,
			SEG_OBESITY/2, 2*SEG_SGLEN);
	draw_segment_filler(cr, vertical_segment, bits[4]?on:off,
			0, SEG_SGLEN+SEG_TRLEN);
	draw_segment_filler(cr, vertical_segment, bits[5]?on:off,
			0, SEG_TRLEN);
	draw_segment_filler(cr, horizontal_segment, bits[6]?on:off,
			SEG_OBESITY/2, SEG_SGLEN);

	cairo_destroy(cr);
}


void redraw_seven_seg_contours(GtkWidget *instance)
{
	sseg_color black;
	cairo_t *cr;

	black=SEVEN_SEG(instance)->con;

	cr=gdk_cairo_create(instance->window);

	draw_segment_contour(cr, horizontal_segment, black,
			SEG_OBESITY/2, 0);
	draw_segment_contour(cr, vertical_segment, black,
			SEG_SGLEN, SEG_TRLEN);
	draw_segment_contour(cr, vertical_segment, black,
			SEG_SGLEN, SEG_SGLEN+SEG_TRLEN);
	draw_segment_contour(cr, horizontal_segment, black,
			SEG_OBESITY/2, 2*SEG_SGLEN);
	draw_segment_contour(cr, vertical_segment, black,
			0, SEG_SGLEN+SEG_TRLEN);
	draw_segment_contour(cr, vertical_segment, black,
			0, SEG_TRLEN);
	draw_segment_contour(cr, horizontal_segment, black,
			SEG_OBESITY/2, SEG_SGLEN);
	cairo_destroy(cr);
}

void seven_seg_expose(GtkWidget *instance)
{
	/*	redraw contour	*/
	redraw_seven_seg_contours(instance);

	/*	draw fillers	*/
	redraw_seven_seg_fillers(instance);
}

static void seven_seg_class_init(SevenSegClass *class)
{
	;
}

static void seven_seg_init(SevenSeg *instance)
{

	/*	turned on	*/
	instance->on.red=1;
	instance->on.green=0;
	instance->on.blue=0;
	instance->on.alpha=0.8;
	/*	turned off	*/
	instance->off.red=1;
	instance->off.green=1;
	instance->off.blue=1;
	instance->off.alpha=1;
	/*	contour 	*/
	instance->con.red=0;
	instance->con.green=0;
	instance->con.blue=0;
	instance->con.alpha=0;



	g_signal_connect(instance, "expose-event",
			G_CALLBACK(seven_seg_expose), NULL);

	gtk_widget_set_size_request(GTK_WIDGET(instance),
				SEG_AREA_XSIZE, SEG_AREA_YSIZE);

	gtk_widget_show(GTK_WIDGET(instance));
	
}


GType seven_seg_get_type(void)
{
	static GType type=0;

	/*	first call?	*/
	if(type == 0) {
		const GTypeInfo info =
		{
			sizeof(SevenSegClass),
			NULL,	/* base init function	*/
			NULL,	/* base final function	*/
			(GClassInitFunc) seven_seg_class_init,
			NULL,	/* class final func	*/
			NULL,	/* class data ptr	*/
			sizeof(SevenSeg),
			0,	/* n preallocs	*/
			(GInstanceInitFunc) seven_seg_init,
		};
		type=g_type_register_static(GTK_TYPE_DRAWING_AREA,
					"seven-seg",
					&info,0);
	}
	return type;
}


GtkWidget *seven_seg_new(void)
{
	return GTK_WIDGET(g_object_new(seven_seg_get_type(), NULL));
}


void seven_seg_set_segments(GtkWidget *widget, guchar data)
{
	SevenSeg *instance;

	instance=SEVEN_SEG(widget);
	if(instance->lighting_segments == data)
		return;
	instance->lighting_segments=data;
	if(widget->window == NULL) {
		printf("7seg_widget:\tcan't redraw NULL\n");
		return;
	}
	/*	redraw 7seg	*/
	gtk_widget_set_size_request(GTK_WIDGET(instance), 0, 0);
	gtk_widget_set_size_request(GTK_WIDGET(instance),
					SEG_AREA_XSIZE, SEG_AREA_YSIZE);	
}

