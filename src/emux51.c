#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emux51.h>
#include <instructions.h>
#include <module.h>
#include <arch/arch.h>
/*			TODO:			*/
/*	timers stuff, I have only T0 at mod1	*/
/*						*/


/*	64K  code memory		*/
unsigned char code_memory[CODE_LENGHT];
/*	256B data memory	*/
unsigned char data_memory[DATA_LENGHT];
unsigned short PC;

unsigned char *SP=&data_memory[0x81];

unsigned short *DPTR=(void *)(&data_memory[0x82]);



unsigned char port_latches[PORTS_CNT];
unsigned char port_collectors[PORTS_CNT];
unsigned char port_fall[PORTS_CNT];

unsigned long counter=0;

#define FORCE_READ 0

opcode_t opcodes[256];


double machine_freq;
double sync_freq;

int isport(unsigned addr)
{
	addr-=0x80;
	addr%=0x10;
	return(!addr);
}

/*	port adresses	*/
unsigned port_to_addr(int port)
{
	port%=PORTS_CNT;
	return(0x80+port*0x10);
}

int addr_to_port(unsigned addr)
{
	addr-=0x80;
	addr/=0x10;
	return(addr%PORTS_CNT);
}

/*		<API for module.c>		*/
int exporting=0;
unsigned char read_port(int port)
{
	return(port_collectors[port]);
}
void write_port(int port, char data)
{
	char old;
	old=read_port(port);
	port_collectors[port]=port_latches[port]&data;
	data_memory[port_to_addr(port)]=port_collectors[port];
	port_fall[port]=old&~read_port(port);

	exporting=1;
	module_export_port(port);
	exporting=0;

}
/*		</API for module.c>		*/

/*		<API for instructions>		*/
unsigned char read_data(unsigned addr)
{
	if(FORCE_READ && isport(addr)){
		module_import_port(addr_to_port(addr));
	}
	return(data_memory[addr]);
	
}

void write_data(unsigned addr, char data)
{
	int port;
	data_memory[addr]=data;
	if(isport(addr)){
		port=addr_to_port(addr);
		port_latches[port]=data;
		port_collectors[port]=data;

		exporting=1;
		module_export_port(port);
		exporting=0;
	}



}
unsigned char read_code(unsigned addr)
{
	return(code_memory[addr]);
}

void write_code(unsigned addr, char data)
{
	code_memory[addr]=data;
}


inline unsigned reg_to_addr(int reg)
{
	unsigned base;
	base=data_memory[PSW]&0x18;
	return(base|(reg&0x07));
}

inline unsigned char read_register(int reg)
{
	return(data_memory[reg_to_addr(reg)]);
}

inline void write_register(int reg, char data)
{
	data_memory[reg_to_addr(reg)]=data;
}

inline unsigned char read_Acc(void)
{
	return(data_memory[Acc]);
}
inline void write_Acc(char data)
{
	data_memory[Acc]=data;
}


/*	So, bit adressable memory is inside (0x20; 0x2F) and (0x80; 0xF8)
 *	Between 0x20 and 0x2F is adressable each bit.
 *	Between 0x80 and 0xF8 are adressable bits in bytes 0x80, 0x88, 0x90 etc.
 *	I wanna be a rev3rse engineer ;)
 */

inline unsigned char addr_to_bit_bit(unsigned char addr)
{
	return(addr&0x07);
}

inline unsigned char addr_to_bit_byte(unsigned char addr)
{
	addr&=~0x07;
	if (addr&0x80)
		return addr;
	return(0x20|(addr>>3));
}

inline void set_bit(unsigned char addr)
{
	unsigned char byte;
	unsigned char bit;
	unsigned char data;

	byte=addr_to_bit_byte(addr);
	bit=addr_to_bit_bit(addr);
	data=data_memory[byte];
	data|=1<<bit;
	write_data(addr, data);
}

inline void clr_bit(unsigned char addr)
{
	unsigned char byte;
	unsigned char bit;
	unsigned char data;

	byte=addr_to_bit_byte(addr);
	bit=addr_to_bit_bit(addr);
	data=data_memory[byte];
	data&=~(1<<bit);
	write_data(byte, data);
}

inline int test_bit(unsigned char addr)
{
	unsigned char byte;
	unsigned char bit;

	byte=addr_to_bit_byte(addr);
	bit=addr_to_bit_bit(addr);

	return(data_memory[byte]&(1<<bit));
}
/*		</TODO: test this stuff>	*/

/*		</API for instructions>		*/


/*	TODO: checksum, segments and other Intel Hex features	*/
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

	/*	source file and destination index	*/


	if (!(file && dest))
		return -1;

	fr=fopen(file, "rt");
	if (fr == NULL)
		return -1;

	while (1 == fscanf(fr, "%s", buff)){
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
	*SP=7;
	PC=0;

}



