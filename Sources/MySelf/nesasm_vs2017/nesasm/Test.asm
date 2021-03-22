;;---CODE START---;;

	.inesprg 1
	.inesmap 0
	.inesmir 1
	.ineschr 0  ; note that we have no CHR-ROM bank in this code
	
	.bank 1
	.org $FFFA
	.dw 0 ; no VBlank routine
	.dw Start 
	.dw 0 ; we'll get to this at a later time

	.bank 0
	.org $8000
; note that I just copy/pasted code from the register sections
Start:
	lda #$FF   ; Like $4000 we just write all 1s 'cause
	sta $400C  ; we don't mind all the stuff in there being "on".

	lda #$50   ; play rate of 5 (5), lower sounding mode (0)
	sta $400E

	lda #$AB
	sta $400F

	lda #%00001000  ; enable Noise Channel
	sta $4015

infinite:
	jmp infinite

	;;--- END OF CODE FILE ---;;