#ifndef EMUX51_H
#define EMUX51_H


#define CODE_LENGHT 64*1024
#define DATA_LENGHT 256


typedef struct {
	unsigned int time;
	void (*f)(unsigned short idx);

}opcode_t;


extern opcode_t opcodes[256];

extern unsigned char code_memory[64*1024];
extern unsigned char data_memory[256];
extern unsigned short PC;

extern unsigned char *SP;
extern unsigned char *PSW;

extern unsigned char *Acc;

#endif
