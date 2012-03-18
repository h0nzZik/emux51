;	Keyboard example.

keyboard	equ	p2
led		equ	p0

	org	0
	sjmp	main

main:
	acall	CI
	mov	led,a
	sjmp	main



;..... test stlaceni klavesy            
;..... vstup : -
;..... vystup: Acc = 0 nebyla stlacena klavesa
;              Acc <>0 byla stlacena klavesa
;..... nici  : r7

KEY:	mov     a,#11011111b    ;klavesnice 3x4
	key_:
	mov     keyboard,a            ;zapis nulu do radky
	push    acc             ;schovej pozici nuly radku
	mov     r7,#50
	djnz    r7,$            ;chvili pockej
	key0:
	mov     a,keyboard            ;sejmi odezvu
	mov     r7,#50
	djnz    r7,$            ;opet pockej
	cjne    a,keyboard,key0	;potlaceni prechodovych deju
	mov     keyboard,#0ffh	;deaktivuj radek
	anl     a,#0fh          ;pouze sloupce
	cjne    a,#0fh,keyE     ;pokud ruzne, pak cy=1 a konec
	pop     acc             ;vyzvedni rotujici nulu
	setb    c
	rlc     a               ;posun o jednicku
	jc      key_            ;a pokracuj dalsim radkem
	clr     a               ;nebyl sjisten stisk klavesy
	ret
	keyE:
	mov     r7,a            ;odezva sloupce
	pop     acc             ;aktivovany radek
	anl     a,#0f0h
	orl     a,r7            ;radek a sloupec stlacene klavesy
ret
       
;..... sejmuti kodu stlacene klavesy
;..... vstup : -
;..... vystup: Acc = 0  pokud neni stlacena klavesa,
;              Acc <> 0 ASCII kod  stlacene klavesy 
;..... nici  : r7,dptr
KBD:	acall   KEY             ;otestuj klavesnici
	jnz     kbd_            ;pokracuj priracenim ASCII kodu
	ret                     ;navrat, pokud nebyla stlacena klavesa
;..... potlaceni chybnych stisku       
	kbd_:
	mov     dpl,a
	acall   KEY
	cjne    a,dpl,KBD       ;potlaceni chybovych stavu
;..... prevod na vnitrni kod           
	push    acc             ;uschovej pro radek
;..... prevod cisla sloupce z kodu 1zN na BC
	mov     r7,#0ffh
	kbd1:
	inc     r7
	rrc     a
	jc      kbd1
;.... prevod cisla radku z kodu 1zN na BC
	pop     acc
	swap    a       
	kbd2:
	rrc     a
	jnc     kbd3    ;ukonceni prevodu
	xch     a,r7
	add     a,#4
	xch     a,r7
	sjmp    kbd2
;..... prevod z vnitrniho kodu na ASCII        
	kbd3:
	mov     a,r7            ;vnitrni kod klavesy 0-15
	mov     dptr,#KEYASCII
	movc    a,@a+dptr       ;ASCII kod
ret
;.... tabulka hodnot stisknuteho hmatniku
KEYASCII:
	db	'x','x','x','x'         ;prvni  sloupec chybi
	db	'B','7','4','1'         ;druhy  sloupec
	db	'0','8','5','2'         ;treti  sloupec
	db	'A','9','6','3'         ;ctvrty sloupec
               
;..... console input : vstup znaku z klavesnice                
CI:	acall   KEY
	jnz     CI      ;cekej na pusteni predchozi
ci_:	acall   KBD
	jz      ci_     ;cekej na novy stisk 
ret
