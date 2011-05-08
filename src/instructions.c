#include <stdlib.h>
#include <stdio.h>
#include <instructions.h>
#include <emux51.h>


#define REG(x) ((x)|(data_memory[(data_memory[*PSW]&0x18)]))


/*	TODO: a lot of instructions ;-P	*/


void mov_rx_imm8(unsigned short idx)
{
	printf("mov r%d, #0x%2x\n", (*PSW&0x18)|(code_memory[idx]&0x07), code_memory[idx+1]);
	data_memory[(*PSW&0x18)|(code_memory[idx]&0x07)]=
		code_memory[idx+1];
	PC+=2;

}



void mov_rx_addr(unsigned short idx)
{
/*	printf("mov rx, addr, base at 0x%x\n", (*PSW&0x18));*/
	data_memory[(*PSW&0x18)|(code_memory[idx]&0x07)]=
		data_memory[code_memory[idx+1]];
	PC+=2;
}


void mov_addr_rx(unsigned short idx)
{
	data_memory[code_memory[idx+1]]=
		data_memory[(*PSW&0x18)|(code_memory[idx]&0x07)];

/*	data_memory[code_memory[idx+1]]=REG(code_memory[idx]&0x07);*/

	PC+=2;
}


void mov_a_rx(unsigned short idx)
{
	*Acc=data_memory[(*PSW&0x18)|(code_memory[idx]&0x07)];
	PC++;
}

void mov_rx_a(unsigned short idx)
{
	data_memory[(*PSW&0x18)|(code_memory[idx]&0x07)]=*Acc;
	PC++;
}

void ljmp(unsigned short idx)
{
	PC=(code_memory[idx+2]<<8)|(code_memory[idx+1]);
}

void ajmp(unsigned short idx)
{
	PC=(((code_memory[idx]>>5)&0x07)<<8)|(code_memory[idx+1]);
}
void sjmp(unsigned short idx)
{
	/*	hey! add 2 before offset addition!	*/
	PC=2+idx+(signed char)code_memory[idx+1];
}




void acall(unsigned short idx)
{
	/*	push return value onto stack, little-endian	*/
	(*SP)++;
	data_memory[*SP]=(unsigned char)((PC+2)&0x00FF);
	(*SP)++;
	data_memory[*SP]=(unsigned char)((PC+2)>>8);

	PC=(((code_memory[idx]>>5)&0x07)<<8)|(code_memory[idx+1]);
}


void lcall(unsigned short idx)
{
	/*	push return value onto stack, little-endian	*/
	(*SP)++;
	data_memory[*SP]=(unsigned char)((PC+3)&0x00FF);
	(*SP)++;
	data_memory[*SP]=(unsigned char)((PC+3)>>8);

	PC=(code_memory[idx+2]<<8)|(code_memory[idx+1]);

}

void ret(unsigned short idx)
{
	PC=(data_memory[*SP]<<8)|(data_memory[(*SP)-1]);
	(*SP)-=2;
}


/*	stack operations	*/
void push(unsigned short idx)
{
	(*SP)++;
	data_memory[*SP]=data_memory[code_memory[idx+1]];
	PC+=2;
}
void pop(unsigned short idx)
{
	data_memory[code_memory[idx+1]]=data_memory[*SP];
	(*SP)--;
	PC+=2;
}


void empty(unsigned short idx)
{
	fprintf(stderr, "unknown instruction on %d\n", idx);
	exit(1);
}
/******************************************************************************/
/*				INICIALIZATION				      */
/******************************************************************************/

void init_mov_instructions(void)
{
	/*	mov a, rx	*/
	opcodes[0xE8].f=mov_a_rx;
	opcodes[0xE8].time=1;
	opcodes[0xEC]=opcodes[0xEB]=opcodes[0xEA]=opcodes[0xE9]=opcodes[0xE8];
	opcodes[0xEF]=opcodes[0xEE]=opcodes[0xED]=opcodes[0xE8];
	
	/*	mov rx, a	*/
	opcodes[0xF8].f=mov_rx_a;
	opcodes[0xF8].time=1;
	opcodes[0xFC]=opcodes[0xFB]=opcodes[0xFA]=opcodes[0xF9]=opcodes[0xF8];
	opcodes[0xFF]=opcodes[0xFE]=opcodes[0xFD]=opcodes[0xF8];


	/*	mov addr, rx	*/
	opcodes[0x88].f=mov_addr_rx;
	opcodes[0x88].time=2;
	opcodes[0x8C]=opcodes[0x8B]=opcodes[0x8A]=opcodes[0x89]=opcodes[0x88];
	opcodes[0x8F]=opcodes[0x8E]=opcodes[0x8D]=opcodes[0x88];

	/*	mov rx, addr	*/
	opcodes[0xA8].f=mov_rx_addr;
	opcodes[0xA8].time=2;
	opcodes[0xAC]=opcodes[0xAB]=opcodes[0xAA]=opcodes[0xA9]=opcodes[0xA8];
	opcodes[0xAF]=opcodes[0xAE]=opcodes[0xAD]=opcodes[0xA8];

	/*	mov rx, #imm8	*/
	opcodes[0x78].f=mov_rx_imm8;
	opcodes[0x78].time=1;
	opcodes[0x7C]=opcodes[0x7B]=opcodes[0x7A]=opcodes[0x79]=opcodes[0x78];
	opcodes[0x7F]=opcodes[0x7E]=opcodes[0x78];
}


void init_call_instructions(void)
{
	/*	lcall	*/
	opcodes[0x12].f=lcall;
	opcodes[0x12].time=2;

	/*	acall	*/
	opcodes[0x11].f=acall;
	opcodes[0x11].time=2;
	opcodes[0x91]=opcodes[0x71]=opcodes[0x51]=opcodes[0x31]=opcodes[0x11];
	opcodes[0xF1]=opcodes[0xD1]=opcodes[0xB1]=opcodes[0x11];
}


void init_stack_instructions(void)
{
	/*	push	*/
	opcodes[0xc0].f=push;
	opcodes[0xc0].time=2;

	/*	pop	*/
	opcodes[0xd0].f=pop;
	opcodes[0xd0].time=2;
}


void init_jump_instructions(void)
{
	/*	sjmp	*/
	opcodes[0x80].f=sjmp;
	opcodes[0x80].time=2;

	/*	ajmp	*/
	opcodes[0x01].f=ajmp;
	opcodes[0x01].time=2;
	opcodes[0x81]=opcodes[0x61]=opcodes[0x41]=opcodes[0x21]=opcodes[0x01];
	opcodes[0xE1]=opcodes[0xC1]=opcodes[0xA1]=opcodes[0x01];

	/*	ljmp	*/
	opcodes[0x02].f=ljmp;
	opcodes[0x02].time=2;

}

/*	TODO: RETI	*/

void init_ret_instructions(void)
{
	/*	ret	*/
	opcodes[0x22].f=ret;
	opcodes[0x22].time=2;
}





void init_instructions(void)
{
	int i;
	for (i=0; i<256; i++) {
		opcodes[i].f=empty;
		opcodes[i].time=10;
	}

	init_ret_instructions();
	init_jump_instructions();
	init_stack_instructions();
	init_call_instructions();
	init_mov_instructions();
}

