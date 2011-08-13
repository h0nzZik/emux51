/*
 * emux51.c - A main source file of emux51 emulator.
 *
 * TODO: do some timers stuff.
 *	 rewrite 'do_int_requests'
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <dlfcn.h>

#include <settings.h>
#include <emux51.h>
#include <instructions.h>
#include <module.h>
#include <arch.h>
#include <hex.h>
#include <gui.h>
#include <alarm.h>

/* check this: */
#define FORCE_READ 0


int running=0;
int loaded=0;
int interrupt_state=0;
char hexfile[PATH_MAX];

/*	64K  code memory		*/
unsigned char code_memory[CODE_LENGHT];
/*	256B data memory	*/
unsigned char data_memory[DATA_LENGHT];
/*	2B program counter	*/
unsigned short PC;
/*	some port variables	*/
unsigned char port_latches[PORTS_CNT];
unsigned char port_collectors[PORTS_CNT];
unsigned char port_fall[PORTS_CNT];
/*	machine cycle counter	*/
unsigned long counter=0;
/*	machine cycle freuency, 1/12 * Fosc	*/
double machine_freq;



int isport(unsigned addr)
{
	addr-=0x80;
	if (addr % 0x10)
		return 0;
	addr/=0x10;
	return(addr < 4);
}

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


/*		<API for modules>		*/
int exporting=0;
unsigned char read_port(int port)
{
	return(port_collectors[port]);
}
void write_port(int port, char data)
{
	char old;

	old=port_collectors[port];
	port_collectors[port]=port_latches[port]&data;
	data_memory[port_to_addr(port)]=port_collectors[port];
	port_fall[port]=old&~read_port(port);

	exporting=1;
	module_export_port(port);
	exporting=0;

}
/*	</API for modules>	*/


/*	<API for instructions>	*/

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
/*
		printf("[emux]\twriting 0x%2x to port %d\n", data, port);
*/
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


unsigned reg_to_addr(int reg)
{
	unsigned base;

	base=data_memory[PSW]&0x18;
	return(base|(reg&0x07));
}

unsigned char read_register(int reg)
{
	return(data_memory[reg_to_addr(reg)]);
}

void write_register(int reg, char data)
{
	data_memory[reg_to_addr(reg)]=data;
}

/*
 *	Accumulator operations
 */

unsigned char read_Acc(void)
{
	return(data_memory[Acc]);
}

void write_Acc(char data)
{
	data_memory[Acc]=data;
}
void add_Acc(unsigned char increment)
{
	unsigned char acc;
	unsigned int sum;

	acc=read_Acc();	
	sum=acc+increment;
	write_carry(sum>255);
	write_Acc((unsigned char) sum);
}

void write_carry(int data)
{
	data&=1;
	if (data)
		set_bit(CARRY);
	else
		clr_bit(CARRY);

}


/*	So, bit adressable memory is inside (0x20, 0x2F) and (0x80, 0xF8).
 *	Between 0x20 and 0x2F is adressable each bit.
 *	Between 0x80 and 0xF8 are adressable bits in bytes 0x80, 0x88, 0x90 etc.
 *	I wanna be rev3rse engineer ;)
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

void set_bit(unsigned char addr)
{
	unsigned char byte;
	unsigned char bit;
	unsigned char data;

	byte=addr_to_bit_byte(addr);
	bit=addr_to_bit_bit(addr);

	data=data_memory[byte];
	data|=1<<bit;
	write_data(byte, data);

}

void clr_bit(unsigned char addr)
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

void neg_bit(unsigned char addr)
{
	unsigned char byte;
	unsigned char bit;
	unsigned char data;

	byte=addr_to_bit_byte(addr);
	bit=addr_to_bit_bit(addr);	
	data=data_memory[byte];
	data^=1<<bit;
	write_data(byte, data);
}


int test_bit(unsigned char addr)
{
	unsigned char byte;
	unsigned char bit;

	byte=addr_to_bit_byte(addr);
	bit=addr_to_bit_bit(addr);
	return(data_memory[byte]&(1<<bit));
}

/*	</API for instructions>	*/







