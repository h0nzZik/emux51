
video	equ	20h

mtrx	equ	p3

pth	equ	0FCh
ptl	equ	018h


	org	0
	sjmp	main
	org	0Bh
	sjmp	ct0

main:
	acall	init
loop:
	sjmp	loop


init:
	mov	r0,#video
	
	mov	video+0,#11111110b
	mov	video+1,#11111100b
	mov	video+2,#11111000b
	mov	video+3,#11110000b
	mov	video+4,#11100000b


	setb	ea
	setb	et0

	mov	th0,#pth
	mov	tl0,#ptl
	mov	tmod,#01h
	setb	tr0

ret




ct0:
	;save originals
	push	psw
	push	a
	;reload timer 0
	mov	th0,#pth
	mov	tl0,#ptl
	;5x7 matrix display refresh
;	acall	refresh
	acall	simple_refresh
	;restore originals	
	pop	a
	pop	psw
reti


simple_refresh:
	cjne	r0,#video+5,_sref1
	mov	r0,#video
	ret
_sref1:	mov	mtrx,@r0
	mov	r7,#10
	djnz	r7,$
	clr	mtrx.7
	inc	r0
ret


refresh:
	cjne	r0,#video+5,_ref1
	setb	mtrx.7
	inc	r0
	ret
_ref1:	cjne	r0,#video+6,_ref2
	clr	mtrx.7
	mov	r0,#video
		ret
_ref2:	mov	mtrx,@r0
	mov	r7,#10
	djnz	r7,$
	clr	mtrx.7
	inc	r0
ret

