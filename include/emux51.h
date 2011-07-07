#ifndef EMUX51_H
#define EMUX51_H


#define CODE_LENGHT 64*1024
#define DATA_LENGHT 256
#define PORTS_CNT 4


#define MACHINE_FREQ_DEFAULT 1000000
#define SYNC_FREQ_DEFAULT 50


/*	byte adress	*/
#define SP	0x81
#define TCON	0x88
#define TMOD	0x89
#define TL0	0x8A
#define TL1	0x8B
#define TH0	0x8C
#define TH1	0x8D
#define PSW	0xD0
#define Acc	0xE0

/****************************************/
/*		bit adresses		*/
/****************************************/

/*	general bit adreseses	*/
#define CARRY	(PSW-0x80+0x07)

/*	port bit adresses	*/
#define P0(x) (0x80 + x)
#define P1(x) (0x90 + x)
#define P2(x) (0xA0 + x)
#define P3(x) (0xB0 + x)


/*	interrupt enable bits		*/
#define EA	0xAF
#define ES	0xAC
#define ET1	0xAB
#define EX1	0xAA
#define ET0	0xA9
#define EX0	0xA8

/*	interrupt priority bits		*/
#define PS	0xBC
#define PT1	0xBB
#define PX1	0xBA
#define PT0	0xB9
#define PX0	0xB8

/*	interrupt flags			*/
#define IE0	0x89
#define IE1	0x8B

/*	timer overflow flags		*/
#define TF0	0x8D
#define TF1	0x8F

/*	level or edge interrupt flags	*/
#define IT0	0x88
#define IT1	0x8A

/*	port 3 alternative functions	*/
#define RXD	P3(0)
#define TXD	P3(1)
#define INT0	P3(2)
#define INT1	P3(3)
#define T0	P3(4)
#define T1	P3(5)
#define WR	P3(6)
#define RD	P3(7)

/*	handler adresses	*/
#define RES_ADDR	0x0000
#define EX0_ADDR	0x0003
#define ET0_ADDR	0x000B
#define EX1_ADDR	0x0013
#define ET1_ADDR	0x001B
#define SER_ADDR	0x0023



extern FILE *async;


/*	control variables	*/

/*	if set, emulator is 'exporting' port and nobody can write	*/
extern int exporting;

/*			*/
extern int running;
extern int loaded;
extern char hexfile[];
extern int interrupt_state;

extern unsigned char code_memory[CODE_LENGHT];
extern unsigned char data_memory[DATA_LENGHT];
extern unsigned short PC;

/*extern unsigned char *SP;*/


/*	port operations			*/
unsigned char read_port (int port);
void write_port(int port, char data);

/*	data memory operations		*/
unsigned char read_data (unsigned addr);
void write_data(unsigned addr, char data);

/*	operations with registers r0-r7	*/
inline unsigned char read_register(int reg);
inline void write_register(int reg, char data);

/*	code memory operations		*/
inline unsigned char read_code(unsigned addr);
inline void write_code(unsigned addr, char data);

/*	operations with Acc		*/
inline unsigned char read_Acc(void);
inline void write_Acc(char data);

/*	operations with bit adress	*/
inline int test_bit(unsigned char addr);
inline void set_bit(unsigned char addr);
inline void clr_bit(unsigned char addr);

/*	stack operations	*/
void push(unsigned char data);
unsigned char pop(void);

void jump_to(unsigned char addr);


/*	returns adresses of port 'port'	*/
/*	int port(int port);*/


void do_reset(void);
void data_dump(char *buffer);

/*	64K  code memory		*/
extern unsigned char code_memory[];

/*	256B data memory	*/
extern unsigned char data_memory[];



void alarm_handler(void);

#endif
