/*
 * instructions.c - 8051 instruction set, not fully implemented.
 */
#include <stdlib.h>
#include <stdio.h>
#include <instructions.h>
#include <emux51.h>


opcode_t opcodes[256];


/*		<mov instructions>		*/
void mov_rx_imm8(unsigned short idx)
{
	unsigned char data;
	int reg;

	data=read_code(idx+1);
	reg=read_code(idx)&0x07;

	write_register(reg, data);
	PC+=2;
}
void mov_rx_addr(unsigned short idx)
{
	int reg;
	unsigned char data;

	reg=read_code(idx)&0x07;
	data=read_data(read_code(idx+1));
	write_register(reg, data);
	PC+=2;
}
void mov_addr_rx(unsigned short idx)
{
	unsigned char data;
	unsigned addr;

	data=read_register(read_code(idx)&0x07);
	addr=read_code(idx+1);
	write_data(addr, data);
	PC+=2;
}
void mov_a_rx(unsigned short idx)
{
	unsigned char data;

	data=read_register(read_code(idx)&0x07);
	write_Acc(data);
	PC++;
}
void mov_rx_a(unsigned short idx)
{
	int reg;
	unsigned char data;

	reg=read_code(idx)&0x07;
	data=read_Acc();
	write_register(reg, data);
	PC++;
}
void mov_a_imm8(unsigned short idx)
{
	unsigned char data;

	data=read_code(idx+1);
	write_Acc(data);
	PC+=2;
}
void mov_at_rx_imm8(unsigned short idx)
{
	unsigned short addr;
	unsigned char data;

	data=read_code(idx+1);
	addr=read_register(read_code(idx)&0x01);
	write_data(addr, data);
	PC+=2;
}
void mov_at_rx_a(unsigned short idx)
{
	unsigned short addr;

	addr=read_register(read_code(idx)&0x01);
	write_data(addr, read_Acc());
	PC+=2;
}
void mov_at_rx_addr(unsigned short idx)
{
	unsigned char src;
	unsigned char dest;

	src=read_code(idx+1);
	dest=read_register(read_code(idx)&0x01);
	write_data(dest, read_data(src));
	PC+=2;
}
void mov_a_at_rx(unsigned short idx)
{
	unsigned char addr;

	addr=read_register(read_code(idx)&0x01);
	write_Acc(read_data(addr));
	PC++;
}
void mov_a_addr(unsigned short idx)
{
	unsigned char addr;

	addr=read_code(idx+1);
	write_Acc(read_data(addr));
	PC+=2;
}
void mov_addr_a(unsigned short idx)
{
	unsigned char addr;

	addr=read_code(idx+1);
	write_data(addr, read_Acc());
	PC+=2;
}
void mov_c_addr(unsigned short idx)
{
	int bit;
	unsigned char addr;

	addr=read_code(idx+1);
	bit=test_bit(addr);
	write_carry(bit);

	PC+=2;
}
void mov_addr_c(unsigned short idx)
{
	int bit;
	unsigned char addr;

	bit=test_bit(CARRY);
	addr=read_code(idx+1);
	if (bit)
		set_bit(addr);
	else
		clr_bit(addr);
	PC+=2;
}

void mov_dptr_imm16(unsigned short idx)
{
	write_data(DPH, read_code(idx+1));
	write_data(DPL, read_code(idx+2));
	PC+=3;
}
void mov_addr_imm8(unsigned short idx)
{
	unsigned char addr;
	unsigned char data;

	addr=read_code(idx+1);
	data=read_code(idx+2);
	write_data(addr, data);
	PC+=3;
}
void mov_addr_at_rx(unsigned short idx)
{
	unsigned char src;
	unsigned char dest;

	dest=read_code(idx+1);
	src=read_register(read_code(idx)&0x01);
	write_data(dest, read_data(src));
	PC+=2;
}
void mov_addr_addr(unsigned short idx)
{
	unsigned char src;
	unsigned char dest;
	unsigned char data;

	src=read_code(idx+1);
	dest=read_code(idx+2);
	data=read_data(src);
	write_data(dest, data);
	PC+=3;
}

