#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include <module.h>

#include <widgets/port_selector.h>
#include <widgets/7seg.h>



typedef struct {
	modid_t id;
	emuf_t *f;
	GtkWidget *vbox;

	unsigned char port_no;
	unsigned char lighting_segments[3];
	GtkWidget *ssegs[3];
} mod_instance_t;


/*	import and decoding	*/

/*	inverted decode array	*/
const unsigned char decode_arr[16]={0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02,
		0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};
void import_segments(mod_instance_t *in)
{
	unsigned char data;
	unsigned char value;
	unsigned char light;
	int i;

	data=in->f->read_port(in->id, in->port_no);
	value=data&0x0F;

	for (i=0; i<3; i++) {
		if (((data>>(4+i))&1) == 0) {
			light=~decode_arr[value];
			if (in->lighting_segments[i]!=light) {
				seven_seg_set_segments(in->ssegs[2-i], light);
			}
			in->lighting_segments[i]=light;
		}
	}
}

void module_read(mod_instance_t *in, int port)
{
	if (port != in->port_no)
		return;
	import_segments(in);
}
#if 0
void test_func(void)
{
	static int i=0;
	i++;


	if (f->time_queue_add(id, 1, test_func)){
		printf("error 'handling'");
		return;
	}

	if (i<50)
		return;
	i=0;
	printf("huraa\n");

}
#endif

/*	handler of "port-select" signal	*/
static void port_select(PortSelector *ps, gpointer data)
{
	mod_instance_t *in;
	int i;
	in=data;
	printf("[3x7]\tport select..\n");
	in->port_no=port_selector_get_port(ps);
	printf("port %d was selected\n", in->port_no);
	memset(in->lighting_segments, 0, sizeof(in->lighting_segments));

	for(i=0; i<3; i++)
		seven_seg_set_segments(in->ssegs[i], 0);
	import_segments(in);
	printf("[3x7]\tdone\n");
}

void *module_init(modid_t modid, emuf_t *funcs)
{
	/*	init module	*/
	int i;
	GtkWidget *hbox;
	GtkWidget *select;
	mod_instance_t *in;

	in=g_malloc0(sizeof(mod_instance_t));

	in->f=funcs;
	in->id=modid;
	if (in->f->handle_event(in->id, "read", module_read)){
		in->f->crash(in->id, "can't 'handle'");
	}

	/*	init gui	*/
	hbox=gtk_hbox_new(FALSE, 0);

	select=v_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), in);
	gtk_box_pack_start(GTK_BOX(hbox), select, FALSE, FALSE, 0);

	for(i=0; i<3; i++) {
		in->ssegs[i]=seven_seg_new();
		gtk_box_pack_start(GTK_BOX(hbox), in->ssegs[i], FALSE, FALSE, 10);
	}
	
	
	

	gtk_widget_show_all(hbox);
	in->f->set_space(in->id, in);
	in->f->set_name(in->id, "3x7seg");
	printf("ok\n");
	return hbox;
#if 0
	printf("...\n");
	if (in->f->time_queue_add(in->id, 1, test_func)){
		in-f->crash(in->id, "cann't time_queue_add'");
	}
#endif

}
void module_exit(mod_instance_t *in, const char *str)
{
	printf("[7seg:%p:%d]\texiting because of %s\n", in, in->id.id, str);
}
