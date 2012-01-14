led	equ	p1.0

	org	0		; reset
	sjmp	start
	org	0Bh		; timer 0
	sjmp	ct0

start:
	acall	init
	sjmp	$

init:
	setb	ea		; enable interrupts
	setb	et0		; enable interrupts from timer 0

	mov	r7, #15		; delay

	mov	tmod, #01h	; timer 0 in mode 1
	setb	tr0		; start timer
ret

ct0:
	push	psw
	djnz	r7, ct0_end
	mov	r7, #15		; reload delay value

	cpl	led		; led blink
ct0_end:
	pop	psw
reti



