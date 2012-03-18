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
	char lighting_segments[3];
	int history[3];
	GtkWidget *window;
	GtkWidget *ssegs[3];
	int mask;
	void *event;
} instance;




void off_handler(instance *self, void *data)
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
void import_segments(instance *self)
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

void module_read(instance *self, int port)
{
	if (port != self->port_no) {
		return;
	}
	import_segments(self);
}

/*	handler of "port-select" signal	*/
static void port_select(PortSelector *ps, instance *self)
{
	int i;
	self->port_no=port_selector_get_port(ps);
	printf("port select %d\n", self->port_no);
	memset(self->lighting_segments, 0, sizeof(self->lighting_segments));

	/*	it was turned off	*/
	for(i=0; i<3; i++){
		seven_seg_set_segments(self->ssegs[i], 0);
	}
	self->mask=0xFF;
	/*	redraw	*/
	import_segments(self);
}

int module_init(instance *self)
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
//	gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
	for(i=0; i<3; i++) {
		self->ssegs[i]=seven_seg_new();
//		tmpbox=gtk_vbox_new(FALSE,0);
//		gtk_container_set_border_width(GTK_CONTAINER(tmpbox), 10);
		gtk_box_pack_start(GTK_BOX(hbox), self->ssegs[i], FALSE, FALSE, 0);
//		gtk_box_pack_start(GTK_BOX(hbox), tmpbox, FALSE, FALSE, 0);
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
int module_exit(instance *self, const char *str)
{
	printf("[3x7seg:%d]\texiting because of %s\n", self->id, str);
	sync_timer_unlink(self->event);
	g_free(self->event);
	gui_remove(self->window);
	return 0;
}

module_info_t module_info={
	"3x7seg panel",
	M_SPACE_SIZE	(sizeof(instance)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	M_PORT_CHANGED	(module_read),
};

