led	equ	p1.0

	org	0
	sjmp	start
	org	03h
	sjmp	ex0

start:
	acall	init
	sjmp	$

init:
	setb	ea
	setb	ex0
	setb	it0
ret

ex0:
	cpl	led
reti

