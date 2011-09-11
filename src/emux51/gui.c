/*
 * gui.c - simple graphical interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <locale.h>

#include <module.h>
#include <settings.h>
#include <gui.h>
#include <emux51.h>
#include <hex.h>

GtkWidget *window;
GtkWidget *mbox;

GtkWidget *file_frame;
GtkWidget *file_vbox;
GtkWidget *file_hbox;
GtkWidget *file_label;
GtkWidget *file_tooltip;
GtkWidget *file_button;
GtkWidget *file_reload;


GtkWidget *run_frame;
GtkWidget *run_vbox;
GtkWidget *freq_label;
GtkWidget *run_hbox;
GtkWidget *run_button;
GtkWidget *freq_button;

GtkWidget *dump_window;
GtkWidget *dump_vbox;
GtkWidget *dump_body;

GtkWidget *mod_load_button;

/*	menu stuff	*/
GtkItemFactory *itf;
GtkWidget *itf_widget;
GtkAccelGroup *accel_group;
/*	this will be set after first emiting 'delete_event' to dump_window	*/
GtkWidget *view_dump_menu_button=NULL;


/*	16 lines * 80 bytes per line	*/
char dumped_text[80*16];
int dump_visible=1;

char last_module_dir[PATH_MAX];
char last_hexfile_dir[PATH_MAX];

static void gui_set_stop(void)
{
	stop();
	printf("stopped\n");
	gtk_button_set_label(GTK_BUTTON(run_button), "Run");
}
static void gui_set_run(void)
{
	start();
	gtk_button_set_label(GTK_BUTTON(run_button), "Pause");
}


static gboolean gui_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (widget == window)
		return FALSE;
	printf("wtf?\n");
	return TRUE;
}

static void mw_destroy(GtkWidget *widget, gpointer data)
{
	printf("destroying all..\n");
	module_destroy_all("clicked");
	gtk_main_quit();
}


/*			file load			*/
static void set_file_label(const char *str)
{
	char buff[20];

	strncpy(buff, str, 20);
	buff[19]=0;
	gtk_widget_set_tooltip_text(GTK_WIDGET(file_label), str);
	gtk_label_set_text(GTK_LABEL(file_label), buff);
	gtk_window_set_title(GTK_WINDOW(window), str);
}



/*	create 'Load HEX' dialog	*/
static void file_load(void *data)
{
	int rval;
	char *fname;
	char *dirname;
	char *newdir;
	GtkWidget *file_dialog;
	GtkFileFilter *filter;


	gui_set_stop();
	/*	create dialog	*/
	file_dialog=gtk_file_chooser_dialog_new("Select file", NULL,
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(
				GTK_DIALOG(file_dialog), GTK_RESPONSE_OK);

	/*	create 'HEX' filter	*/
	filter=gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter), "Intel HEX files");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Hh][Ee][Xx]");
	gtk_file_chooser_add_filter(
			GTK_FILE_CHOOSER(file_dialog), GTK_FILE_FILTER(filter));

	/*	create 'All Files' filter	*/
	filter=gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter), "All files");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*");
	gtk_file_chooser_add_filter(
			GTK_FILE_CHOOSER(file_dialog), GTK_FILE_FILTER(filter));

	/*	get HEX directory	*/
	dirname=getenv("hex_dir");
	if(dirname) {
		gtk_file_chooser_set_current_folder(
			GTK_FILE_CHOOSER(file_dialog), dirname);
	}

	rval=gtk_dialog_run(GTK_DIALOG(file_dialog));
	if (rval == GTK_RESPONSE_OK) {
		fname=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_dialog));
		newdir=gtk_file_chooser_get_current_folder
			(GTK_FILE_CHOOSER(file_dialog));
		if (load_hex(fname, code_memory, CODE_LENGHT) == 0) {
			strncpy(hexfile, fname, PATH_MAX);
			/*	set only file name as label	*/
			set_file_label(fname+strlen(newdir)+1);
			do_reset();
			refresh_dump();
			loaded=1;

			/*	save settings	*/
			if (dirname == NULL || strcmp(newdir, dirname)) {
				g_setenv("hex_dir", newdir, 1);
				config_save();
			}
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
		gui_set_stop();
		return;
	}
	if (running == 0) {
		gui_set_run();
	} else {
		gui_set_stop();
	}
}