/*	TODO:	external, second timer	*/

/*	FIXME:	Do not use this macros	*/

inline int timer_0_event(void)
{
	if ((data_memory[TMOD]&0x04) == 0)
		return 1;
	if ((port_fall[3]&0x10) == 1)
		return 1;
	return 0;
}

inline int timer_1_event(void)
{
	if ((data_memory[TMOD]&0x40) == 0)
		return 1;
	if ((port_fall[3]&0x20) == 1)
		return 1;
	return 0;
}


inline int timer_0_mode(void)
{
	return (data_memory[TMOD]&0x03);
}

inline int timer_1_mode(void)
{
	return ((data_memory[TMOD]>>4)&0x03);
}

inline int timer_0_running(void)
{
	/*	TR0 is not set	*/
	if ((data_memory[TCON]&0x10) == 0)
		return 0;
	if ((data_memory[TMOD]&0x08) == 0)
		return 1;
	if ((data_memory[port_to_addr(3)]&0x04) == 1)
		return 1;
	return 0;
}

inline int timer_1_running(void)
{
	/*	TR1 is not set	*/
	if ((data_memory[TCON]&0x40) == 0)
		return 0;
	/*	timer 1 cannot run in mode 3	*/
	if (timer_1_mode() == 3)
		return 0;
	if ((data_memory[TMOD]&0x80) == 0)
		return 1;
	if ((data_memory[port_to_addr(3)]&0x08) == 1)
		return 1;
	return 0;
}



/*		Some timer stuff. Timer 0 in mode 1 (16b) is ok,
 *		but I must do the other stuff.
 */
static inline void update_timer_0(void)
{

	data_memory[TL0]++;
	if(timer_0_mode() == 0)
		data_memory[TL0]&=0x1f;
/*	increment TH0 if 8b mode or overflow	*/
	if ((timer_0_mode() & 2) || (data_memory[TL0] == 0)) {
		data_memory[TH0]++;
	}
/*		overflow test - TODO:		*/
	if ((timer_0_mode() == 1) && (data_memory[TH0] == 0)
	   && (data_memory[TL0] == 0)) {
		data_memory[TCON]^=0x20;
	}
}
static inline void update_timer_1(void)
{
#if 0
	printf("updating timer 1\n");
	(*TL1)++;
	if(timer_1_mode == 0)
		*TL1&=0x1f;
/*	increment TH1 if 8b mode or overflow	*/
	*TH1+=timer_1_mode&2 || !*TL1;
/*		TH1 overflows to TF1		*/
	*TCON^=((timer_1_mode&2&&!*TL1)||(!*TH1))<<7;
#endif
}
static inline void do_timers_stuff(void)
{

	if (timer_0_running() && timer_0_event())
		update_timer_0();

	if (timer_1_running() && timer_0_event())
		update_timer_1();
}



/*	TODO: interrupt requests	*/
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

/*		<main execution loop>
 *	So, first we have to do current instruction,
 *	Second we have to do some timer stuff and handle interrupts,
 *	before it we can check P3.
 */

void do_every_instruction_stuff(int times)
{

	while (times){
		counter++;
		if (FORCE_READ)
			module_import_port(3);
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
		module_queue_perform(decrement);
		cnt-=decrement;

/*		dump();*/

	}
	return cnt;
}
/*		</main execution loop>		*/

void alarm_handler(void)
{

	static int alarm_calls=0;
	static int last=0;

	alarm_calls++;
	if (alarm_calls == sync_freq) {
/*		printf("one second\n");*/
		alarm_calls=0;
	}
	last=do_few_instructions(machine_freq/sync_freq+last);
}





int main(int argc, char *argv[])
{

	/*	main	*/
	char buff[80];
	if (argc < 2){
		printf("usage: %s file\n", argv[0]);
		return 1;
	}
	init_instructions();
	init_machine();
/*	update_ports();*/
	if (load_hex(argv[1], code_memory, 64*1024)) {
		fprintf(stderr, "cannot open file\n");
		return 1;
	}

	modules_init();

	machine_freq=MACHINE_FREQ_DEFAULT;
	sync_freq=SYNC_FREQ_DEFAULT;

	if (sync_freq > machine_freq){
		fprintf(stderr, "bad sync frequency\n");
		return 1;
	}

	if(module_new("modules/bin/first.so")){
/*	if (module_new("modules/bin/gtk-test.so")){*/
		printf("cannot load module\n");
		return 1;
	}


	setup_timer(sync_freq, alarm_handler);
	printf("smyckaaaaaa...!\n");

	while (1){

		scanf("%79s", buff);
		printf("buff: %s\n", buff);
/*		printf("%d\n", choice);*/
	}

	return 0;
}
