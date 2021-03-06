/*
 * emux51.c - A main source file of emux51 emulator.
 *
 *
 */

/* POSIX headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>

/* emux51 headers */
#include <settings.h>
#include <emux51.h>
#include <instructions.h>
#include <module.h>
#include <arch.h>
#include <hex.h>
#include <gui.h>
#include <alarm.h>

/* other headers */
#include <glib.h>



/*	is running?	*/
volatile gint G_GNUC_MAY_ALIAS running;

int loaded=0;
int interrupt_state=0;

/*	64K  code memory		*/
unsigned char code_memory[CODE_LENGHT];
/*	256B data memory	*/
unsigned char data_memory[DATA_LENGHT];
unsigned char sfr_memory[DATA_LENGHT];
/*	2B program counter	*/
unsigned short PC;
/*	port latch register	*/
unsigned char port_latches[PORTS_CNT];
/*	port output		*/
unsigned char port_collectors[PORTS_CNT];
/*	port external state	*/
unsigned char port_externals[PORTS_CNT];
/*	P3 falling edge	*/
unsigned char fall;
/*	machine cycle counter	*/
long long cycle_counter=0;
/*	oscilator frequency	*/
unsigned long Fosc=12000000;


#ifndef MODULE_DIR
	#define MODULE_DIR NULL
#endif
#ifndef UI_FILE
	#define UI_FILE "emux51.glade"
#endif

char *module_directory=MODULE_DIR;
char *ui_file=UI_FILE;

static GOptionEntry option_entries[]=
{
	{ "module_directory", 0, 0, G_OPTION_ARG_FILENAME, &module_directory,
		"", "/path/to/emux51-modules" },
	{ "ui_file", 0, 0, G_OPTION_ARG_FILENAME, &ui_file,
		"", "emux51.glade"},
	{ NULL }
};


void export_all(void)
{
	int i;
	for(i=0; i<4; i++)
		module_export_port(i);
}


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


void update_port(int port)
{
	unsigned char old=port_collectors[port];

	port_collectors[port]=port_latches[port]&port_externals[port];
	sfr_memory[port_to_addr(port)]=port_collectors[port];

	/*		*/
	if (port == 3) {
		fall=old&(~port_collectors[port]);
		if (fall&0x04)
			set_bit(IE0);
		if (fall&0x08)
			set_bit(IE1);
	}

	module_export_port(port);

}
/*	<API for instructions>	*/

unsigned char read_data(unsigned char addr)
{
	if (addr < 0x80)
		return(data_memory[addr]);
	else
		return(sfr_memory[addr]);
	
}

void write_data(unsigned char addr, char data)
{
	int port;


	if (addr < 0x80) {
		data_memory[addr]=data;

	} else if (isport(addr)){
		port=addr_to_port(addr);
		port_latches[port]=data;
		update_port(port);
	} else {
		sfr_memory[addr]=data;
	}
}

unsigned char indirect_read_data(unsigned char addr)
{
	return (data_memory[addr]);
}
void indirect_write_data(unsigned char addr, char data)
{
	data_memory[addr]=data;
}

unsigned char rmw_read(unsigned char addr)
{
	if (isport(addr))
		return(port_latches[addr_to_port(addr)]);
	return(read_data(addr));
}

unsigned char read_code(unsigned short addr)
{
	return(code_memory[addr]);
}


unsigned char reg_to_addr(int reg)
{
	unsigned base;

	base=sfr_memory[PSW]&0x18;
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
	return(sfr_memory[Acc]);
}

void write_Acc(char data)
{
	sfr_memory[Acc]=data;
}
void add_Acc(unsigned char increment)
{
	unsigned char acc;
	int sum;
	int nibble_sum;

	acc=read_Acc();	
	sum=acc+increment;
	write_bit(CARRY, sum>255);
	nibble_sum=(acc&0x0F)+increment;
	write_bit(ACARRY, nibble_sum>15);
	write_Acc((unsigned char) sum);
}
void sub_Acc(unsigned char decrement)
{
	unsigned char acc;
	int sum;
	int nibble_sum;

	acc=read_Acc();
	sum=acc-decrement;
	write_bit(CARRY, sum<0);
	nibble_sum=(acc&0x0F)-decrement;
	write_bit(ACARRY, nibble_sum<0);
	write_Acc((unsigned char) sum);	
}



