#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emux51.h>
#include <instructions.h>
#include <arch/arch.h>

#define MACHINE_FREQ_DEFAULT 1
#define SYNC_FREQ_DEFAULT 1



/*	64K  code memory		*/
unsigned char code_memory[CODE_LENGHT];
/*	256B data memory	*/
unsigned char data_memory[DATA_LENGHT];
unsigned short PC;

unsigned char *SP=&data_memory[0x81];
unsigned char *PSW=&data_memory[0xD0];

unsigned char *Acc=&data_memory[0xE0];

unsigned char *TCON=&data_memory[0x88];
unsigned char *TMOD=&data_memory[0x89];
unsigned char *TL0=&data_memory[0x8A];
unsigned char *TL1=&data_memory[0x8B];
unsigned char *TH0=&data_memory[0x8C];
unsigned char *TH1=&data_memory[0x8D];
unsigned short *DPTR=(void *)(&data_memory[0x82]);


unsigned char *P0=&data_memory[0x80];
unsigned char *P1=&data_memory[0x90];
unsigned char *P2=&data_memory[0xA0];
unsigned char *P3=&data_memory[0xB0];



unsigned char port_latches[4];
unsigned char port_collectors[4];
unsigned char P3_falling=0;






opcode_t opcodes[256];



/*	try unsigned :P	*/
/*signed int pending;*/
double machine_freq;
double sync_freq;


/*	TODO: checksum, segments,...	*/

int load_hex(const char *file, unsigned char *dest, unsigned int dest_len)
{
	FILE *fr;
	char buff[80];

	int ok;
	unsigned size;
	unsigned offset;
	unsigned type;
	unsigned segment=0;

	int i;
	unsigned int data;
	unsigned int sum=0;

	/*	source and destination index	*/


	if (!(file && dest))
		return -1;

	fr=fopen(file, "rt");
	if (fr == NULL)
		return -1;

	while (1 == fscanf(fr, "%s", buff)){
#if 0		/*	checksum	*/
		sum=0;
		for (i=0; i<80 && buff[i]; i++){
			sscanf(buff+2*i, "%2x", &data);
			sum+=data;
		}
		if (sum%256) {
			fprintf(stderr, "checksum error\n");
		}
#endif


		if (buff[0] != ':') {
			fprintf(stderr, "bad format\n");
			return -1;
		}

		ok=sscanf(buff, ":%2x%4x%2x", &size, &offset, &type);
		if (ok != 3) {
			fprintf(stderr, "bad format\n");
			return -1;
		}

		switch(type){

			case 0:
			sum=0;
			if (16*segment+offset+size >= dest_len){
				fprintf(stderr, "out of bounds\n");
				return -1;
			}
			for (i=0; i<size; i++) {
/*				data=16*(buff[9+2*i])+buff[9+2*i+1];*/
				sscanf(buff+9+2*i, "%2x", &data);
				sum+=data;
				dest[16*segment+offset+i]=data;


			}


			break;
			case 1:
				return 0;
			default:
				return 3;

		}



	}
	return 2;

}


void dump(void)
{
	int i;
	for(i=0; i<256; i++){
		if (i%16 == 0)
			printf("\n");
		printf("%2x", data_memory[i]);
	}
	printf("\nPC == %x\n", PC);
	printf("\n*SP == %x\n", *SP);

}



void init_machine(void)
{
	memset(code_memory, 0, CODE_LENGHT);
	memset(data_memory, 0, DATA_LENGHT);
	memset (port_latches, 0xFF, 4);
	memset (port_collectors, 0xFF, 4);
	*P0=*P1=*P2=*P3=0xFF;
	*SP=7;
	PC=0;

}



/*	TODO:	external, second timer	*/


#define timer_0_runs (*TCON&0x10 && (!(*TMOD&0x08) || (*P3&0x04)))
#define timer_1_runs (*TCON&0x40 && (!(*TMOD&0x80) || (*P3&0x08)))


