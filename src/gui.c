#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <module.h>
#include <gui.h>
#include <emux51.h>
#include <hex.h>

GtkWidget *window;
GtkWidget *mbox;

GtkWidget *file_hbox;
GtkWidget *file_label;
GtkWidget *file_button;
GtkWidget *file_reload;
GtkWidget *file_dialog;

GtkWidget *run_button;

GtkWidget *dump_window;
GtkWidget *dump_vbox;
GtkWidget *dump_body;


GtkWidget *mod_load_button;


/*	16 lines * 80 bytes per line	*/
char dumped_text[80*16];

char last_module_dir[PATH_MAX];
char last_hexfile_dir[PATH_MAX];

static void gui_set_stop(void)
{
	running=0;
	gtk_button_set_label(GTK_BUTTON(run_button), "Run");
}
static void gui_set_run(void)
{
	running=1;
	gtk_button_set_label(GTK_BUTTON(run_button), "Pause");
}


/*			main window			*/

static gboolean gui_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (widget == window)
		return FALSE;
	else if (widget == dump_window)
		return TRUE;
	printf("wtf?\n");
	return TRUE;
}

static void mw_destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}


/*			file load			*/

static void set_file_label(const char *str)
{
#if 1
	int len;
	char buff[FILE_LABEL_LEN+4];

	len=strlen(str);
	if (len > FILE_LABEL_LEN) {
		str+=len-FILE_LABEL_LEN;
		sprintf(buff, "...%s", str);
	} else {
		sprintf(buff, "%s", str);
	}
	gtk_label_set_text(GTK_LABEL(file_label), buff);
#else
	char buff[PATH_MAX+10];
	sprintf(buff, "[emux51]\t%s", str);
	gtk_window_set_title(GTK_WINDOW(window), buff);
#endif
}




static void file_load(void * data)
{
	int rval;
	char *fname;

	gui_set_stop();

	file_dialog=gtk_file_chooser_dialog_new("Select file", NULL,
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(file_dialog), GTK_RESPONSE_OK);

	rval=gtk_dialog_run(GTK_DIALOG(file_dialog));
	if (rval == GTK_RESPONSE_OK) {
		fname=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_dialog));
		if (load_hex(fname, code_memory, CODE_LENGHT) == 0) {
			strncpy(hexfile, fname, PATH_MAX);
			set_file_label(fname);
			do_reset();
			refresh_dump();
			loaded=1;
		} else {
			loaded=0;
			*hexfile=0;
			set_file_label("No file loaded");
		}
	}
	gtk_widget_destroy(file_dialog);
}


static void reload(void *data)
{
	if (loaded == 0)
		return;

	gui_set_stop();

	printf("file name: %s\n", hexfile);
	if (load_hex(hexfile, code_memory, CODE_LENGHT) == 0) {
		do_reset();
		refresh_dump();
		loaded=1;
	} else {
		set_file_label("No file loaded");
		loaded=0;
		*hexfile='\0';
	}
	
}



/*	handler of run/pause button	*/
static void run_pause_hndl(void *data)
{
	if (!loaded) {
		running=0;
		return;
	}
	if (running == 0) {
		gui_set_run();
	} else {
		gui_set_stop();
	}
}


static void gui_mod_ld(void *data)
{
	GtkWidget *dialog;
	char *fname;
	int rval;

	dialog=gtk_file_chooser_dialog_new("Select module", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	rval=gtk_dialog_run(GTK_DIALOG(dialog));
	if (rval == GTK_RESPONSE_OK) {
		fname=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		module_new(fname);
	}
	gtk_widget_destroy(dialog);

}


int gui_run(int *argc, char **argv[])
{
	gtk_init(argc, argv);
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), " Welcome in Emux51");
	g_signal_connect(window, "delete_event",
			G_CALLBACK(gui_delete_event), NULL);
	g_signal_connect(window, "destroy",
			G_CALLBACK(mw_destroy), NULL);


	data_dump(dumped_text);

	mbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), mbox);
	gtk_container_set_border_width(GTK_CONTAINER(window), 2);
	/*	file	*/
	file_label=gtk_label_new("No file loaded.");

	gtk_box_pack_start(GTK_BOX(mbox), file_label, TRUE, TRUE, 0);

	file_hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mbox), file_hbox, TRUE, TRUE, 0);

	file_button=gtk_button_new_with_label("Load");
	gtk_box_pack_start(GTK_BOX(file_hbox), file_button, TRUE, FALSE, 0);

	file_reload=gtk_button_new_with_label("Reload");
	gtk_box_pack_start(GTK_BOX(file_hbox), file_reload, TRUE, FALSE, 0);

	g_signal_connect(file_button, "clicked", G_CALLBACK(file_load), NULL);
	g_signal_connect(file_reload, "clicked", G_CALLBACK(reload), NULL);

	/*	run	*/
	run_button=gtk_button_new_with_label("Run");
	gtk_box_pack_start(GTK_BOX(mbox), run_button, TRUE, TRUE, 0);
	g_signal_connect(run_button, "clicked", G_CALLBACK(run_pause_hndl), NULL);

	/*	module load	*/
	mod_load_button=gtk_button_new_with_label("Load module");
	gtk_box_pack_start(GTK_BOX(mbox), mod_load_button, TRUE, TRUE, 0);
	g_signal_connect(mod_load_button, "clicked", G_CALLBACK(gui_mod_ld), NULL);
		
	/*	dump	*/
	dump_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dump_window), "Memory dump window");
	g_signal_connect(dump_window, "delete_event",
			G_CALLBACK(gui_delete_event), NULL);

	dump_vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(dump_window), dump_vbox);

	dump_body=gtk_label_new(dumped_text);
	gtk_box_pack_start(GTK_BOX(dump_vbox), dump_body, TRUE, TRUE, 0);

	gtk_widget_show_all(window);
	gtk_widget_show_all(dump_window);

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return 0;
}


void refresh_dump(void)
{
	data_dump(dumped_text);
	gtk_label_set_text(GTK_LABEL(dump_body), dumped_text);
}

int gui_counter=0;
void gui_callback(void)
{
/*	gdk_threads_enter();*/
	if (gui_counter == 10) {
		gui_counter=0;
		refresh_dump();
	}
	gui_counter++;
/*	gdk_threads_leave();*/
}


static gboolean 
gui_module_delete_event(GtkWidget *widget, GdkEvent *event, void * data)
{
	module_destroy(data, "You were killed dude");
	printf("killed\n");

	return FALSE;
}

int gui_add(void *object, void *module)
{
	GtkWidget *window;

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "module window\n");
	gtk_container_add(GTK_CONTAINER(window), object);
	g_signal_connect(window, "delete-event",
			G_CALLBACK(gui_module_delete_event), module);

/*	gtk_widget_show_all(window);*/
	gtk_widget_show(window);

	return 0;
}


