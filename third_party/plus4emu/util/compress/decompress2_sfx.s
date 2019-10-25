
; assume running on the C16 (RAM ends at $4000)
.define SFX_C16_MODE            0
; do not verify checksum if this is set to any non-zero value
.define NO_CRC_CHECK            0
; do not read ahead one byte of compressed data
.define NO_READ_BUFFER          0
; select border effect type (1-3), or 0 to disable border effects and
; saving/restoring of the border color
.define BORDER_EFFECT_TYPE      2
; do not blank display, and do not save/restore $FF06
.define NO_BLANK_DISPLAY        0
; do not save and restore zeropage variables used by the decompressor
.define NO_ZP_SAVE_RESTORE      0
; do not restore RAM/ROM paging after decompression, return in RAM ($FF3F) mode
.define NO_ROM_ENABLE_RESTORE   0
; do not enable interrupts after decompression
.define SFX_NO_CLI              0
; start BASIC program after decompression
.define SFX_RUN_BASIC_PROGRAM   1

sfxStartAddress = $100d

        .setcpu "6502"
        .code

; base address of zeropage variables
.if NO_ZP_SAVE_RESTORE = 0
zpBaseAddress = $02
.else
zpBaseAddress = $de
.endif
zpBytesUsed = $0a

loadAddrLow = zpBaseAddress + $00
loadAddrHigh = zpBaseAddress + $01
copyAddrLow = zpBaseAddress + $02
copyAddrHigh = zpBaseAddress + $03
symbolsRemainingLow = zpBaseAddress + $00
symbolsRemainingHigh = zpBaseAddress + $01
decompressWriteAddrLow = zpBaseAddress + $02
decompressWriteAddrHigh = zpBaseAddress + $03
lzMatchReadAddrLow = zpBaseAddress + $04
lzMatchReadAddrHigh = zpBaseAddress + $05
tableEnd = zpBaseAddress + $04
decodedValueHigh = zpBaseAddress + $05
shiftRegister = zpBaseAddress + $06
crcValue = zpBaseAddress + $06
readByteBuffer = zpBaseAddress + $07
tmpLow = zpBaseAddress + $08
tmpHigh = zpBaseAddress + $09

loadEndAddrLow = $9d
loadEndAddrHigh = $9e
copyEndAddrLow = $37
copyEndAddrHigh = $38

slotBitsTable = $0204                   ; 3 * 52 bytes
slotBaseLowTable = $027c
slotBaseHighTable = $02b0

borderColor = $ff19

decompressCode1Size = decompressCode1End - decompressCode1Start
decompressCode2Size = decompressCode2End - decompressCode2Start
tmpLength1 = decompressCode1Size * (decompressCode1Size > decompressCode2Size)
tmpLength2 = decompressCode2Size * (decompressCode1Size <= decompressCode2Size)
decompressCodeCopyLength = tmpLength1 + tmpLength2
decompressCode1Start_ = sfxLoaderEnd
decompressCode1End_ = decompressCode1Start_ + decompressCode1Size
decompressCode2Start_ = decompressCode1End_
decompressCode2End_ = decompressCode2Start_ + decompressCode2Size

        .word $1001, $100b, $000a
        .byte $9e, .sprintf("%d", sfxStartAddress)
        .res sfxStartAddress - $100a, $00
        .org sfxStartAddress

        .proc main
        sei
        cld
        ldx #decompressCodeCopyLength
l1:     lda decompressCode1Start_ - 1, x
        sta decompressCode1Start - 1, x
        lda decompressCode2Start_ - 1, x
        sta decompressCode2Start - 1, x
        dex
        bne l1
        .if SFX_C16_MODE <> 0
        stx copyEndAddrLow
        lda #$40
        sta copyEndAddrHigh
        .endif
        dex
        txs
        .if NO_ZP_SAVE_RESTORE = 0
l2:     inx
        lda zpBaseAddress, x            ; save zeropage variables
        pha
        cpx #<(zpBytesUsed - 1)
        bcc l2
        ldx #$00
        .else
        inx
        sec
        .endif
        .if NO_BLANK_DISPLAY = 0
        lda $ff06                       ; save TED registers
        stx $ff06
        pha
        .endif
        .if BORDER_EFFECT_TYPE <> 0
        lda borderColor
        pha
        .endif
        stx $ff3f
        dex
        .if NO_CRC_CHECK = 0
        stx crcValue
        .endif
        lda loadEndAddrLow
        sbc #<decompressCode2End_       ; NOTE: carry is set by the loop above
        tay
        lda #<decompressCode2End_
        sta loadAddrLow
        txa                             ; X = $ff
        adc loadEndAddrHigh
        sta loadAddrHigh
        tya                             ; assume loadEndAddrHigh is never zero,
        eor #$ff                        ; so carry is always set
        adc copyEndAddrLow
        sta copyAddrLow
        sta readAddrLow
        txa                             ; X = $ff
        adc copyEndAddrHigh
        tax                             ; = copyAddrHigh
        tya
        beq l5