#define timer_0_event (!((*TMOD)&0x04) || (P3_falling&0x10))
#define timer_1_event (!((*TMOD)&0x40) || (P3_falling&0x20))

#define timer_0_mode ((*TMOD)&0x03)
#define timer_1_mode (((*TMOD)>>4)&0x03)

/*		TODO: odladit	*/
static inline void update_timer_0(void)
{
/*	increment TH0/TL0 if inc	*/
	printf("updating timer 0\n");
	(*TL0)++;
	if(timer_0_mode == 0)
		*TL0&=0x1f;
/*	increment TH0 if 8b mode or overflow	*/
	*TH0+=timer_0_mode&2 || !*TL0;
/*		overflow test - TODO:		*/
	*TCON^=((timer_0_mode&2 && (!(*TL0)||(timer_0_mode&1 && !(*TH0))))||!(*TH0))<<5;
}
static inline void update_timer_1(void)
{
	printf("updating timer 1\n");
	(*TL1)++;
	if(timer_1_mode == 0)
		*TL1&=0x1f;
/*	increment TH1 if 8b mode or overflow	*/
	*TH1+=timer_1_mode&2 || !*TL1;
/*		TH1 overflows to TF1		*/
	*TCON^=((timer_1_mode&2&&!*TL1)||(!*TH1))<<7;
}

static inline void do_timers_stuff(void)
{
	if (timer_0_runs&&timer_0_event)
		update_timer_0();
	if (timer_1_runs&&timer_1_event&&(timer_1_mode!=3))
		update_timer_1();

}


static inline void update_ports(void)
{
	unsigned char old;
	old=*P0;
	*P0=port_latches[0]&port_collectors[0];
	*P1=port_latches[1]&port_collectors[1];
	*P2=port_latches[2]&port_collectors[2];
	*P3=port_latches[3]&port_collectors[3];
	P3_falling=old&~*P3;
}

/*	FIXME: ET0 etc..	*/

/*	FIXME: interrupt requests	*/
static inline void set_irqs(void)
{
	#if 0
	/*	sets IE0 if request		*/
	*TCON|=(*P3>>1)&0x02;
	/*	sets IE1 if request		*/
	*TCON|=*P2&0x08;
	#endif

}

static inline void do_int_requests(void)
{


}

void do_every_instruction_stuff(int times)
{

	while (times){
		update_ports();
		set_irqs();
		do_timers_stuff();
		times--;
	}
	do_int_requests();
}


int do_few_instructions(int cnt)
{
	int decrement;
	while (cnt > 0){
		decrement=opcodes[code_memory[PC]].time;
		opcodes[code_memory[PC]].f(PC);
		do_every_instruction_stuff(decrement);
		cnt-=decrement;
		dump();

	}
	return cnt;
}


void alarm_handler(void)
{

	static int alarm_calls=0;
	static int last=0;

	alarm_calls++;
	if (alarm_calls == 100) {
		printf("stovka alarmu\n");
		alarm_calls=0;
	}
	last=do_few_instructions(machine_freq/sync_freq+last);
}





int main(int argc, char *argv[])
{

	/*	main	*/
	char buff[80];
	int i;

	if (argc < 2){
		printf("usage: %s file\n", argv[0]);
		return 1;
	}


	init_instructions();
	init_machine();
	update_ports();
	if (load_hex(argv[1], code_memory, 64*1024)) {
		fprintf(stderr, "cannot open file\n");
		return 1;
	}

	for(i=0; i<64; i++){
		if (i%8 == 0)
			printf("\n");
		printf("%2x", code_memory[i]);
	}

	machine_freq=MACHINE_FREQ_DEFAULT;
	sync_freq=SYNC_FREQ_DEFAULT;

	setup_timer(sync_freq, alarm_handler);

	if (sync_freq > machine_freq){
		fprintf(stderr, "bad sync frequency\n");
		return 1;
	}



	printf("smyckaaaaaa...!\n");

	while (1){

		scanf("%79s", buff);
		printf("buff: %s\n", buff);
/*		printf("%d\n", choice);*/
	}

	return 0;
}
