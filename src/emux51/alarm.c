/*
 * emux51.c - Alarm integration into overall scheme.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <arch.h>
#include <alarm.h>



int pipefd[2];
GIOChannel *read_end;
GIOChannel *write_end;

void (*user_func)()=NULL;

/*	timer event handler	*/
static void handler(void)
{
	char data='x';
	write(pipefd[1], &data, sizeof(data));
}

static gboolean
watch(GIOChannel *channel, GIOCondition condition, gpointer data)
{
	GError *err;
	char b;
	gsize cnt;

	err=NULL;
	g_io_channel_read_chars(channel, &b, sizeof(b), &cnt, &err);

	if(user_func)
		user_func();
	return TRUE;
}


int set_timer(float freq, void (*callback)(void))
{
	/*	create pipe	*/
	if (arch_create_pipe(pipefd)){
		printf("cannot create pipe\n");
		exit (1);
	}
	/*	wrap  pipe into the glib IO channels	*/
	read_end=g_io_channel_unix_new(pipefd[0]);

	g_io_channel_set_encoding(read_end, NULL, NULL);
	g_io_channel_set_buffered(read_end, FALSE);
	/*	add watch to read end of pipe	*/
	g_io_add_watch(read_end, G_IO_IN, watch, NULL);

	/*	set up timer	*/
	setup_timer(freq, handler);
	user_func=callback;
	return 0;

}
