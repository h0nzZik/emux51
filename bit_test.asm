	;timer test
start:
	mov	r7, #00h
	mov	th0, r7

	mov	r7, #00h
	mov	tl0, r7

	mov	r7, #1
	mov	tmod, r7

	mov	r7, #10h
	mov	tcon, r7

loop:
	jnb	TF0, $
	clr	TF0
	djnz	r7, loop
	mov	r7,#20h
	clr	TF0
	cpl	P1.0

sjmp loop

