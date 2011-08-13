#ifndef _7SEG_H
#define _7SEG_H

#include <glib.h>

G_BEGIN_DECLS

#define SEG_SGLEN 30
#define SEG_OBESITY 10
#define SEG_TRLEN (SEG_OBESITY/2)




typedef struct _SevenSeg
{
	GtkDrawingArea da;
	guchar lighting_segments;
	GdkColor contour_color;
	GdkColor filler_color;
	GdkColor empty_color;
} SevenSeg;

typedef struct _SevenSegClass
{
	GtkDrawingAreaClass parent_class;
} SevenSegClass;


GtkWidget *seven_seg_new(void);
void seven_seg_set_segments(GtkWidget *widget, guchar data);

GType seven_seg_get_type(void);



#define SEVEN_SEG(obj)	(G_TYPE_CHECK_INSTANCE_CAST(\
			(obj), seven_seg_get_type(), SevenSeg))



G_END_DECLS
#endif
