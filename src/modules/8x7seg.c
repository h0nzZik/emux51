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
	
} instance;




void off_handler(instance *self, void *data)
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
	time_queue_add(self, 100, M_QUEUE(off_handler), NULL);
}


/*	import and decoding	*/

/*	inverted decode array	*/
const char decode_arr[16]={0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02,
		0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};
void import_segments(instance *self)
{
	char data;
	char lighting;
	int select;

	read_port(self, self->port_no, &data);
	/*	suppression	*/
	if ((data>>7)&1)
		return;
	lighting=~decode_arr[data&0x0F];
	select=data>>4;
	self->history[select]=1;
	seven_seg_set_segments(self->ssegs[select], lighting);
	self->current=select;
}

void module_read(instance *self, int port)
{
	if (port != self->port_no)
		return;
	import_segments(self);
}

/*	handler of "port-select" signal	*/
static void port_select(PortSelector *ps, instance *self)
{
	int i;

	self->port_no=port_selector_get_port(ps);

	/*	it was turned off	*/
	for(i=0; i<8; i++){
		seven_seg_set_segments(self->ssegs[i], 0);
	}
	/*	redraw	*/
	import_segments(self);
}

int module_init(instance *self)
{
	/*	init module	*/
	int i;

	GtkWidget *hbox;
	GtkWidget *select;

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

	time_queue_add(self, 200, M_QUEUE(off_handler), NULL);
	
	gtk_widget_show_all(hbox);
	self->window=gui_add(hbox, self, "8x7seg panel");
	return 0;
}
int module_exit(instance *self, const char *str)
{
	printf("[4x7seg:%d]\texiting because of %s\n", self->id, str);
	gui_remove(self->window);
	return 0;
}

module_info_t module_info={
	"8x7seg panel",
	M_SPACE_SIZE	(sizeof(instance)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	M_PORT_CHANGED	(module_read),
};