l3:     stx copyAddrHigh
l4:     dey
        lda (loadAddrLow), y
        sta (copyAddrLow), y
        .if NO_CRC_CHECK = 0
        eor crcValue
        asl
        adc #$ac
        sta crcValue
        .endif
        tya
        bne l4
l5:     dec loadAddrHigh
        dex
        lda loadAddrHigh
        cmp #>(decompressCode2End_ - $0100)
        bne l3
        inx
        stx readAddrHigh
        lda #$80                        ; NOTE: this also initializes the
        .if NO_CRC_CHECK = 0
        cmp crcValue                    ; shift register (which is the same
        bne l6                          ; variable)
        .else
        sta shiftRegister
        .endif
        .if NO_READ_BUFFER = 0
        jsr read8Bits                   ; skip CRC byte
        .endif
        jmp decompressData
l6:
        .if NO_CRC_CHECK = 0
        sty $ff3e                       ; reset machine on CRC error
        jmp ($fffc)
        .endif
        .endproc

sfxLoaderEnd:

; -----------------------------------------------------------------------------

        .org $0130

decompressCode1Start:

        .proc read2Bits
        lda #$40
        .byte $2c
        .endproc

        .proc read8Bits
        lda #$01
        .endproc

        .proc readBits
l1:     asl shiftRegister
        bne :+
        jsr readCompressedByte
:       rol
        bcc l1
        rts
        .endproc

        .proc readCompressedByte
        sta tmpLow
        .endproc

        .proc readLiteralByte
        inc readAddrLow
        bne :+
        .if BORDER_EFFECT_TYPE = 1
        dec borderColor
        .endif
        .if BORDER_EFFECT_TYPE = 2
        inc borderColor
        .endif
        inc readAddrHigh
:
        .if NO_READ_BUFFER = 0
        lda readByteBuffer
        pha
        .endif
l1:     lda $ffff                       ; = readAddrLow, readAddrHigh
        .if BORDER_EFFECT_TYPE = 3
        sta borderColor
        .endif
        .if NO_READ_BUFFER = 0
        sta readByteBuffer
        pla
        .endif
        bcc writeByte
        rol
        sta shiftRegister
        lda tmpLow
        rts
        .endproc

readAddrLow = readLiteralByte::l1 + 1
readAddrHigh = readLiteralByte::l1 + 2

        .proc decompressDataBlock_
l1:     asl shiftRegister               ; check next character or match code
        bne :+                          ; assume X = 0 (set by copyData loop)
        jsr readCompressedByte
:       bcc l2
        inx
        cpx #$09
        bcc l1
l2:     txa
        beq copyData                    ; literal byte ?
        dex
        .if BORDER_EFFECT_TYPE = 2
        inc borderColor
        .endif
        bcc copyLZMatch
        jsr read8Bits                   ; literal sequence, get length - 17
        adc #$0f                        ; convert length to 16..271 range
        tax
        tya
        rol
l3:     sta tmpHigh                     ; = copyData
        inx
        inc tmpHigh
l4:     bcc readLiteralByte
        lda (lzMatchReadAddrLow), y
l5:     sta (decompressWriteAddrLow), y ; store decompressed data ( = writeByte)
        dex
        beq l7
l6:     iny
        bne l4
        inc decompressWriteAddrHigh
        inc lzMatchReadAddrHigh         ; NOTE: this assumes that the read
        bne l4                          ; address never overflows
l7:     dec tmpHigh
        bne l6
        tya
        ldy #$00
        sec
        adc decompressWriteAddrLow
        sta decompressWriteAddrLow
        bcc :+
        inc decompressWriteAddrHigh
:       dec symbolsRemainingLow
        bne l1
        dec symbolsRemainingHigh
        bne l1
        pla                             ; return with A=0, Z=1 if there are
        rts                             ; more blocks to be decompressed
        .endproc

copyData = decompressDataBlock_::l3
writeByte = decompressDataBlock_::l5

        .proc copyLZMatch
        jsr readEncodedValue            ; LZ match: decode length value
        pha
        ldx decodedValueHigh
        stx tmpHigh
        bne l1                          ; check length:
        tax
        lsr
        beq l2                          ; 1 or 2 bytes ?
