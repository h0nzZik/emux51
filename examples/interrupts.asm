	org	0
	sjmp	start
	org	0Bh
	sjmp	ct0

start:
	acall	init
	sjmp	$

init:
	setb	ea
	setb	et0
	mov	r7, #1
	mov	tmod, r7
	mov	r7, #15
	setb	tr0
ret

ct0:
	push	psw
	djnz	r7, ct0_end
	mov	r7, #15

	cpl	P1.0

ct0_end:
	pop	psw
reti