/*	So, bit addressable memory is inside (0x20, 0x2F) and (0x80, 0xF8).
 *	Between 0x20 and 0x2F is addressable each bit.
 *	Between 0x80 and 0xF8 are addressable bits in bytes 0x80, 0x88, 0x90 etc.
 *	I wanna be rev3rse engineer ;)
 */

unsigned char addr_to_bit_bit(unsigned char addr)
{
	return(addr&0x07);
}

unsigned char addr_to_bit_byte(unsigned char addr)
{
	addr&=~0x07;
	if (addr&0x80)
		return addr;
	return(0x20|(addr>>3));
}

void set_bit(unsigned char bit_addr)
{
	unsigned char byte;
	unsigned char bit;
	unsigned char data;

	byte=addr_to_bit_byte(bit_addr);
	bit=addr_to_bit_bit(bit_addr);
/*
	if (isport(byte))	
		data=port_latches[addr_to_port(byte)];
	else 
		data=read_data(byte);
*/
	data=rmw_read(byte);
	data|=1<<bit;
	write_data(byte, data);

}

void clr_bit(unsigned char bit_addr)
{
	unsigned char byte;
	unsigned char bit;
	unsigned char data;

	byte=addr_to_bit_byte(bit_addr);
	bit=addr_to_bit_bit(bit_addr);
/*
	if (isport(byte))	
		data=port_latches[addr_to_port(byte)];
	else
		data=read_data(byte);
*/
	data=rmw_read(byte);
	data&=~(1<<bit);
	write_data(byte, data);
}

void write_bit(unsigned char bit, int data)
{
	data&=1;
	if (data)
		set_bit(bit);
	else
		clr_bit(bit);
}

void neg_bit(unsigned char bit_addr)
{
	unsigned char byte;
	unsigned char bit;
	unsigned char data;


	byte=addr_to_bit_byte(bit_addr);
	bit=addr_to_bit_bit(bit_addr);
/*
	if (isport(byte))	
		data=port_latches[addr_to_port(byte)];
	else
		data=read_data(byte);
*/
	data=rmw_read(byte);
	data^=1<<bit;
	write_data(byte, data);

}


int test_bit(unsigned char bit_addr)
{
	unsigned char byte;
	unsigned char bit;

	byte=addr_to_bit_byte(bit_addr);
	bit=addr_to_bit_bit(bit_addr);
	return(read_data(byte)&(1<<bit)?1:0);
}

int test_bit_rmw(unsigned char bit_addr)
{
	unsigned char byte;
	unsigned char bit;

	byte=addr_to_bit_byte(bit_addr);
	bit=addr_to_bit_bit(bit_addr);
	return(rmw_read(byte)&(1<<bit)?1:0);

}

/*	</API for instructions>	*/

void do_reset(void)
{
	memset (data_memory, 0, DATA_LENGHT);
	memset (sfr_memory, 0, DATA_LENGHT);
	memset (port_latches, 0xFF, 4);
	memset (port_collectors, 0xFF, 4);
	memset (port_externals, 0xFF, 4);

	sfr_memory[SP]=7;


	sfr_memory[0x80]=0xFF;
	sfr_memory[0x90]=0xFF;
	sfr_memory[0xA0]=0xFF;
	sfr_memory[0xB0]=0xFF;

	PC=0;

	cycle_counter=0;

	interrupt_state=0;

	printf("[emux51]\treset\n");
	module_reset_all();

}


void init_machine(void)
{
	memset(code_memory, 0, CODE_LENGHT);
	do_reset();
}


/*	TODO:	external, second timer	*/


int timer_0_mode(void)
{
	return (sfr_memory[TMOD]&0x03);
}

int timer_1_mode(void)
{
	return ((sfr_memory[TMOD]>>4)&0x03);
}
int timer_0_event(void)
{
	if ((sfr_memory[TMOD]&0x04) == 0)
		return 1;
	if ((fall&0x10) == 1)
		return 1;
	return 0;
}

int timer_1_event(void)
{
	if ((sfr_memory[TMOD]&0x40) == 0)
		return 1;
	if ((fall&0x20) == 1)
		return 1;
	return 0;
}
int timer_0_running(void)
{
	/*	TR0 is not set	*/
	if ((sfr_memory[TCON]&0x10) == 0)
		return 0;
	if ((sfr_memory[TMOD]&0x08) == 0)
		return 1;
	if ((sfr_memory[port_to_addr(3)]&0x04) == 1)
		return 1;
	return 0;
}

