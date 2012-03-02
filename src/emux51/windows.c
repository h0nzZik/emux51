/*
 * arch/windows.c - WIN32 architecture dependent code.
 */
#include <fcntl.h>
#include <windows.h>
#include <winbase.h>
#include <io.h>


void (*timer_callback)(void);
static void CALLBACK alarm_handler(UINT id, UINT msg, DWORD user, DWORD dw1, DWORD dw2)
{
	timer_callback();

}
int setup_timer(float freq, void (*callback)(void))
{
	unsigned delay;
	unsigned tolerancy;
	void *rval;

/*		delay in miliseconds		*/
	delay=1000/freq;
/*		5% tolerancy			*/
	tolerancy=5*1000/freq/100;

	timer_callback=callback;

	rval=timeSetEvent(delay, tolerancy, alarm_handler, 0, TIME_PERIODIC);

	if (rval == NULL)
		return -1;
	return 0;

}
int arch_create_pipe(int pipefd[2])
{
	return(_pipe(pipefd, 256, O_TEXT));
}
