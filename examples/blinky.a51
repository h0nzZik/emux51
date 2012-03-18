led	equ	p1.0

	org	0		; reset
	sjmp	start
	org	0Bh		; timer 0
	sjmp	ct0

start:
	acall	init
	sjmp	$

;	Inicialize and run timer
init:
	setb	ea		; enable interrupts globally
	setb	et0		; enable interrupts from timer 0

	mov	r7, #15		; delay

	mov	tmod, #01h	; set timer 0 to mode 1
	setb	tr0		; start timer 0
ret

ct0:
	push	psw		; store program status word
	djnz	r7, ct0_end
	mov	r7, #15		; reload delay value

	cpl	led		; change led state
ct0_end:
	pop	psw		; restore program status word
reti