void movc_a_code_dptr(unsigned short idx)
{
	unsigned short addr;
	unsigned char data;

	addr=read_data(DPH)<<8;
	addr|=read_data(DPL);
	addr+=read_Acc();

	data=read_code(addr);


	write_Acc(data);
	PC++;
}
void movc_a_code_pc(unsigned short idx)
{
	idx+=read_Acc();
	write_Acc(read_code(idx));
	PC++;
}

void xch_a_addr(unsigned short idx)
{
	unsigned char adata;
	unsigned char addr;

	addr=read_code(idx+1);
	adata=read_Acc();

	write_Acc(read_data(addr));
	write_data(addr, adata);
	PC+=2;
}

void xch_a_rx(unsigned short idx)
{
	unsigned char reg;
	unsigned char adata;

	reg=read_code(idx)&0x07;
	adata=read_Acc();
	write_Acc(read_register(reg));
	write_register(reg, adata);
	PC++;
}


void xch_a_at_rx(unsigned short idx)
{
	unsigned char adata;
	unsigned char rdata;
	unsigned char addr;

	addr=read_register(read_code(idx)&1);
	adata=read_Acc();
	rdata=read_data(addr);

	write_data(addr, adata);
	write_Acc(rdata);
	PC++;
}

void xchd_a_at_rx(unsigned short idx)
{
	unsigned char adata;
	unsigned char rdata;
	unsigned char addr;

	addr=read_register(read_code(idx)&1);
	adata=read_Acc();
	rdata=read_data(addr);

	adata&=0xF0;
	adata|=rdata&0x0F;
	rdata&=0xF0;
	rdata|=read_Acc()&0x0F;

	write_data(addr, rdata);
	write_Acc(adata);
	PC++;
}


/*		</mov instructions>		*/


/*		<byte conditional jumps>		*/
void djnz_rx_addr(unsigned short idx)
{
	int reg;
	signed char addr;
	unsigned char rdata;

	reg=read_code(idx)&0x07;
	addr=read_code(idx+1);
	rdata=read_register(reg);
	rdata--;
	write_register(reg, rdata);

	PC+=2;
	if (rdata) {
		PC+=addr;
	}

}
void jz(unsigned int idx)
{
	signed char inc;

	inc=read_code(idx+1);
	PC+=2;
	if(!read_Acc()){
		PC+=inc;
	}
}
void jnz(unsigned int idx)
{
	signed char inc;

	inc=read_code(idx+1);
	PC+=2;
	if(read_Acc()){
		PC+=inc;
	}
}
void cjne_rx_imm8_addr(unsigned int idx)
{
	signed char inc;
	unsigned char data;
	unsigned char rdata;

	rdata=read_register(read_code(idx)&0x07);
	data=read_code(idx+1);
	inc=read_code(idx+2);
	PC+=3;
	if (rdata != data)
		PC+=inc;
	write_carry(rdata < data);
}
/*		</byte conditional jumps>		*/


/*		<setb instructions>		*/
void setb_addr(unsigned short idx)
{
	set_bit(read_code(idx+1));
	PC+=2;
}
void setb_c(unsigned short idx)
{
	clr_bit(CARRY);
	PC++;
}
/*		</setb instructions>		*/

/*		<clr instructions>		*/
void clr_addr(unsigned short idx)
{
	clr_bit(read_code(idx+1));
	PC+=2;
}
void clr_c(unsigned short idx)
{
	clr_bit(CARRY);
	PC++;
}
void clr_a(unsigned short idx)
{
	write_Acc(0);
	PC++;
}

