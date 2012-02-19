#include <stdio.h>
#include <math.h>

#include <gtk/gtk.h>
#include <widgets/led.h>

static void led_resize(Led *self)
{
	int size;

	size=2*(self->radius+self->space);
	gtk_widget_set_size_request(GTK_WIDGET(self), size, size);
}

static void led_redraw(Led *self)
{
	gtk_widget_set_size_request(GTK_WIDGET(self), 0, 0);
	led_resize(self);
}

static void led_expose(Led *self)
{
	cairo_t *cr;

	if (self == NULL || GTK_WIDGET(self)->window == NULL)
		return;

	cr=gdk_cairo_create(GTK_WIDGET(self)->window);

	cairo_arc(cr, self->radius+self->space, self->radius+self->space,
		self->radius, 0, 2*M_PI);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_stroke_preserve(cr);

	if (self->active) {
			cairo_set_source_rgb(cr, self->r, self->b, self->g);
	} else {
		cairo_set_source_rgb(cr, 1, 1, 1);
	}
	cairo_fill(cr);
	cairo_destroy(cr);
}


static void led_init(Led *self)
{
	g_signal_connect(self, "expose-event",
			G_CALLBACK(led_expose), NULL);
	gtk_widget_show(GTK_WIDGET(self));
}



static void led_class_init(LedClass *class)
{
	;
}


GType led_get_type(void)
{
	GType type=0;


	type=g_type_from_name("emux51-led");

	/*	first call?	*/
	if (type == 0) {
		const GTypeInfo info =
		{
			sizeof(LedClass),
			NULL,	/* base init function	*/
			NULL,	/* base final function	*/
			(GClassInitFunc) led_class_init,
			NULL,	/* class final func	*/
			NULL,	/* class data ptr	*/
			sizeof(Led),
			0,	/* n preallocs	*/
			(GInstanceInitFunc) led_init,
		};
		type=g_type_register_static(GTK_TYPE_DRAWING_AREA,
					"emux51-led",
					&info,0);
	}

	return type;
}
/****************	interface	*****************/
GtkWidget *led_new(int radius, int space)
{
	Led *self;

	self=g_object_new(led_get_type(), NULL);

	self->radius=radius;
	self->space=space;
	led_resize(self);
	return GTK_WIDGET(self);
}
void led_set_active(GtkWidget *led, int active)
{
	Led *self=LED(led);

	/*	TODO: check me	*/
	if (self->active == active)
		return;
	self->active=active;
	led_redraw(self);
}

void led_set_rgb(GtkWidget *led, double r, double g, double b)
{
	Led *self=LED(led);

	self->r=r;
	self->g=g;
	self->b=b;
	led_redraw(self);
}

