/*
 * arch/posx.c - POSIX architecture dependent code.
 */
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dlfcn.h>

void (*timer_callback)(void);

static void alarm_handler()
{
	timer_callback();
}

int setup_timer(float freq, void (*callback)(void))
{
	struct itimerval time_setting;
	struct timeval wait_time;

	timer_callback=callback;

	wait_time.tv_sec=1/freq;
	wait_time.tv_usec=(unsigned long int)(1000000/freq)%1000000;

	time_setting.it_interval=wait_time;
	time_setting.it_value=wait_time;

	signal(SIGALRM, alarm_handler);
	setitimer(ITIMER_REAL, &time_setting, NULL);

	return 0;	
}


void *load_lib(const char *path)
{
	void *lib;
	lib=dlopen(path, RTLD_LAZY|RTLD_LOCAL);
	return lib;
}

void *load_sym(void *lib, const char *name)
{
	void *sym;
	sym=dlsym(lib, name);	
	return sym;
}

void close_lib(void *lib)
{
	dlclose(lib);	
}

int arch_create_pipe(int pipefd[2])
{
	return(pipe(pipefd));
}
