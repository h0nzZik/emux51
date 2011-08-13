;	zobrazuje 000 - 999 na 3*7seg displej,
;	cislo se navysi po detekci sestupne hrany.
;	http://openpaste.org/24078/

bcd_jd	EQU	40h
bcd_st	EQU	41h
tmp	equ	30h

video	EQU	20h

hrana	bit	P2.2
vys	equ	P3

	org	0
	mov	R0, #video
	mov	bcd_jd, #00h
	mov	bcd_st, #00h
	mov	R1, #tmp
nula:	jb	hrana, jedna
	acall	zobraz
	djnz	R7, $
	sjmp	nula
jedna:	jnb	hrana, dalsi
	acall	zobraz
	djnz	r7, $
	sjmp	jedna
dalsi:	acall	bcd_inc
	mov	@r1, bcd_jd
	mov	a, #11100000b
	xchd	a, @r1
	mov	#video, a
	
	xch	a, @r1
	swap	a
	xch	a, @r1
	mov	a, #11010000b
	xchd	a, @r1
	mov	#video+1, a
	
	mov	@r1, bcd_st
	mov	a, #10110000b
	xchd	a, @r1
	mov	#video+2, a
	sjmp	nula	
	
	
	
	
zobraz:	push	PSW
	mov	vys, @R0
	inc	R0
	cjne	R0, #video + 3, _sk
	mov	R0, #video
_sk:	pop	PSW
	ret
	
bcd_inc:
	mov	a, bcd_jd
	add	a, #01
	da	a
	mov	bcd_jd, a
	mov	a, bcd_st
	addc	a, #00h
	da	a
	mov	bcd_st, a	
	ret

END
