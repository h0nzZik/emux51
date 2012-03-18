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
#include <gmodule.h>

//#include <locale.h>

#include <module.h>
#include <settings.h>
#include <gui.h>
#include <emux51.h>
#include <hex.h>

GtkWidget *about_dialog;
GtkWidget *window;
GtkWidget *menu_view_dump;
GtkWidget *dump_window;
GtkWidget *memory_dump_label;
GtkWidget *reg_dump_label;
GtkWidget *file_label;
GtkWidget *freq_label;
GtkWidget *run_pause_button;
GtkWidget *stop_button;

GtkWidget *int_textview;
GtkTextBuffer *int_textbuffer;


char last_module_dir[PATH_MAX];
char *hex_file;


G_MODULE_EXPORT void emux51_quit();
G_MODULE_EXPORT void emux51_stop();
G_MODULE_EXPORT void emux51_run_pause();
G_MODULE_EXPORT void dump_close();
G_MODULE_EXPORT void file_load_hex();
G_MODULE_EXPORT void file_reload_hex();
G_MODULE_EXPORT void file_load_module();
G_MODULE_EXPORT void view_dump_toggled();
G_MODULE_EXPORT void about_clicked();
G_MODULE_EXPORT void edit_change_frequency();




/*			file load			*/
static void set_file_label(const char *str)
{
/*	char buff[20];

	strncpy(buff, str, 20);
	buff[19]=0;
	gtk_label_set_text(GTK_LABEL(file_label), buff);
	gtk_window_set_title(GTK_WINDOW(window), str);
*/
	gtk_label_set_text(GTK_LABEL(file_label), str);
}

static int emux51_load_hex(char *file)
{
	char *s;

	emux51_stop();

	printf("[emux51] loading hex file: <%s>\n", file);

	if (load_hex(file, code_memory, CODE_LENGHT) == 0) {
		/*	OK	*/
		printf("[emux51]\tloaded hex file <%s>.\n", file);
		gtk_widget_set_sensitive(run_pause_button, TRUE);
		gtk_widget_set_sensitive(stop_button, TRUE);

		s=g_path_get_dirname(file);
		g_setenv("hex_dir", s, TRUE);
		config_save();
		free(s);
	

		s=g_path_get_basename(file);
		set_file_label(s);
		free(s);

		hex_file=file;

		return 0;	// OK
	}

	/*	error	*/
	printf("[emux51]\tcan't load hex file <%s>.\n",file);
	set_file_label("No file loaded.");
	gtk_widget_set_sensitive(run_pause_button, FALSE);
	gtk_widget_set_sensitive(stop_button, FALSE);

	g_free(file);
	hex_file=NULL;

	return -1;	// Error
	
	
}

/*	create 'Load HEX' dialog	*/
void file_load_hex(void *data)
{
	int rval;
	char *fname;
	char *dirname;
	GtkWidget *file_dialog;
	GtkFileFilter *filter;



	emux51_stop();

	if (hex_file){
		g_free(hex_file);
		hex_file=NULL;
	}

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
		fname=gtk_file_chooser_get_filename
			(GTK_FILE_CHOOSER(file_dialog));

		emux51_load_hex(fname);
	}
	gtk_widget_destroy(file_dialog);
}




void file_reload_hex(void *data)
{
	if (hex_file == 0)
		return;

	printf("[emux51]reloading <%s>\n", hex_file);
	emux51_load_hex(hex_file);
}


/*	create 'Load module' dialog	*/
void file_load_module(void *data)
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
		gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER(dialog), dir);
	}

	rval=gtk_dialog_run(GTK_DIALOG(dialog));
	if (rval == GTK_RESPONSE_OK) {
		fname=gtk_file_chooser_get_filename
			(GTK_FILE_CHOOSER(dialog));

		newdir=gtk_file_chooser_get_current_folder
			(GTK_FILE_CHOOSER(dialog));

		printf("[emux51]\tloading module <%s>\n", fname);
		if(module_new(fname)) {
			printf("[emux51]\tcan't load module\n");
		} else {
			printf("[emux51]\tmodule loaded.\n");
		}
		if (dir == NULL || strcmp(newdir, dir)) {
			g_setenv("module_dir", newdir, 1);
			config_save();
		}
	}
	gtk_widget_destroy(dialog);
}

/*	make string from frequency value	*/
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

/*	parse frequency value from string	*/
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
		program_pause();
	Fosc=freq;
	format_frequency(buff, Fosc);
	gtk_label_set_text(GTK_LABEL(freq_label), buff);
	if (state)
		program_start();
}

/*	create set frequency dialog	*/
void edit_change_frequency()
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

const char *dump_head=
"--\t00\t01\t02\t03\t04\t05\t06\t07\t08\t09\t0A\t0B\t0C\t0D\t0E\t0F\n";

static void data_dump(char *buffer)
{
	unsigned int i, j;

	sprintf(buffer,"%s", dump_head);
	buffer+=strlen(buffer);

	for (i=0; i<16; i++) {
		sprintf(buffer, "%X0\t", i);
		buffer+=3;
		for (j=0; j<16; j++) {
			sprintf(buffer, "%02X\t",
				data_memory[16*i+j]&0xFF);
			buffer+=3;
		}
		*buffer++='\n';
	}

}
static void reg_dump(char *buffer)
{
	sprintf(buffer, "Acc\t%02Xh\n"
			"B\t%02Xh\n"
			"PC\t%04Xh\n"
			"P0\t%02Xh\t\t"
			"P1\t%02Xh\t\t"
			"P2\t%02Xh\t\t"
			"P3\t%02Xh\n",
			sfr_memory[Acc]&0xFF,
			sfr_memory[B_reg]&0xFF,
			PC&0xFFFF,
			sfr_memory[0x80]&0xFF,
			sfr_memory[0x90]&0xFF,
			sfr_memory[0xA0]&0xFF,
			sfr_memory[0xB0]&0xFF
			);
}

void dump_close()
{
	gtk_check_menu_item_set_active(
		GTK_CHECK_MENU_ITEM(menu_view_dump), 0);
}

void refresh_dump(void)
{
	char buffer[2048];
	if (gtk_widget_get_visible(dump_window) == FALSE)
		return;
	
	memset(buffer,0,sizeof(buffer));
	data_dump(buffer);
	gtk_label_set_text(GTK_LABEL(memory_dump_label), buffer);
	memset(buffer,0,sizeof(buffer));
	reg_dump(buffer);
	gtk_label_set_text(GTK_LABEL(reg_dump_label), buffer);
	
}

void emux51_quit()
{
	printf("[emux51]\texiting\n");
	module_destroy_all("program exit");
	gtk_main_quit();
}

void emux51_run_pause()
{
	if (gtk_toggle_tool_button_get_active
		(GTK_TOGGLE_TOOL_BUTTON(run_pause_button)))
	{
		printf("[emux51]\tstarted\n");
		program_start();
	} else {
		printf("[emux51]\tpaused\n");
		program_pause();
	}
}

void emux51_stop()
{
	gtk_toggle_tool_button_set_active
		(GTK_TOGGLE_TOOL_BUTTON(run_pause_button), FALSE);
	program_stop();
	refresh_dump();
}

void about_clicked()
{
	gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_hide(about_dialog);
}

void view_dump_toggled(GtkWidget *button)
{
	if (gtk_check_menu_item_get_active
	   (GTK_CHECK_MENU_ITEM(button)) == TRUE) {
		gtk_widget_show(dump_window);
		refresh_dump();
	} else {
		gtk_widget_hide(dump_window);
	
	}
}

void int_log_append(const char *str)
{
	GtkTextMark *end;
	GtkTextIter *iter;
//	printf("appending: %s\n", str);

//	gtk_text_buffer_get_end_iter(int_textbuffer, &iter);
//	gtk_text_buffer_insert_at_cursor(int_textbuffer,str, -1);
//	gtk_text_buffer_insert(int_textbuffer, &iter,str,-1);


//	gtk_text_view_scroll_mark_onscreen(int_textview, end);

}

/*	starts gui	*/
int gui_run(int *argc, char **argv[])
{
	GtkBuilder *builder;
	GError *error=NULL;

	gtk_init(argc, argv);

	builder=gtk_builder_new();
	gtk_builder_add_from_file(builder,"emux.glade", &error);
	if (error){
		printf("[emux51]\tgtk builder error: %s\n", error->message);
		exit(1);
	}
	


	menu_view_dump	= GTK_WIDGET(gtk_builder_get_object
			(builder, "menu_view_dump"	));
	
	window		= GTK_WIDGET(gtk_builder_get_object
			(builder, "window"));
	
	dump_window	= GTK_WIDGET(gtk_builder_get_object
			(builder, "dump_window"		));
	
	about_dialog	= GTK_WIDGET(gtk_builder_get_object
			(builder, "aboutdialog"		));
	
	memory_dump_label=GTK_WIDGET(gtk_builder_get_object
			(builder, "memory_dump_label"	));
	
	reg_dump_label	= GTK_WIDGET(gtk_builder_get_object
			(builder, "reg_dump_label"	));
	
	file_label	= GTK_WIDGET(gtk_builder_get_object
			(builder, "file_label"		));
	
	freq_label	= GTK_WIDGET(gtk_builder_get_object
			(builder, "freq_label"		));

	run_pause_button= GTK_WIDGET(gtk_builder_get_object
			(builder, "tool_run_pause"	));

	stop_button	= GTK_WIDGET(gtk_builder_get_object
			(builder, "tool_stop"		));

	int_textview	= GTK_WIDGET(gtk_builder_get_object
			(builder, "interrupt_textview"));

	int_textbuffer	= gtk_builder_get_object
			(builder, "interrupt_textbuffer");	

	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);

	gtk_widget_show(window);
	printf("[emux51]\tentering to loop..\n");
	gtk_main();
	
	
	return 0;
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

/*	TODO: remove	*/
static gboolean 
gui_module_delete_event(GtkWidget *widget, GdkEvent *event, void * module)
{
	module_destroy(module, "delete-event");
	return TRUE;
}

/*	add module into GUI scheme	*/
/*	TODO: remove	*/
void * do_gui_add(void *object, void *module, const char *title)
{
	GtkWidget *window;

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_container_add(GTK_CONTAINER(window), object);
	g_signal_connect(window, "delete-event",
			G_CALLBACK(gui_module_delete_event), module);

	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_widget_show(window);

	return window;
}
/*	TODO: remove	*/
void do_gui_remove(void *window)
{
	gtk_widget_destroy(window);
}


