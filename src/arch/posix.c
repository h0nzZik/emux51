#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

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
