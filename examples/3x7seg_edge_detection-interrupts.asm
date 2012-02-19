;	zobrazuje 000 - 999 na 3*7seg displej,
;	cislo se navysi po detekci sestupne hrany.

horni	equ	0FFh
dolni	equ	000h

bcd_jd	EQU	40h
bcd_st	EQU	41h

video	EQU	20h

hrana	bit	P2.2
out	equ	P3

	org	0
	sjmp	main
	org	0Bh
	sjmp	ct0


main:
	acall	init
loop:
	jnb	hrana,$
	jb	hrana,$

	acall	increment
	acall	reload
	sjmp	loop

ct0:
	push	psw
	push	a

	mov	th0, #horni
	mov	tl0, #dolni

	acall	refresh

	pop	a
	pop	psw
reti



reload:
	mov	r1,#video
	mov	a,bcd_jd
	xchd	a,@r1

	inc	r1
	swap	a
	xchd	a,@r1
	
	inc	r1
	mov	a,bcd_st
	xchd	a,@r1
ret

increment:
	mov	a, bcd_jd
	add	a, #01
	da	a
	mov	bcd_jd, a

	mov	a, bcd_st
	addc	a, #00h
	da	a
	mov	bcd_st, a	
ret




init:
	mov	R0, #video
	mov	bcd_jd, #00h
	mov	bcd_st, #00h

	mov	video+0,#11100000b
	mov	video+1,#11010000b
	mov	video+2,#10110000b

	setb	ea
	setb	et0

	mov	tmod,#01h
	mov	th0,#horni
	mov	tl0,#dolni
	setb	tr0
	
ret	
	
	
refresh:
	mov	out, @R0
	inc	r0
	cjne	r0, #video + 3, _re
	mov	r0, #video
	_re:
ret
	

