;	Shows number from range 000 - 999 at 3x7seg display,
;	increasing it after falling edge.
;	Without interrupts.

bcd_low	equ	40h		;	units and tens (packed BCD format)
bcd_hig	equ	41h		;	hundreds
tmp	equ	30h

video	equ	20h		;	video memory

input	equ	P2.2		;	edge
out	equ	P3		;	3x7seg dynamic display

	org	0

	; clear counters and set pointer to 'video'
	mov	r0, #video
	mov	bcd_low, #00h
	mov	bcd_hig, #00h

	; wait for log. 1
zero:	jb	input, one
	acall	refresh		; refresh display
	djnz	R7, $		; short delay
	sjmp	zero

	; wait for log. 0
one:	jnb	input, next
	acall	refresh		; refresh display
	djnz	r7, $		; short delay
	sjmp	one

	; increment value and reload video memory
next:	acall	bcd_inc
	mov	@r1, bcd_low
	mov	a, #11100000b
	xchd	a, @r1
	mov	#video, a
	
	xch	a, @r1
	swap	a
	xch	a, @r1
	mov	a, #11010000b
	xchd	a, @r1
	mov	#video+1, a
	
	mov	@r1, bcd_hig
	mov	a, #10110000b
	xchd	a, @r1
	mov	#video+2, a
	sjmp	zero
	
	
	
;	Refresh 3x7seg display	
refresh:
	push	PSW			; store flags
	mov	out, @R0
	inc	R0
	cjne	R0, #video + 3, _sk
	mov	R0, #video
_sk:	pop	PSW			; restore flags
	ret

;	Increment bcd counter
bcd_inc:
	mov	a, bcd_low
	add	a, #01
	da	a
	mov	bcd_low, a
	mov	a, bcd_hig
	addc	a, #00h
	da	a
	mov	bcd_hig, a	
	ret

END