void cpl_addr(unsigned short idx)
{
	unsigned char addr;
	addr=read_code(idx+1);

	if (test_bit(addr)) {
		clr_bit(addr);
	} else {
		set_bit(addr);
	}
	PC+=2;
}
void cpl_c(unsigned short idx)
{
	neg_bit(CARRY);
	PC++;
}
void cpl_a(unsigned short idx)
{
	write_Acc(~read_Acc());
	PC++;
}
/*		</clr instructions>		*/

/*		<bit conditional jumps>		*/
void jb(unsigned short idx)
{

	signed char inc;
	unsigned char addr;

	inc=read_code(idx+2);
	addr=read_code(idx+1);
	PC+=3;

	if(test_bit(addr)){
		PC+=inc;
	}
}
void jbc(unsigned short idx)
{
	signed char inc;
	unsigned char addr;

	inc=read_code(idx+2);
	addr=read_code(idx+1);
	PC+=3;

	if(test_bit(addr)){
		PC+=inc;
		clr_bit(addr);
	}
}


void jnb(unsigned short idx)
{
	signed char inc;
	unsigned char addr;

	addr=read_code(idx+1);
	inc=read_code(idx+2);


	PC+=3;
	if(!test_bit(addr)){
		PC+=inc;
	}
}
void jc(unsigned short idx)
{
	signed char inc;

	inc=read_code(idx+1);
	PC+=2;

	if(test_bit(CARRY)){
		PC+=inc;
	}

}
void jnc(unsigned short idx)
{
	signed char inc;

	inc=read_code(idx+1);
	PC+=2;

	if(!test_bit(CARRY)){
		PC+=inc;
	}

}
/*	</bit conditional jumps>	*/




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
	PC+=2;
	PC+=(signed char)code_memory[idx+1];
}




void acall(unsigned short idx)
{
	/*	push return adress onto stack, little-endian	*/
	push((PC+2)&0x00FF);
	push((PC+2)>>8);

	PC=(((code_memory[idx]>>5)&0x07)<<8)|(code_memory[idx+1]);
}


void lcall(unsigned short idx)
{
	/*	push return adress onto stack, little-endian	*/
	push((PC+2)&0x00FF);
	push((PC+2)>>8);

	PC=(code_memory[idx+2]<<8)|(code_memory[idx+1]);

}

void ret(unsigned short idx)
{
	PC=(data_memory[data_memory[SP]]<<8)|(data_memory[data_memory[SP]-1]);
	data_memory[SP]-=2;
}

void reti(unsigned short idx)
{
	PC=pop()<<8|pop();
	if (interrupt_state&0x02)
		interrupt_state&=~0x02;
	else
	if (interrupt_state&0x01)
		interrupt_state&=~0x01;
	else
	/*	wtf?	*/
		printf("[emux51]\treti from normal state\n");
}


/*	stack operations	*/
void push_addr(unsigned short idx)
{
	unsigned char addr;

	addr=read_code(idx+1);
	push(read_data(addr));
	PC+=2;
}
void pop_addr(unsigned short idx)
{
	unsigned char addr;

	addr=read_code(idx+1);
	write_data(addr, pop());
	PC+=2;
}



/*	aritmetic instructions	*/
void add_a_imm8(unsigned short idx)
{
	unsigned char increment;

	increment=read_code(idx+1);
	increment+=read_code(idx)&0x10;
	add_Acc(increment);
	PC+=2;
}
void add_a_addr(unsigned short idx)
{
	unsigned char increment;

	increment=read_data(read_code(idx+1));
	increment+=read_code(idx)&0x10;
	add_Acc(increment);
	PC+=2;
}
void add_a_at_rx(unsigned short idx)
{
	unsigned char increment;

	increment=read_data(read_register(read_code(idx)&0x01));
	increment+=read_code(idx)&0x10;
	add_Acc(increment);
	PC++;
}

void add_a_rx(unsigned short idx)
{
	unsigned char increment;

	increment=read_register(read_code(idx)&0x07);
	increment+=read_code(idx)&0x10;
	add_Acc(increment);
	PC++;
}

