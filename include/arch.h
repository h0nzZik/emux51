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
void *load_lib(const char *path);
void close_lib(void *lib);
void *load_sym(void *lib, const char *name);

#endif
