#include <stdio.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <module.h>

#include <widgets/port_selector.h>
#include <widgets/7seg.h>



typedef struct {
	module_base_t base;
	int port_no;
	char lighting_segments[3];
	int history[3];
	GtkWidget *window;
	GtkWidget *ssegs[3];
	int mask;
	void *event;
} Display3x7seg;




void off_handler(Display3x7seg *self, void *data)
{
	int i;
	for(i=0; i<3; i++){
		if ((self->mask>>i)&1 && self->lighting_segments[i] && !self->history[i]){
			self->lighting_segments[i]=0;		
			seven_seg_set_segments(self->ssegs[2-i], 0);
		}
		else {
			self->history[i]=0;	
		}
	}
	sync_timer_add(self->event, 40);
}


/*	import and decoding	*/

/*	inverted decode array	*/
const char decode_arr[16]={0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02,
		0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};
void import_segments(Display3x7seg *self)
{
	char data;
	int value;
	char light;
	int i;
	int mask;

	read_port(self, self->port_no, &data);
	value=data&0x0F;
	mask=data>>4;
	light=~decode_arr[value];

	for (i=0; i<3; i++) {
		/*	light	*/
		if (((mask>>i)&1) == 0) {
			if (self->lighting_segments[i]!=light) {
				seven_seg_set_segments(self->ssegs[2-i], light);
				self->lighting_segments[i]=light;
			}
			self->history[i]=1;
		}
	}
	self->mask=mask;
}
void module_read(Display3x7seg *self, int port)
{
	if (port != self->port_no) {
		return;
	}
	import_segments(self);
}

int module_reset(Display3x7seg *self)
{
	int i;
	for (i=0; i<3; i++) {
		seven_seg_set_segments(self->ssegs[i], 0);
		self->lighting_segments[i]=0x00;
		self->history[i]=0;
	}
	self->mask=0xFF;
	sync_timer_unlink(self->event);
	sync_timer_add(self->event, 40);

	return 0;
}


/*	handler of "port-select" signal	*/
static void port_select(PortSelector *ps, Display3x7seg *self)
{
	int i;
	unwatch_port(self, self->port_no);
	self->port_no=port_selector_get_port(ps);
	watch_port(self, self->port_no);
	module_reset(self);
	/*	redraw	*/
	import_segments(self);
}


int module_init(Display3x7seg *self)
{
	/*	init module	*/
	int i;

	GtkWidget *hbox;
	GtkWidget *select;

	GtkWidget *tmpbox;
	self->mask=0xFF;


	/*	init gui	*/
	hbox=gtk_hbox_new(FALSE, 0);

	select=v_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), self);
	gtk_box_pack_start(GTK_BOX(hbox), select, FALSE, FALSE, 0);
	watch_port(self, 0);

	for(i=0; i<3; i++) {
		self->ssegs[i]=seven_seg_new();
		gtk_box_pack_start(GTK_BOX(hbox), self->ssegs[i], FALSE, FALSE, 5);

	}

	/*	allocate timer event	*/
	self->event=timer_event_alloc(self, M_QUEUE(off_handler), NULL);
	if (self->event == NULL) {
		return 1;
	}
	sync_timer_add(self->event, 40);

	gtk_widget_show_all(hbox);
	self->window=gui_add(hbox, self, "3x7seg panel");
	return 0;
}
int module_exit(Display3x7seg *self, const char *str)
{
	printf("[3x7seg:%d]\texiting because of %s\n", *(int *)self, str);
	sync_timer_unlink(self->event);
	g_free(self->event);
	unwatch_port(self, self->port_no);
	gui_remove(self->window);
	return 0;
}

module_info_t modules[]=
{
	{"3x7seg panel",
		M_SPACE_SIZE	(sizeof(Display3x7seg)),
		M_INIT		(module_init),
		M_EXIT		(module_exit),
		M_PORT_CHANGED	(module_read),
		M_RESET		(module_reset),
	},
	{ NULL }
};