void inc_rx(unsigned short idx)
{
	unsigned char reg;
	unsigned char data;

	reg=read_code(idx)&0x07;
	data=read_register(reg);
	data++;
	write_register(reg, data);
	PC++;
}

void anl_a_addr(unsigned short idx)
{
	unsigned char data;

	data=read_data(read_code(idx+1));
	data&=read_Acc();
	write_Acc(data);
	PC+=2;
}
void anl_addr_a(unsigned short idx)
{
	unsigned char data;
	unsigned char addr;

	addr=read_code(idx+1);
	data=read_data(addr);
	data&=read_Acc();
	write_data(addr, data);
	PC+=2;
}
void anl_a_imm8(unsigned short idx)
{
	unsigned char mask;
	unsigned char acc;

	mask=read_code(idx+1);
	acc=read_Acc();
	acc&=mask;
	write_Acc(acc);
	PC+=2;
}
void anl_a_at_rx(unsigned short idx)
{
	unsigned char mask;
	unsigned char acc;

	mask=read_data(read_register(read_code(idx)&0x01));
	acc=read_Acc();
	acc&=mask;
	write_Acc(acc);
	PC++;
}

void anl_a_rx(unsigned short idx)
{
	unsigned char mask;
	unsigned char acc;

	mask=read_register(read_code(idx)&0x07);
	acc=read_Acc();
	acc&=mask;
	write_Acc(acc);
	PC++;
}
void anl_addr_imm8(unsigned short idx)
{
	unsigned char addr;
	unsigned char mask;
	unsigned char data;

	addr=read_code(idx+1);
	mask=read_code(idx+2);
	data=read_data(addr);
	data&=mask;
	write_data(addr, mask);
	PC+=3;
}
void anl_c_addr(unsigned short idx)
{
	int bit;
	int c;

	bit=test_bit(read_code(idx+1));
	c=test_bit(CARRY);
	c&=bit;
	write_carry(c);
	PC+=2;
}
void anl_c_naddr(unsigned short idx)
{
	int bit;
	int c;

	bit=test_bit(read_code(idx+1));
	c=!test_bit(CARRY);
	c&=bit;
	write_carry(c);
	PC+=2;

}
void da_a(unsigned short idx)
{
	unsigned int a;
	a=read_Acc();
	if ((a&0x0F)>0x09 || test_bit(ACARRY))
		a+=0x06;
	if ((a&0xF0)>0x90 || test_bit(CARRY))
		a+=0x60;
	write_carry(a>>8?1:0);
	write_Acc(a&0xFF);
	PC++;
}
void swap_a(unsigned short idx)
{
	unsigned char a;
	a=read_Acc()>>4;
	a|=read_Acc()<<4;
	write_Acc(a);
	PC++;
}

void nop(unsigned short idx)
{
	PC++;
}

void empty(unsigned short idx)
{
	fprintf(stderr, "[emux]\tunknown instruction at %u\n", idx);
	fprintf(stderr, "[emux]\t\topcode: 0x%x\n", read_code(idx));
	exit(1);
}

/******************************************************************************/
/*				INITIALIZATION				      */
/******************************************************************************/

