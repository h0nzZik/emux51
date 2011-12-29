#ifndef _7SEG_H
#define _7SEG_H
#include <glib.h>
G_BEGIN_DECLS

typedef struct _Led
{
	GtkDrawingArea da;
	int active;

	int radius;
	int space;

	double r;
	double g;
	double b;
	
} Led;

typedef struct _LedClass
{
	GtkDrawingAreaClass parent_class;
} LedClass;

#define LED(obj)	(G_TYPE_CHECK_INSTANCE_CAST(\
			(obj), led_get_type(), Led))



GtkWidget *led_new(int radius, int space);
void led_set_active(GtkWidget *led, int active);
void led_set_rgb(GtkWidget *led, double r, double g, double b);


G_END_DECLS
#endif