/*	create 'Load module' dialog	*/
static void gui_mod_ld(void *data)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	char *fname;
	int rval;
	char *dir;
	char *newdir;
	char buff[20];

	dialog=gtk_file_chooser_dialog_new("Select module", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	/*	create .dll or .so filter	*/
	sprintf(buff, "*%s", MODULE_EXTENSION);
	filter=gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter), "Dynamic libraries");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), buff);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(filter));
	/*	create all files filter	*/
	filter=gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter), "All files");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(filter));

	
	/*	try to set	*/
	dir=getenv("module_dir");
	if (dir){
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dir);
	}

	rval=gtk_dialog_run(GTK_DIALOG(dialog));
	if (rval == GTK_RESPONSE_OK) {
		fname=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		newdir=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
		if(module_new(fname)) {
			printf("can't load module\n");
		} 
		if (dir == NULL || strcmp(newdir, dir)) {
			g_setenv("module_dir", newdir, 1);
			config_save();
		}
	}
	gtk_widget_destroy(dialog);
}

char *format_frequency(char *buff, unsigned long freq)
{
	if (freq < 2*1000) {
		sprintf(buff, "%ld Hz", freq);
	} else if (freq < 2*1000*1000) {
		if (freq%1000 == 0)
			sprintf(buff, "%ld KHz", freq/1000);			
		else
			sprintf(buff, "%.3f KHz", (float)freq/1000);
	} else {
		if (freq%1000000 == 0)
			sprintf(buff, "%ld MHz", freq/1000000);			
		else
			sprintf(buff, "%.6f MHz", (float)freq/1000000);
	}
	return buff;
}

unsigned long parse_frequency(const char *str)
{
	float freq=0;

	if (sscanf(str, "%f", &freq) != 1)
		return 0;
	while (*str) {
		if (isalpha(*str)) {
			if(!strcmp(str, "kHz") || !strcmp(str, "KHz"))
				return (1000*freq);
			if(!strcmp(str, "MHz"))
				return (1000*1000*freq);
			return (unsigned long)freq;
		}
		str++;
	}
	return (unsigned long)freq;
	
}

static void set_frequency(unsigned long freq)
{
	char buff[40];
	int state;
	state=g_atomic_int_get(&running);
	if (state)
		pause();
	Fosc=freq;
	format_frequency(buff, Fosc);
	gtk_label_set_text(GTK_LABEL(freq_label), buff);
	if (state)
		start();
}

/*	create set frequency dialog	*/
static void change_frequency(GtkWidget *button, void *data)
{

	GtkWidget *dialog;
	GtkWidget *entry;
	GtkWidget *hbox;
	GtkWidget *label;
	int response;
	char buff[40];
	const char *freq;
	unsigned long f;

	dialog=gtk_dialog_new_with_buttons("Set frequency",
					GTK_WINDOW(window),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_OK,
					GTK_RESPONSE_OK,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL,
					NULL);
	hbox=gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0);
	label=gtk_label_new("Fosc:");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	entry=gtk_entry_new();
	format_frequency(buff, Fosc);
	gtk_entry_set_text(GTK_ENTRY(entry), buff);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);
	response=gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_OK){
		freq=gtk_entry_get_text(GTK_ENTRY(entry));
		f=parse_frequency(freq);
		if (f) {
			set_frequency(f);
		}
	}
	gtk_widget_destroy(dialog);

}


static void dump_view(gpointer data, guint action, GtkWidget *button)
{
	int active;

	view_dump_menu_button=button;

	active=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(button));

	if (active)
		gtk_widget_show(dump_window);
	else
		gtk_widget_hide(dump_window);

	dump_visible=active;	
}

static gboolean dump_destroy(GtkWidget *dwin, gpointer data)
{
	gtk_widget_hide(dwin);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_dump_menu_button), 0);
	return TRUE;
}


/*		MENU		*/
static GtkItemFactoryEntry items[]={
{ "/File", NULL, NULL, 0, "<Branch>" },
{ "/File/_Open HEX","<control>O", file_load, 0, "<StockItem>", GTK_STOCK_OPEN},
{ "/File/_Load Module", "<CTRL>L", gui_mod_ld, 0, "<StockItem>", GTK_STOCK_OPEN},
{ "/File/_Quit", "<CTRL>Q", mw_destroy,	0, "<StockItem>", GTK_STOCK_QUIT},
{ "/View", NULL, NULL, 0, "<Branch>"},
{ "/View/_Dump", NULL, dump_view, 0, "<CheckItem>"}
};
int nitems=sizeof(items)/sizeof(items[0]);

