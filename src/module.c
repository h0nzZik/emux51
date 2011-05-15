#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>

#include <emux51.h>
#include <module.h>
#include <arch/arch.h>

#define MODULES_MAX 32

module_t modules[MODULES_MAX];
unsigned int module_cnt;

pthread_t module_thread_id;

unsigned long used_bits;


/*	protocol:
 *	0xid	action	0xdata
 *	actions:register, read, write, unregister
 */



/*	TODO:	*/

int module_register(unsigned int req_bits, struct sockaddr_in addr)
{
	unsigned long int id;
	int i;
	int empty;

	printf("registering mask 0x%x\n", req_bits);
	printf("port == %u\n", ntohs(addr.sin_port));

	if (module_cnt == MODULES_MAX)
		return -1;
	if (used_bits&req_bits)
		return -2;

	for (i=0; i<MODULES_MAX; i++) {
		if (modules[i].id == 0)
			break;
	}
	if (i >= MODULES_MAX) {
		/*	this would never happen	*/
		fprintf(stderr, "%s: %d: wtf?\n", __FILE__, __LINE__);
		return -3;
	}
	empty=i;

	do {
		id=1+rand();
		for(i=0; i<MODULES_MAX; i++){
			if (modules[i].id == id){
				id=0;
				break;
			}
		}
	}while(!id);

	module_cnt++;
	modules[empty].id=id;
	modules[empty].bits=req_bits;
	modules[empty].addr=addr;

	used_bits|=req_bits;
	

	return 0;
}

int module_unregister(unsigned int id)
{
	int i;
	for (i=0; i<module_cnt; i++){
		if(modules[i].id == id) {
			used_bits&=~modules[i].bits;
			memset(&modules[i], 0, sizeof(module_t));
			return 0;
		}
	}
	fprintf(stderr, "cannot unregister module '%x'\n", modules[i].id);
	return -1;
}
void *module_thread(void *parameter)
{
	int s;
	size_t b;
	socklen_t len;
	struct sockaddr_in host, me;

	char buff[80];
	char cmd[20];
	unsigned int id;
	char action;
	unsigned int data;
/*
	sscanf("12 ble 36", "%x %s %x", &id, command, &data);
	printf("command: %s\n", command);
*/
	printf("sizeof(buff)== %d\n", sizeof(buff));

	signal(SIGALRM, SIG_IGN);
	len=sizeof(host);
	memset(&host, 0, len);
	memset(&me, 0, len);

	s=socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return NULL;
	}
	me.sin_family=AF_INET;
	me.sin_addr.s_addr=INADDR_ANY;
	me.sin_port=htons(8000);

	if (0 > bind(s, (struct sockaddr *)&me, sizeof(struct sockaddr))){
		perror("bind");
		return NULL;
	}

	while (1) {
		b=recvfrom(s, &buff, sizeof(buff), 0,
			(struct sockaddr *)&host, &len);
		if (b == -1) {
			perror("recvfrom");
			return NULL;
		}
		/*	in connection-less protocol, this cannot occur	*/
		if (b == 0) {
			fprintf(stderr, "I though this cannot occcur\n");
			continue;
		}
		if (sscanf(buff, "%x %s %x", &id, cmd, &data) != 3) {
			fprintf(stderr, "invalid message\n");
			continue;
		}

		if (id == 0) {
			if (!strcmp(cmd, "register")){
				module_register(data, host);
			}
			else if (!strcmp(cmd, "read")){
				
			}
		}


		
	};



	
	return NULL;
}



int modules_init(void)
{
	int rval;
#ifdef WIN32
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data)) {
		fprintf(stderr, "WSAStartup\n");
		return -1;
	}
#endif
/*	broadcast_send();*/
	srand(time(NULL));
	memset(modules, 0, MODULES_MAX*sizeof(module_t));
	rval=pthread_create(&module_thread_id, NULL, module_thread, NULL);
	if (rval){
		perror("pthread_create");
		return -1;
	}
	while (1)
		getchar();
	

	return 0;
}
