#ifndef EMUX51_H
#define EMUX51_H


#define CODE_LENGHT 64*1024
#define DATA_LENGHT 256
#define PORTS_CNT 4


#define MACHINE_FREQ_DEFAULT 1000000
#define SYNC_FREQ_DEFAULT 100


/*	byte adress	*/
#define PSW	0xD0

#define Acc	0xE0
#define TCON	0x88
#define TMOD	0x89
#define TL0	0x8A
#define TL1	0x8B
#define TH0	0x8C
#define TH1	0x8D

/*	bit adreses	*/
#define CARRY	(PSW-0x80+0x07)



typedef struct {
	unsigned int time;
	void (*f)(unsigned short idx);

}opcode_t;


extern opcode_t opcodes[256];

extern unsigned char code_memory[CODE_LENGHT];
extern unsigned char data_memory[DATA_LENGHT];
extern unsigned short PC;

extern unsigned char *SP;


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


/*	if set, emulator is 'exporting' port and nobody can write	*/
extern int exporting;

/*	returns adresses of port 'port'	*/
/*	int port(int port);*/

#endif
