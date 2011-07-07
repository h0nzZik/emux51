#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H


void init_instructions(void);

typedef struct {
	unsigned int time;
	void (*f)(unsigned short idx);

}opcode_t;

extern opcode_t opcodes[256];

#endif
