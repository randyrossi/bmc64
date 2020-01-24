!to "delaykey.8k",cbm

; cartridge code appears at $8000
*=$8000

; this is the cartridge header
; this has BASIC inits nopsled'd out

	!byte    $09, $80, $25, $80, $c3, $c2, $cd, $38, $30, $8e, $16, $d0, $20, $a3, $fd, $20
	!byte    $50, $fd, $20, $15, $fd, $20, $5b, $ff, $58, $ea, $ea, $ea, $ea, $ea, $ea, $ea
	!byte    $ea, $ea, $ea, $ea, $ea

; vars in memory
; the ? means they don't get assembled into the image and make a huge blank cart!
; yet you can still refer to them in your program

; your code starts here
*=$8025

	;PREPARE_SID_CHIP
	LDA     #$0D    ; Volume level 14 (of 16)
	STA     $D418   ; SID 1 volume register
	LDA     #$06    ; attack=0l decay=6
	STA     $D405   ; SID attack/decay register voice 1
	LDA     #$00    ; sustain=0, release=0
	STA     $D406   ; SID sustain/release register voice 1
	JMP     MAINROUTINE

MAINTEXT:
        !BYTE   147,5           ;CLEAR SCREEN AND WHITE
	!PET    "key delay test by the 8-bit guy."

MAINROUTINE
        LDA     #$00
	STA     $D020
	STA     $D021
	LDY     #$00
TEXT1:
        LDA     MAINTEXT,Y
	JSR     $FFD2           ;CHAR OUT
	INY
	CPY     #34
	BNE     TEXT1

LISTEN:
        JSR     $F142
	CMP     #$00
	BEQ     LISTEN
	INC     $D020
	INC     $D021
	LDA     #%00100000
	STA     $D404   ; control gate voice 1
	LDA     #67
	STA     $D401   ; set freq high byte voice 1
	LDA     #135
	STA     $D400   ; set freq low byte voice 1
	LDA     #%00100001
	STA     $D404   ; control gate voice 1
	JMP     LISTEN

; put this after your program code
; to fill up the image to make an 8k cart
* = $9fff                     ; fill up to -$9fff
        !byte 0


