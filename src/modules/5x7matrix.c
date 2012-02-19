#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <module.h>

#include <widgets/port_selector.h>
#include <widgets/led.h>


typedef struct {
	int id;

	int port;
	int column;
	char last_data;

	void *off_event;

	int invert;
	int history[5];
	GtkWidget *leds[5][7];
	GtkWidget *window;
} instance;



void column_set_off(instance *self, int column)
{
	int i;
	for (i=0; i<7; i++) {
		led_set_active(self->leds[column][i], 0);
	}
}


void off_handler(instance *self, void *data)
{
	int i;
	for(i=0; i<5; i++){
		if (i == self->column)
			continue;

		if (self->history[i])
			self->history[i]=0;
		else
			column_set_off(self, i);
	}
	sync_timer_add(self->off_event, 40);
}


void reset_handler(instance *self, void *data)
{
	

}

static void update(instance *self)
{
	char data;
	int i;

	read_port(self, self->port, &data);

	/*	raising edge on 7th bit	*/
	if (((data&0x80) != 0) && ((self->last_data&0x80) == 0)) {
//		printf("raising edge\n");
		self->column++;
		self->column%=5;
	}
	/*	falling edge on 7th bit	*/
	if (((data&0x80) == 0) && ((self->last_data&0x80) != 0)) {
//		printf("falling edge\n");
	}

	if (/*self->column != 0 && */self->column < 5) {
		self->history[self->column]=1;
		for (i=0; i<7; i++) {
			led_set_active(self->leds[(5+self->column-1)%5][i],
					((data>>i)&1)^self->invert);
		}
	}

	self->last_data=data;
}


static void port_select(PortSelector *ps, instance *self)
{
	self->port=port_selector_get_port(ps);

	printf("port %d\n", self->port);
}


void module_read(instance *self, int port)
{
	update(self);
}


int module_init(instance *self)
{
	int i,j;
	float c[3];
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *select;

	self->invert=1;


	vbox=gtk_vbox_new(FALSE, 0);


	select=h_port_selector_new();
	g_signal_connect(select, "port-select",
			G_CALLBACK(port_select), self);
	gtk_box_pack_start(GTK_BOX(vbox), select, FALSE, FALSE, 0);

	table=gtk_table_new(7, 5, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);


	/*	randomize colors	*/
	i=rand()%3;
	c[(i+0)%3]=(rand()%0x100)/(double)0x100;
	if (c[(i+0)%3]<0.7)
		c[(i+1)%3]=(0x80+(rand()%0x80))/(double)0x80;
	else
		c[(i+1)%3]=0;
	c[(i+2)%3]=0;


	for(i=0; i<7; i++) {
		for(j=0; j<5; j++) {
			self->leds[j][i]=led_new(12,2);
			gtk_table_attach_defaults(GTK_TABLE(table), self->leds[j][i], 
					j, j+1, i, i+1);
			led_set_rgb(self->leds[j][i], c[0], c[1], c[2]);
			led_set_active(self->leds[j][i], 1);
		}
	}

	self->off_event=timer_event_alloc(self, M_QUEUE(off_handler), NULL);
	if (self->off_event == NULL) {
		return 1;
	}
	sync_timer_add(self->off_event, 40);


	gtk_widget_show_all(vbox);
	self->window=gui_add(vbox, self, "5x7 matrix display");
	return 0;
}



void module_exit(instance *self, char *reason)
{
	int i,j;
	printf("[5x7:%d]\texiting: %s.\n",self->id, reason);
	for (i=0; i<7; i++) {
		for (j=0; j<5; j++)
			gtk_widget_destroy(self->leds[j][i]);
	}

	sync_timer_unlink(self->off_event);
	g_free(self->off_event);	

	gui_remove(self->window);
	
}


module_info_t module_info={
	"5x7 matrix display",
	M_SPACE_SIZE	(sizeof(instance)),
	M_INIT		(module_init),
	M_EXIT		(module_exit),
	M_PORT_CHANGED	(module_read),
};