void do_reset(void)
{
	memset (data_memory, 0, DATA_LENGHT);
	memset (port_latches, 0xFF, 4);
	memset (port_collectors, 0xFF, 4);
	data_memory[SP]=7;

	data_memory[0x80]=0xFF;
	data_memory[0x90]=0xFF;
	data_memory[0xA0]=0xFF;
	data_memory[0xB0]=0xFF;

	PC=0;
}


void init_machine(void)
{
	memset(code_memory, 0, CODE_LENGHT);
	do_reset();
}





/*	TODO:	external, second timer	*/


inline int timer_0_mode(void)
{
	return (data_memory[TMOD]&0x03);
}

inline int timer_1_mode(void)
{
	return ((data_memory[TMOD]>>4)&0x03);
}
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
	if ((timer_0_mode() > 2) || (data_memory[TL0] == 0)) {
		data_memory[TH0]++;
	}
/*		overflow test - TODO:		*/
	switch(timer_0_mode()) {
		case 0:
		case 1:
		if ((data_memory[TH0] == 0) && (data_memory[TL0] == 0)){
			neg_bit(TF0);
		}
		break;
		case 2:
		if (data_memory[TL0] == 0) {
			neg_bit(TF0);
			data_memory[TL0]=data_memory[TH0];
		}
		break;

		case 3:
		break;
	}

}
static inline void update_timer_1(void)
{

	data_memory[TL1]++;
	if(timer_1_mode() == 0)
		data_memory[TL1]&=0x1f;
/*	increment TH0 if 8b mode or overflow	*/
	if ((timer_1_mode() & 2) || (data_memory[TL1] == 0)) {
		data_memory[TH1]++;
	}
/*		overflow test - TODO:		*/
	switch(timer_1_mode()) {
		case 0:
		case 1:
		if ((data_memory[TH1] == 0) && (data_memory[TL1] == 0)){
			neg_bit(TF1);
		}
		break;
		case 2:
		if (data_memory[TL1] == 0) {
			neg_bit(TF1);
			data_memory[TL1]=data_memory[TH1];
		}
		break;

		case 3:
		break;
	}

}
static void do_timers_stuff(void)
{
	if (timer_0_running() && timer_0_event()) {
		update_timer_0();
	}

	if (timer_1_running() && timer_0_event())
		update_timer_1();
}


/*	TODO: interrupt requests	*/
static inline void set_irqs(void)
{
	/*	INT0	*/
	if((test_bit(IT0) && (port_fall[3]&0x04)) || test_bit(INT0))
		set_bit(IE0);
	/*	INT1	*/
	if((test_bit(IT1) && (port_fall[3]&0x08)) || test_bit(INT1))
		set_bit(IE1);

}
	/*	TODO: serial	*/


void push(unsigned char data)
{
	data_memory[SP]++;
	data_memory[data_memory[SP]]=data;
}
unsigned char pop(void)
{
	unsigned char data;
	data=data_memory[data_memory[SP]];
	data_memory[SP]--;
	return(data);
}

void jump_to(unsigned char addr)
{
	PC=addr;
}

#define HAVE_REQUEST (test_bit(IE0) || test_bit(TF0) || test_bit(IE1) || test_bit(TF1))
#define HAVE_PRIORITY_REQUEST ( (test_bit(IE0) && test_bit(PX0)) ||\
				(test_bit(TF0) && test_bit(PT0)) ||\
				(test_bit(IE1) && test_bit(PX1)) ||\
				(test_bit(TF1) && test_bit(PT1)) )