int timer_1_running(void)
{
	/*	TR1 is not set	*/
	if ((sfr_memory[TCON]&0x40) == 0)
		return 0;
	/*	timer 1 cannot run in mode 3	*/
	if (timer_1_mode() == 3)
		return 0;
	if ((sfr_memory[TMOD]&0x80) == 0)
		return 1;
	if ((sfr_memory[port_to_addr(3)]&0x08) == 1)
		return 1;
	return 0;
}



/*		Some timer stuff. Timer 0 in mode 1 (16b) is ok,
 *		but I must do the other stuff.
 */
static void update_timer_0(void)
{

	sfr_memory[TL0]++;
	if(timer_0_mode() == 0)
		sfr_memory[TL0]&=0x1f;
/*	increment TH0 if 8b mode or overflow	*/
	if ((timer_0_mode() > 2) || (sfr_memory[TL0] == 0)) {
		sfr_memory[TH0]++;
	}
/*		overflow test - TODO:		*/
	switch(timer_0_mode()) {
		case 0:
		case 1:
		if ((sfr_memory[TH0] == 0) && (sfr_memory[TL0] == 0)){
			neg_bit(TF0);
		}
		break;
		case 2:
		if (sfr_memory[TL0] == 0) {
			neg_bit(TF0);
			sfr_memory[TL0]=sfr_memory[TH0];
		}
		break;

		case 3:
		break;
	}

}
static void update_timer_1(void)
{

	sfr_memory[TL1]++;
	if(timer_1_mode() == 0)
		sfr_memory[TL1]&=0x1f;
/*	increment TH0 if 8b mode or overflow	*/
	if ((timer_1_mode() & 2) || (sfr_memory[TL1] == 0)) {
		sfr_memory[TH1]++;
	}
/*		overflow test - TODO:		*/
	switch(timer_1_mode()) {
		case 0:
		case 1:
		if ((sfr_memory[TH1] == 0) && (sfr_memory[TL1] == 0)){
			neg_bit(TF1);
		}
		break;
		case 2:
		if (sfr_memory[TL1] == 0) {
			neg_bit(TF1);
			sfr_memory[TL1]=sfr_memory[TH1];
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

void push(unsigned char data)
{
	sfr_memory[SP]++;
	data_memory[sfr_memory[SP]]=data;
}
unsigned char pop(void)
{
	unsigned char data;
	data=data_memory[sfr_memory[SP]];
	sfr_memory[SP]--;
	return(data);
}

void jump_to(unsigned char addr)
{
	PC=addr;
}


#define EX0_REQUEST (test_bit(EX0) && ((test_bit(IT0) && test_bit(IE0))\
			||(!test_bit(IT0) && !test_bit(INT0))))

#define EX1_REQUEST (test_bit(EX1) && ((test_bit(IT1) && test_bit(IE1))\
			||(!test_bit(IT1) && !test_bit(INT1))))

#define ET0_REQUEST (test_bit(ET0) && test_bit(TF0))
#define ET1_REQUEST (test_bit(ET1) && test_bit(TF1))

#define HAVE_REQUEST (EX0_REQUEST || EX1_REQUEST || ET0_REQUEST || ET1_REQUEST)


#define EX0_PREQUEST (EX0_REQUEST && test_bit(PX0))
#define EX1_PREQUEST (EX1_REQUEST && test_bit(PX1))
#define ET0_PREQUEST (ET0_REQUEST && test_bit(PT0))
#define ET1_PREQUEST (ET1_REQUEST && test_bit(PT1))

#define HAVE_PREQUEST (EX0_PREQUEST || EX1_PREQUEST || ET0_PREQUEST || ET1_PREQUEST)