void init_mov_instructions(void)
{
	int i;
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
	for (i=0x88; i<=0x8F; i++) {
		opcodes[i].f=mov_addr_rx;
		opcodes[i].time=2;
	}

	/*	mov rx, addr	*/
	for (i=0xA8; i<=0xAF; i++) {
		opcodes[i].f=mov_rx_addr;
		opcodes[i].time=2;
	}

	/*	mov rx, #imm8	*/
	for (i=0x78; i<=0x7F; i++) {
		opcodes[i].f=mov_rx_imm8;
		opcodes[i].time=1;
	}

	/*	mov a, #imm8	*/
	opcodes[0x74].f=mov_a_imm8;
	opcodes[0x74].time=1;

	/*	mov @rx, #imm8	*/
	opcodes[0x76].f=mov_at_rx_imm8;
	opcodes[0x76].time=1;
	opcodes[0x77]=opcodes[0x76];

	/*	mov @rx, a	*/
	opcodes[0xF6].f=mov_at_rx_a;
	opcodes[0xF6].time=1;
	opcodes[0xF7]=opcodes[0xF6];

	/*	mov @rx, addr	*/
	opcodes[0xA6].f=mov_at_rx_addr;
	opcodes[0xA6].time=2;
	opcodes[0xA7]=opcodes[0xA6];

	/*	mov a, @rx	*/
	opcodes[0xE6].f=mov_a_at_rx;
	opcodes[0xE6].time=1;
	opcodes[0xE7]=opcodes[0xE6];

	/*	mov a, addr	*/
	opcodes[0xE5].f=mov_a_addr;
	opcodes[0xE5].time=1;

	/*	mov c, addr	*/
	opcodes[0xA2].f=mov_c_addr;
	opcodes[0xA2].time=2;

	/*	mov addr, c	*/
	opcodes[0x92].f=mov_addr_c;
	opcodes[0x92].time=2;

	/*	mov dptr, #imm16*/
	opcodes[0x90].f=mov_dptr_imm16;
	opcodes[0x90].time=2;

	/*	mov addr, #imm8	*/
	opcodes[0x75].f=mov_addr_imm8;
	opcodes[0x75].time=2;

	/*	mov addr, @rx	*/
	opcodes[0x86].f=mov_addr_at_rx;
	opcodes[0x86].time=2;
	opcodes[0x86]=opcodes[0x86];

	/*	mov addr, addr	*/
	opcodes[0x85].f=mov_addr_addr;
	opcodes[0x86].time=2;

	/*	mov a, @a+dptr	*/
	opcodes[0x93].f=movc_a_code_dptr;
	opcodes[0x93].time=2;

	/*	mov a, @a+pc	*/
	opcodes[0x83].f=movc_a_code_pc;
	opcodes[0x83].time=2;
	/*	mov addr, a	*/
	opcodes[0xF5].f=mov_addr_a;
	opcodes[0xF5].time=1;



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
	opcodes[0xc0].f=push_addr;
	opcodes[0xc0].time=2;

	/*	pop	*/
	opcodes[0xd0].f=pop_addr;
	opcodes[0xd0].time=2;
}


void init_jump_instructions(void)
{
	int i;
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

	/*	djnz	*/
	for (i=0xD8; i<=0xDF; i++) {
		opcodes[i].f=djnz_rx_addr;
		opcodes[i].time=2;
	}
	/*	cjne	*/
	for (i=0xB8; i<=0xBF; i++) {
		opcodes[i].f=cjne_rx_imm8_addr;
		/*	CHECK:	*/
		opcodes[i].time=2;
	}

}


void init_setb(void)
{
	/*	setb addr	*/
	opcodes[0xD2].f=setb_addr;
	opcodes[0xD2].time=1;
	/*	setb c		*/
	opcodes[0xD3].f=setb_c;
	opcodes[0xD3].time=1;
}

void init_clr(void)
{
	/*	clr addr	*/
	opcodes[0xC2].f=clr_addr;
	opcodes[0xC2].time=1;
	/*	clr c		*/
	opcodes[0xC3].f=clr_c;
	opcodes[0xC3].time=1;
	/*	clr a		*/
	opcodes[0xE4].f=clr_a;
	opcodes[0xE4].time=1;

}


void init_cpl(void)
{
	opcodes[0xB2].f=cpl_addr;
	opcodes[0xB2].time=1;

	opcodes[0xB3].f=cpl_c;
	opcodes[0xB3].time=1;

	opcodes[0xF4].f=cpl_a;
	opcodes[0xF4].time=1;

}

