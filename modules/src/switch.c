#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <module.h>


modid_t id;
emuf_t *f;


GtkWidget *main_box;
GtkWidget *button_box[4];
GtkWidget *check_buttons[4][8];
GtkWidget *labels[4];


GtkWidget *menubar;
GtkWidget *portmenu;

/*	menu	*/
GtkWidget *ports;
/*->*/	GtkWidget *menu_port_item[4];

char ext_state[4];


static void toggled(GtkToggleButton *button, gpointer data)
{
	int active;
	int port;
	int bit;

	port=((int)data)/8;
	bit=7-(((int)data)%8);

	active=gtk_toggle_button_get_active(button);
	if (active) {
		ext_state[port]|=1<<bit;
	} else {
		ext_state[port]&=~(1<<bit);
	}
	printf("on P%d is now 0x%x\n", ext_state[port]);
	f->write_port(id, port, ext_state[port]);
}

static void menu_toggled(GtkToggleButton *button, gpointer data)
{
	int active;
	int rval;
	int i;
	active=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(button));
	if (active){
		rval=f->alloc_bits(id, (int)data, 0xFF);
		if (rval){
			printf("[switch]\tcannot alloc port %d\n", (int)data);
			return;
		}
		ext_state[(int)data]=0xFF;
		for (i=0; i<8; i++) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(check_buttons[(int)data][i]), TRUE);
		}
		gtk_widget_show(button_box[(int)data]);
	} else {
		f->write_port(id, (int)data, 0xFF);
		f->free_bits(id, (int)data, 0xFF);
		gtk_widget_hide(button_box[(int)data]);
	}
}

void * module_init(modid_t modid, void *cbs)
{
	int i;
	int j;
	char buff[40];
	GtkWidget *button;

	id=modid;
	f=cbs;


	memset(ext_state, 4, 0xFF);

	/*	<gui>	*/


	main_box=gtk_vbox_new(FALSE, 0);

	/*	menu	*/
	menubar=gtk_menu_bar_new();
	portmenu=gtk_menu_new();

	ports=gtk_menu_item_new_with_label("Ports");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(ports), portmenu);

	for (i=0; i<4; i++) {
		sprintf(buff, "Port %d", i);
		menu_port_item[i]=gtk_check_menu_item_new_with_label(buff);
		gtk_menu_shell_append(GTK_MENU_SHELL(portmenu), menu_port_item[i]);
		g_signal_connect(G_OBJECT(menu_port_item[i]), "toggled",
				 G_CALLBACK(menu_toggled), (gpointer)i);
	}

	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), ports);
	gtk_box_pack_start(GTK_BOX(main_box), menubar, FALSE, FALSE, 2);


	/*	buttons	*/
	for(i=0; i<4; i++) {
		button_box[i]=gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(main_box), button_box[i], FALSE, FALSE, 0);
		sprintf(buff, "Port %d", i);
		labels[i]=gtk_label_new(buff);
		gtk_box_pack_start(GTK_BOX(button_box[i]), labels[i], FALSE, FALSE, 0);

		for(j=0; j<8; j++) {
			button=gtk_check_button_new();
			g_signal_connect(button, "toggled",
				G_CALLBACK(toggled), (gpointer)(8*i+j));
			check_buttons[i][j]=button;
			gtk_box_pack_start(GTK_BOX(button_box[i]), button,
				FALSE, FALSE, 0);
		}
	}

	gtk_widget_show_all(main_box);

/*	set default state	*/
	for (i=0; i<4; i++)
		gtk_widget_hide(button_box[i]);

	/*	</gui>		*/

	return main_box;
}

void module_exit(const char *str)
{
	int i;

	printf("called\n");
	for(i=0; i<4; i++) {
		f->write_port(id, i, 0xFF);
		f->free_bits(id, i, 0xFF);
	}
}
