#ifndef MODULE_H
#define MODULE_H

#include <arch/arch.h>
int modules_init(void);

typedef struct {
	unsigned  int id;
	unsigned long int bits;
	struct sockaddr_in addr;
} module_t;


#endif