static void do_int_requests(void)
{

	/* are interrupts enabled? */
	if (test_bit(EA) == 0)
		return;
	/* is there any request? */
	if (HAVE_REQUEST == 0)
		return;
	/* high priority request can't be interrupted */
	if (interrupt_state & 0x2)
		return;
	/* low priority request can be interrupted only by hight */
	if ((interrupt_state == 1) && (HAVE_PREQUEST == 0)){
		return;
	}

	if (HAVE_PREQUEST == 1)
		interrupt_state|=2;
	else
		interrupt_state|=1;

	push((unsigned char)(PC&0x00FF));
	push((unsigned char)(PC>>8));

/*		high priority		*/
	if (EX0_PREQUEST) {
		clr_bit(IE0);
//		printf("[emux]\tjumping to ext0\n");
		int_log_append("[High priority] External 0\n");
		jump_to(EX0_ADDR);
		return;
	}
	if (ET0_PREQUEST) {
		clr_bit(TF0);
//		printf("[emux]\tjumping to timer 0\n");
		int_log_append("[High priority] External 0\n");
		
		jump_to(ET0_ADDR);
		return;
	}
	if (EX1_PREQUEST) {
		clr_bit(IE0);
//		printf("[emux]\tjumping to ext1\n");
		int_log_append("[High priority] External 0\n");

		jump_to(EX1_ADDR);
		return;
	}
	if (ET1_PREQUEST) {
		clr_bit(TF1);
//		printf("[emux]\tjumping to timer 1\n");
		int_log_append("[High priority] External 0\n");
	
		jump_to(ET1_ADDR);
		return;
	}
/*		low priority		*/
	if (EX0_REQUEST) {
		clr_bit(IE0);
//		printf("[emux]\tjumping to ext0\n");
		int_log_append("[Low priority]\tExternal 0\n");
		
		jump_to(EX0_ADDR);
		return;
	}
	if (ET0_REQUEST) {
		clr_bit(TF0);
//		printf("[emux]\tjumping to timer 0\n");
		int_log_append("[Low priority]\tTimer 0\n");
		
		jump_to(ET0_ADDR);
		return;
	}
	if (EX1_REQUEST) {
		clr_bit(IE1);
//		printf("[emux]\tjumping to ext1\n");
		int_log_append("[Low priority]\tExternal 1\n");
		jump_to(EX1_ADDR);
		return;
	}
	if (ET1_REQUEST) {
		clr_bit(TF1);
//		printf("[emux]\tjumping to timer 1\n");
		int_log_append("[low priority]\tTimer 1\n");
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
		do_timers_stuff();
		times--;
	}
	do_int_requests();
}
int do_few_instructions(int cycles)
{
	int decrement;
	while (cycles > 0){
		decrement=opcodes[code_memory[PC]].time;
		opcodes[code_memory[PC]].f(PC);

		do_every_instruction_stuff(decrement);
		cycle_queue_perform(decrement);
		cycles-=decrement;
		cycle_counter += decrement;
	}
	return cycles;
}
/*		</main execution loop>		*/

/*	remaining cycles to 'zero-time'	*/
unsigned int remaining_machine_cycles;
unsigned int remaining_sync_cycles;

void alarm_handler(void)
{
	static int last=0;
	int cnt;

	if (g_atomic_int_get(&running) == 0)
		return;
	time_queue_perform();

	cnt=remaining_machine_cycles/remaining_sync_cycles;
	last=do_few_instructions(cnt+last);
	remaining_machine_cycles-=cnt;
	remaining_sync_cycles--;

	if (remaining_sync_cycles == 0) {
		remaining_machine_cycles=Fosc;
		remaining_sync_cycles=12*SYNC_FREQ;
	}
	gui_callback();
}

void sigint_handler(int data)
{
	printf("[emux]\texiting because of keyboard interrupt\n");
	exit(0);
}


int main(int argc, char *argv[])
{
	GOptionContext *ctx;
	GError *err=NULL;
	printf("[emux]\tstarting..\n");
	
	if (g_module_supported() == FALSE) {
		printf("[emux51]\tWarning: GModule is not supported on this system.\n");
	}

	g_set_application_name("Emux51");

	/*	parse command-line arguments	*/
	ctx=g_option_context_new(NULL);
	g_option_context_add_main_entries(ctx, option_entries, NULL);
	g_option_context_add_group(ctx, gtk_get_option_group(TRUE));
	if (g_option_context_parse(ctx, &argc, &argv, &err) == 0) {
		g_print("[emux51]\toption parsing failed: %s\n", err->message);
		exit(1);
	}
	srand(time(NULL));
	config_parse();
	signal(SIGINT, sigint_handler);
	init_instructions();
	init_machine();
	modules_init();

	set_timer(SYNC_FREQ, alarm_handler);

	gui_run(&argc, &argv);
	printf("[emux]\texiting..\n");
	return 0;
}

void program_start(void)
{
	printf("program_start\n");
	remaining_machine_cycles=Fosc;
	remaining_sync_cycles=12*SYNC_FREQ;
	g_atomic_int_set(&running, 1);
}
void program_stop(void)
{
	g_atomic_int_set(&running, 0);
	do_reset();
	export_all();
}
void program_pause(void)
{
	g_atomic_int_set(&running, 0);
}