void init_bit_cond_jumps(void)
{
	/*	jbc	*/
	opcodes[0x10].f=jbc;
	opcodes[0x10].time=1;
	/*	jb	*/
	opcodes[0x20].f=jb;
	opcodes[0x20].time=1;
	/*	jnb	*/
	opcodes[0x30].f=jnb;
	opcodes[0x30].time=1;


}

void init_ret_instructions(void)
{
	/*	ret	*/
	opcodes[0x22].f=ret;
	opcodes[0x22].time=2;
	/*	reti	*/
	opcodes[0x32].f=reti;
	opcodes[0x32].time=2;
}

void init_aritm_instructions(void)
{
	int i;

	/*	add	*/
	opcodes[0x24].f=add_a_imm8;
	opcodes[0x24].time=1;

	opcodes[0x25].f=add_a_addr;
	opcodes[0x25].time=1;

	opcodes[0x26].f=add_a_at_rx;
	opcodes[0x26].time=1;
	opcodes[0x27]=opcodes[0x26];

	for(i=0x28; i<0x30; i++) {
		opcodes[i].f=add_a_rx;
		opcodes[i].time=1;
	}

	/*	addc	*/
	opcodes[0x34].f=add_a_imm8;
	opcodes[0x34].time=1;

	opcodes[0x35].f=add_a_addr;
	opcodes[0x35].time=1;

	opcodes[0x36].f=add_a_at_rx;
	opcodes[0x36].time=1;
	opcodes[0x37]=opcodes[0x26];

	for(i=0x38; i<0x40; i++) {
		opcodes[i].f=add_a_rx;
		opcodes[i].time=1;
	}

	/*	anl	*/
	opcodes[0x52].f=anl_addr_a;
	opcodes[0x52].time=1;

	opcodes[0x53].f=anl_addr_imm8;
	opcodes[0x53].time=2;

	opcodes[0x54].f=anl_a_imm8;
	opcodes[0x54].time=1;

	opcodes[0x55].f=anl_a_addr;
	opcodes[0x55].time=1;

	opcodes[0x56].f=anl_a_at_rx;
	opcodes[0x56].time=1;
	opcodes[0x57]=opcodes[0x56];

	for(i=0x58; i<0x60; i++) {
		opcodes[i].f=anl_a_rx;
		opcodes[i].time=1;
	}

	opcodes[0x81].f=anl_c_addr;
	opcodes[0x81].time=2;

	opcodes[0xB0].f=anl_c_naddr;
	opcodes[0xB0].time=2;

	/*	inc rx	*/
	for (i=0x08; i<=0x0F; i++) {
		opcodes[i].f=inc_rx;
		/*	CHECK	*/
		opcodes[i].time=1;
	}
}

void init_xchange(void)
{
	int i;

	opcodes[0xD6].f=xchd_a_at_rx;
	opcodes[0xD6].time=1;
	opcodes[0xD7]=opcodes[0xD6];

	opcodes[0xC5].f=xch_a_addr;
	opcodes[0xC5].time=1;

	for(i=0xC8; i<=0xCf; i++) {
		opcodes[i].f=xch_a_rx;
		opcodes[i].time=1;
	}

	opcodes[0xC6].f=xch_a_at_rx;
	opcodes[0xC6].time=1;
	opcodes[0xC7]=opcodes[0xC6];

	opcodes[0xC4].f=swap_a;
	opcodes[0xC4].time=1;
}


void init_instructions(void)
{
	int i;
	for (i=0; i<256; i++) {
		opcodes[i].f=empty;
		opcodes[i].time=10;
	}

	/*	NOP	*/
	opcodes[0x00].f=nop;
	opcodes[0x00].time=1;
	opcodes[0xD4].f=da_a;
	opcodes[0xD4].time=1;

	init_xchange();
	init_ret_instructions();
	init_jump_instructions();
	init_stack_instructions();
	init_call_instructions();
	init_mov_instructions();
	init_clr();
	init_setb();
	init_cpl();
	init_bit_cond_jumps();
	init_aritm_instructions();
}