l1:     ldx #$02                        ; 3 bytes or longer
l2:     lda prefixSizeTable, x
        jsr readBits
        adc tableOffsetTable, x
        tax
        jsr readEncodedValue            ; decode offset value
        eor #$ff                        ; calculate read address
        adc decompressWriteAddrLow
        sta lzMatchReadAddrLow
        lda decompressWriteAddrHigh
        sbc decodedValueHigh            ; assume that the read address never
        sta lzMatchReadAddrHigh         ; overflows, so carry is always set
        pla
        tax
        bcs copyData + 2                ; skip 'sta tmpHigh'
prefixSizeTable:
        .byte $40, $20, $08
tableOffsetTable:
        .byte $07, $0b, $13
        .endproc

offs3PrefixSize = copyLZMatch::prefixSizeTable + 2

decompressCode1End:

; -----------------------------------------------------------------------------

        .org $0335

decompressCode2Start:

        .proc readEncodedValue
        tya
        sty decodedValueHigh
        ldy slotBitsTable, x
        beq l2
l1:     asl shiftRegister
        bne :+
        jsr readCompressedByte
:       rol
        rol decodedValueHigh
        dey
        bne l1
l2:     clc
        adc slotBaseLowTable, x
        pha
        lda decodedValueHigh
        adc slotBaseHighTable, x
        sta decodedValueHigh
        pla                             ; return with LSB in A
        rts
        .endproc

        .proc decompressDataBlock
        ldx #$04                        ; NOTE: the caller must set Y to zero
:       jsr read8Bits                   ; read start address, and the number
        sta symbolsRemainingLow - 1, x  ; of symbols (literals and match
        dex                             ; codes) - 1
        bne :-
        tax                             ; X = (symbolsRemainingLow)
        jsr read2Bits
        lsr
        pha                             ; save last block flag
        bcs l1                          ; is compression enabled ?
        lda symbolsRemainingHigh        ; no compression, copy literal data
        iny
        sty symbolsRemainingLow
        sty symbolsRemainingHigh
        dey
        jmp copyData
l1:     inc symbolsRemainingLow         ; compression enabled:
        inc symbolsRemainingHigh
        jsr read2Bits                   ; select prefix size for 3-byte or
        tax                             ; longer matches (2 to 5 bits)
        inx
        inx
        lda #$01
        sty offs3PrefixSize
:       ror offs3PrefixSize             ; = $40, $20, $10, or $08
        asl
        dex
        bne :-
        adc #$14
        sta tableEnd                    ; = $18, $1c, $24, or $34
l2:     tya                             ; read all decode tables
        sta slotBaseLowTable, x
        beq l4
l3:     adc slotBaseLowTable - 1, x
        sta slotBaseLowTable, x
        lda tmpHigh
        adc slotBaseHighTable - 1, x
l4:     sta slotBaseHighTable, x
        lda #$10
        jsr readBits
        sta slotBitsTable, x
        tay
        iny
        lda #$00
        sta tmpHigh
:       rol
        rol tmpHigh
        dey
        bne :-
        inx                             ; check for end of:
        cpx #$08                        ; length table (8 slots),
        beq l2
        cpx #$0c                        ; offsets for 1-byte matches (4 slots),
        beq l2
        cpx #$14                        ; offsets for 2-byte matches (8 slots),
        beq l2
        cpx tableEnd                    ; offsets for longer matches
        bcc l3
        ldx #$00                        ; decompressDataBlock_ requires X = 0
        jmp decompressDataBlock_        ; done reading all tables
        .endproc

        .proc decompressData
:       jsr decompressDataBlock         ; NOTE: the caller must set Y to zero
        beq :-                          ; decompress all data blocks
                                        ; done decompression, clean up and exit
        .if BORDER_EFFECT_TYPE <> 0
        pla
        sta borderColor                 ; restore TED registers
        .endif
        .if NO_BLANK_DISPLAY = 0
        pla
        sta $ff06
        .endif
        .if NO_ZP_SAVE_RESTORE = 0
        ldx #<zpBytesUsed
:       pla
        sta zpBaseAddress - 1, x        ; restore zeropage variables
        dex
        bne :-
        .endif
        .if NO_ROM_ENABLE_RESTORE = 0
        sty $ff3e
        .endif
        .if SFX_NO_CLI = 0
        cli
        .endif
        .if SFX_RUN_BASIC_PROGRAM = 0
        jmp $867e
        .else
        jsr $8bbe
        jmp $8bdc
        .endif
        .endproc

decompressCode2End:

