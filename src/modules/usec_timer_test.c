#include <stdio.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <module.h>




typedef struct {
	int id;
	unsigned count;
	GtkWidget *window;
} instance_t;


void muj_test(instance_t *self, void *data)
{
	if (self->count == 0) {
		printf("first\n");
	}
#if 0
	self->count++;
	if (self->count == 1000) {
		printf("\ndone\n");
	} else {
		usec_timer_add(self, 1000, M_QUEUE(muj_test), NULL);
	}
#endif

}


void *module_init(instance_t *self)
{
	GtkWidget *vbox;
	GtkWidget *label;

	printf("hello world\n");

	vbox=gtk_vbox_new(FALSE, 0);
	label=gtk_label_new("Hello, world!");

	gtk_container_set_border_width(GTK_CONTAINER(vbox), 2);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);

	/*	create window and add 'vbox' into it	*/
	self->window=gui_add(vbox, self, "Timer test");

	usec_timer_add(self, 1000000, M_QUEUE(muj_test), NULL);

	return 0;
}

void module_exit(instance_t *self, char *reason)
{
	printf("[test:%d]\texiting: %s.\n",self->id, reason);

	/*	remove our window	*/
	gui_remove(self->window);
}



module_info_t module_info={
	"usec timer test",
	M_SPACE_SIZE	(sizeof(instance_t)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	M_PORT_CHANGED	(NULL),
};
