#include <stdio.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <module.h>

#include <widgets/port_selector.h>
#include <widgets/7seg.h>



typedef struct {
	int id;

	int port_no;
	int current;
	int history[8];

	GtkWidget *window;
	GtkWidget *ssegs[8];
	void *event;
} Display8x7segMX;




void off_handler(Display8x7segMX *self, void *data)
{
	int i;

	for(i=0; i<8; i++){
		if(i != self->current) {
			if (self->history[i])
				self->history[i]=0;
			else
				seven_seg_set_segments(self->ssegs[i], 0);
		}
	}
	sync_timer_add(self->event, 40);
}


/*	import and decoding	*/

/*	inverted decode array	*/
const char decode_arr[16]={0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02,
		0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};
void import_segments(Display8x7segMX *self)
{
	char data;
	char lighting;
	int select;

	read_port(self, self->port_no, &data);

	select=data>>4;
	self->current=select;
	/*	suppression	*/
	if ((data>>7)&1)
		return;
	lighting=~decode_arr[data&0x0F];
	self->history[select]=1;
	seven_seg_set_segments(self->ssegs[select], lighting);
}

void module_read(Display8x7segMX *self, int port)
{
	if (port != self->port_no)
		return;
	import_segments(self);
}

/*	handler of "port-select" signal	*/
static void port_select(PortSelector *ps, Display8x7segMX *self)
{
	int i;
	unwatch_port(self, self->port_no);
	self->port_no=port_selector_get_port(ps);
	watch_port(self, self->port_no);

	/*	it was turned off	*/
	for(i=0; i<8; i++){
		seven_seg_set_segments(self->ssegs[i], 0);
	}
	/*	redraw	*/
	import_segments(self);
}


int module_reset(Display8x7segMX *self)
{
	int i;
	for (i=0; i<8; i++) {
		seven_seg_set_segments(self->ssegs[i], 0);
		self->history[i]=0;
	}
	sync_timer_unlink(self->event);
	sync_timer_add(self->event, 40);
	self->current=0x00;
	return 0;
}



int module_init(Display8x7segMX *self)
{
	/*	init module	*/
	int i;

	GtkWidget *hbox;
	GtkWidget *select;

	watch_port(self, self->port_no);

	/*	init gui	*/
	hbox=gtk_hbox_new(FALSE, 0);

	select=v_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), self);
	gtk_box_pack_start(GTK_BOX(hbox), select, FALSE, FALSE, 0);

	for(i=0; i<8; i++) {
		self->ssegs[i]=seven_seg_new();
		gtk_box_pack_start(GTK_BOX(hbox), self->ssegs[i], FALSE, FALSE, 10);
	}

	self->event=timer_event_alloc(self, M_QUEUE(off_handler), NULL);
	if (self->event == NULL) {
		return 1;
	}
	sync_timer_add(self->event, 40);
	
	gtk_widget_show_all(hbox);
	self->window=gui_add(hbox, self, "8x7seg panel");
	return 0;
}
int module_exit(Display8x7segMX *self, const char *str)
{
	printf("[4x7seg:%d]\texiting because of %s\n", self->id, str);
	unwatch_port(self, self->port_no);
	gui_remove(self->window);
	return 0;
}

module_info_t modules[]=
{
	{ "8x7seg panel",
		M_SPACE_SIZE	(sizeof(Display8x7segMX)),
		M_INIT		(module_init),
		M_EXIT		(module_exit),
		M_PORT_CHANGED	(module_read),
		M_RESET		(module_reset),
	},
	{ NULL }
};

