/*
 * widgets/7seg.c - implementation of SevenSeg widget.
 */

int test_func(void)
{
	return 5;
}

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


void draw_segment(GtkWidget *instance, GdkPoint *seg_points,
			GdkColor filler, GdkColor contour, int x, int y)
{
	int i;
	GdkGC *gc;
	GdkPoint points[6];

	for(i=0; i<6; i++) {
		points[i].x=seg_points[i].x+x;
		points[i].y=seg_points[i].y+y;
	}

	/*	filler	*/
	gc=gdk_gc_new(instance->window);
	gdk_gc_set_rgb_fg_color(gc, &filler);	
	gdk_draw_polygon(instance->window, gc, TRUE, points, 6);	
	g_object_unref(gc);

	/*	contour	*/
	gc=gdk_gc_new(instance->window);
	gdk_gc_set_rgb_fg_color(gc, &contour);	
	gdk_draw_polygon(instance->window, gc, FALSE, points, 6);	
	g_object_unref(gc);
}

void draw_seven_seg(GtkWidget *instance)
{
	guchar segs;
	int bits[8];
	int i;
	GdkColor fc, ec, cc;

	cc=SEVEN_SEG(instance)->contour_color;
	ec=SEVEN_SEG(instance)->empty_color;
	fc=SEVEN_SEG(instance)->filler_color;
	segs=SEVEN_SEG(instance)->lighting_segments;

	for (i=0;i<8; i++)
		bits[i]=(segs>>i)&1;

	draw_segment(instance, horizontal_segment, bits[0]?fc:ec, cc,
			SEG_OBESITY/2, 0);
	draw_segment(instance, vertical_segment, bits[1]?fc:ec, cc,
			SEG_SGLEN, SEG_TRLEN);
	draw_segment(instance, vertical_segment, bits[2]?fc:ec, cc,
			SEG_SGLEN, SEG_SGLEN+SEG_TRLEN);
	draw_segment(instance, horizontal_segment, bits[3]?fc:ec, cc,
			SEG_OBESITY/2, 2*SEG_SGLEN);
	draw_segment(instance, vertical_segment, bits[4]?fc:ec, cc,
			0, SEG_SGLEN+SEG_TRLEN);
	draw_segment(instance, vertical_segment, bits[5]?fc:ec, cc,
			0, SEG_TRLEN);
	draw_segment(instance, horizontal_segment, bits[6]?fc:ec, cc,
			SEG_OBESITY/2, SEG_SGLEN);
}


static void seven_seg_class_init(SevenSegClass *class)
{
	;
}

static void seven_seg_init(SevenSeg *instance)
{
	/*	black contour	*/
	instance->contour_color.red=0;
	instance->contour_color.green=0;
	instance->contour_color.blue=0;
	/*	red filler	*/
	instance->filler_color.red=0xFFFF;
	instance->filler_color.green=0;
	instance->filler_color.blue=0;
	/*	white empty	*/
	instance->empty_color.red=0xFFFF;
	instance->empty_color.green=0xFFFF;
	instance->empty_color.blue=0xFFFF;

	g_signal_connect(instance, "expose-event",
			G_CALLBACK(draw_seven_seg), NULL);
	gtk_widget_set_size_request(GTK_WIDGET(instance),
				SEG_SGLEN+SEG_OBESITY+1, 2*SEG_SGLEN+SEG_OBESITY+1);
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
	draw_seven_seg(GTK_WIDGET(instance));
}

