led	equ	p1.0

	org	0
	sjmp	start
	org	03h
	sjmp	ex0

;	Start/reset handler
start:
	acall	init
	sjmp	$

init:
	setb	ea	; enable interrupts globally
	setb	ex0	; enable interrupts from external source 0
	setb	it0	; set it as falling edge source
ret

;	External interrupt 0 handler
ex0:
	cpl	led	; change 'led' state
reti