static void do_int_requests(void)
{

	if (test_bit(EA) == 0)
		return;
/*		going to interrupt?	*/
	if (HAVE_REQUEST == 0)
		return;
	if (interrupt_state == 2)
		return;
	if ((interrupt_state == 1) && (HAVE_PRIORITY_REQUEST == 0))
		return;

	if (HAVE_PRIORITY_REQUEST == 1)
		interrupt_state|=2;
	else {
		interrupt_state|=1;
	}

	push((unsigned char)(PC&0x00FF));
	push((unsigned char)(PC>>8));

/*		high priority		*/
	if (test_bit(PX0) && test_bit(IE0)) {
		interrupt_state=2;
		printf("[emux]\tjumping to ext0\n");
		jump_to(EX0_ADDR);
		return;
	}
	if (test_bit(PT0) && test_bit(TF0)) {
		clr_bit(TF0);
		interrupt_state=2;
		printf("[emux]\tjumping to timer 0\n");
		jump_to(ET0_ADDR);
		return;
	}
	if (test_bit(PX1) && test_bit(IE1)) {
		interrupt_state=2;
		printf("[emux]\tjumping to ext1\n");
		jump_to(EX1_ADDR);
		return;
	}
	if (test_bit(PT1) && test_bit(TF1)) {
		clr_bit(TF1);
		interrupt_state=2;
		printf("[emux]\tjumping to timer 1\n");
		jump_to(ET1_ADDR);
		return;
	}
/*		low priority		*/
	if (test_bit(IE0)) {
		interrupt_state=1;
		printf("[emux]\tjumping to ext0\n");
		jump_to(EX0_ADDR);
		return;
	}
	if (test_bit(TF0)) {
		clr_bit(TF0);
		interrupt_state=1;
		printf("[emux]\tjumping to timer 0\n");
		jump_to(ET0_ADDR);
		return;
	}
	if (test_bit(IE1)) {
		interrupt_state=1;
		printf("[emux]\tjumping to ext1\n");
		jump_to(EX1_ADDR);
		return;
	}
	if (test_bit(TF1)) {
		clr_bit(TF1);
		interrupt_state=1;
		printf("[emux]\tjumping to timer 1\n");
		jump_to(ET1_ADDR);
		return;
	}

	printf("[emux]\twtf?\n");



}

/*		<main execution loop>
 *	So, first we have to do current instruction,
 *	Next we have to do some timer stuff and handle interrupts,
 *	but before it we can check P3.
 */

void do_every_instruction_stuff(int times)
{

	while (times){
		counter++;
		if (FORCE_READ)
			module_import_port(3);
/*		set_irqs();				*/
		do_timers_stuff();
		times--;
	}
	do_int_requests();
}
int do_few_instructions(int cycles)
{
	int decrement;
	while (cycles > 0){
		if (running == 0)
			return 0;
		decrement=opcodes[code_memory[PC]].time;
		opcodes[code_memory[PC]].f(PC);

		do_every_instruction_stuff(decrement);
		module_op_queue_perform(decrement);
		cycles-=decrement;
	}
	return cycles;
}
/*		</main execution loop>		*/

void alarm_handler(void)
{

	static int last=0;
	int cnt;
/*	static int alarm_calls=0;

	alarm_calls++;
	if (alarm_calls == SYNC_FREQ) {
		printf("one second\n");
		alarm_calls=0;
	}
*/
	module_time_queue_perform();
	if (running) {
		gui_callback();
		cnt=machine_freq/SYNC_FREQ+last;
		last=do_few_instructions(cnt);
	}
}

void sigint_handler(int data)
{
	printf("[emux]\texiting because of keyboard interrupt\n");
	module_destroy_all("keyboard interrupt");
	exit(0);
}



char *dump_head="--\t00\t01\t02\t03\t04\t05\t06\t07\t08\t09\t0A\t0B\t0C\t0D\t0E\t0F\n";

void data_dump(char *buffer)
{
	unsigned int i, j;

	sprintf(buffer, dump_head);
	buffer+=strlen(buffer);

	for (i=0; i<16; i++) {
		sprintf(buffer, "%X0\t", i);
		buffer+=3;
		for (j=0; j<16; j++) {
			sprintf(buffer, "%02X\t", data_memory[16*i+j]);
			buffer+=3;
		}
		*buffer++='\n';
	}

}

int main(int argc, char *argv[])
{
//	void *hndl;
	printf("[emux]\tstarting..\n");
//	hndl=dlopen("./widgets.so", RTLD_NOW | RTLD_GLOBAL);
//	if (hndl == NULL)
//		printf("dlopen error\n");
	signal(SIGINT, sigint_handler);
	init_instructions();
	init_machine();
	modules_init();

	machine_freq=MACHINE_FREQ_DEFAULT;
	set_timer(SYNC_FREQ, alarm_handler);

	gui_run(&argc, &argv);
	printf("[emux]\texiting..\n");
	return 0;
}
