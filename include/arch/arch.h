#ifndef ARCH_H
#define ARCH_H

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif


int setup_timer(float freq, void (*callback)(void));

#endif
