; zobrazuje 000 - 999 na 3*7seg displej,
; cislo se navysi po detekci sestupne hrany.

; memory
bcd_jd	EQU	40h
bcd_st	EQU	41h
video	EQU	20h

; I/O
inb	bit	P1.0
out	equ	P2

; timer
upper	EQU	0FDh
lower	EQU	66h

	org 0
	sjmp	main
	org	0Bh
	sjmp	ct0

main:
	acall	init
m_loop:
	jb	inb, $
	jnb	inb, $
	acall	bcd_inc
	acall	napln
	sjmp	m_loop


ct0:
	push	psw
	mov	th0, #upper
	mov	tl0, #lower
	acall	zobraz
 	pop	psw
reti

init:
; video
	mov video+0, #11100000b
	mov video+1, #11010000b
	mov video+2, #10110000b
	mov video+3, #01110000b
	mov r0, #video

	mov bcd_jd, #0
	mov bcd_st, #0

	setb	ea
	setb	et0

	mov th0, #upper
	mov tl0, #lower
	mov	tmod, #01h
	setb	tr0
ret

zobraz:
	mov	out, @R0
	inc	r0
	cjne	r0, #video + 4, _sk
	mov	r0, #video
	_sk:
ret

napln:
	mov	r1, #video
	mov	a, bcd_jd
	xchd	a, @r1
	inc	r1
	swap	a
	xchd	a, @r1
	inc	r1
	mov	a, bcd_st
	xchd	a, @r1
	inc	r1
	swap	a
	xchd	a, @r1
ret
 
bcd_inc:
	mov a, bcd_jd
	add a, #01
	da a
	mov bcd_jd, a
	mov a, bcd_st
	addc a, #00h
	da a
	mov bcd_st, a 
ret

END
