        video  EQU     20h
        vys    equ     P3

	ya	EQU	3Fh
	yy	EQU	40h
	mm	EQU	41h
	dy	EQU	42h



	org	0
	mov	R0, #video
	mov	ya, #19h
	mov	yy, #70h
	mov	mm, #01h
	mov	dy, #01h
	acall	bcd_to_video
main:
	djnz    r7, $
	acall	zobraz
	sjmp	main

zobraz:        push    PSW
       mov     vys, @R0
       inc     R0
       cjne    R0, #video + 8, _sk	;	pouze 6 vyuzivanych segmentu
       mov     R0, #video
_sk:   pop     PSW
       ret

bcd_to_video:

	mov	r1, #video
	mov	a, dy
	swap	a
	mov	@r1, #70h
	xchd	a, @r1
	inc	r1
	swap	a
	mov	@r1, #60h
	xchd	a, @r1

	inc	r1
	mov	a, mm
	swap	a
	mov	@r1, #50h
	xchd	a, @r1
	inc	r1
	swap	a
	mov	@r1, #40h
	xchd	a, @r1

	inc	r1
	mov	a, ya
	swap	a
	mov	@r1, #30h
	xchd	a, @r1
	inc	r1
	swap	a
	mov	@r1, #20h
	xchd	a, @r1

	inc	r1
	mov	a, yy
	swap	a
	mov	@r1, #10h
	xchd	a, @r1
	inc	r1
	swap	a
	mov	@r1, #00h
	xchd	a, @r1

	ret
END