int gui_run(int *argc, char **argv[])
{
	gtk_init(argc, argv);


	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "POSIX");

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), " Welcome to Emux51");
	g_signal_connect(window, "delete_event",
			G_CALLBACK(gui_delete_event), NULL);
	g_signal_connect(window, "destroy",
			G_CALLBACK(mw_destroy), NULL);

	data_dump(dumped_text);

	mbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), mbox);
	gtk_container_set_border_width(GTK_CONTAINER(window), 3);

	/*	menu	*/
	accel_group=gtk_accel_group_new();
	itf=gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
	gtk_item_factory_create_items(itf, nitems, items, NULL);
	itf_widget=gtk_item_factory_get_widget(itf, "<main>");
	gtk_box_pack_start(GTK_BOX(mbox), itf_widget, TRUE, TRUE, 0);

	/*	file	*/

	file_frame=gtk_frame_new("");
	gtk_box_pack_start(GTK_BOX(mbox), file_frame, FALSE, FALSE, 0);

	file_vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(file_frame), file_vbox);
	gtk_container_set_border_width(GTK_CONTAINER(file_vbox), 2);

	file_label=gtk_label_new("");
	set_file_label("No file loaded.");
	gtk_box_pack_start(GTK_BOX(file_vbox), file_label, TRUE, TRUE, 0);

	file_hbox=gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(file_vbox), file_hbox, TRUE, TRUE, 0);
//	gtk_container_set_border_width(GTK_CONTAINER(file_vbox), 4);

	file_button=gtk_button_new_with_label("Load");
	gtk_box_pack_start(GTK_BOX(file_hbox), file_button, TRUE, TRUE, 0);

	file_reload=gtk_button_new_with_label("Reload");
	gtk_box_pack_start(GTK_BOX(file_hbox), file_reload, TRUE, TRUE, 0);

	g_signal_connect(file_button, "clicked", G_CALLBACK(file_load), NULL);
	g_signal_connect(file_reload, "clicked", G_CALLBACK(reload), NULL);

	/*	run	*/
	run_frame=gtk_frame_new("");
	gtk_box_pack_start(GTK_BOX(mbox), run_frame, FALSE, FALSE, 0);

	run_vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(run_frame), run_vbox);

	freq_label=gtk_label_new("");
	set_frequency(12000000);
	gtk_box_pack_start(GTK_BOX(run_vbox), freq_label, TRUE, TRUE, 0);


	run_hbox=gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(run_vbox), run_hbox, FALSE, FALSE, 0);

	run_button=gtk_button_new_with_label("Run");
	gtk_box_pack_start(GTK_BOX(run_hbox), run_button, TRUE, TRUE, 0);
	g_signal_connect(run_button, "clicked", G_CALLBACK(run_pause_hndl), NULL);

	freq_button=gtk_button_new_with_label("Frequency");
	gtk_box_pack_start(GTK_BOX(run_hbox), freq_button, TRUE, TRUE, 0);
	g_signal_connect(freq_button, "clicked", G_CALLBACK(change_frequency), NULL);
	

	/*	module load	*/
	mod_load_button=gtk_button_new_with_label("Load module");
	gtk_box_pack_start(GTK_BOX(mbox), mod_load_button, TRUE, TRUE, 0);
	g_signal_connect(mod_load_button, "clicked", G_CALLBACK(gui_mod_ld), NULL);
		
	/*	dump	*/
	dump_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dump_window), "Memory dump window");
	g_signal_connect(dump_window, "delete_event",
			G_CALLBACK(dump_destroy), NULL);

	dump_vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(dump_window), dump_vbox);

	dump_body=gtk_label_new(dumped_text);
	gtk_box_pack_start(GTK_BOX(dump_vbox), dump_body, TRUE, TRUE, 0);



	/*	keyboard shortcuts for both windows	*/
	gtk_window_add_accel_group(GTK_WINDOW(dump_window), accel_group);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_resizable(GTK_WINDOW(dump_window), FALSE);

	/*	and show	*/
	gtk_widget_show_all(window);
	gtk_widget_show_all(dump_vbox);

	gtk_main();
	return 0;
}


void refresh_dump(void)
{	if (dump_visible) {
		data_dump(dumped_text);
		gtk_label_set_text(GTK_LABEL(dump_body), dumped_text);
	}
}

int gui_counter=0;
void gui_callback(void)
{
	if (gui_counter == 5) {
		gui_counter=0;
		refresh_dump();
	}
	gui_counter++;
}


static gboolean 
gui_module_delete_event(GtkWidget *widget, GdkEvent *event, void * data)
{
	module_destroy(data, "delete-event");

//	return FALSE;
	return TRUE;
}

/*	add module into GUI scheme	*/
void * gui_add(void *object, void *delete_data, const char *title)
{
	GtkWidget *window;

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_container_add(GTK_CONTAINER(window), object);
	g_signal_connect(window, "delete-event",
			G_CALLBACK(gui_module_delete_event), delete_data);

	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_widget_show(window);

	return window;
}

void gui_remove(void *window)
{
	gtk_widget_destroy(window);
}


