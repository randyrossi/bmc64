
; assume running on the C16 (RAM ends at $4000)
.define SFX_C16_MODE            0
; do not verify checksum if this is set to any non-zero value
.define NO_CRC_CHECK            0
; do not read ahead one byte of compressed data
.define NO_READ_BUFFER          0
; disable border effect and saving/restoring of the border color
.define NO_BORDER_EFFECT        0
; do not clear the color memory ($0800-$0B5F) after decompression
.define NO_COLOR_MEMORY_CLEAR   0
; do not restore RAM/ROM paging after decompression, return in RAM ($FF3F) mode
.define NO_ROM_ENABLE_RESTORE   0
; do not blank display, and do not save/restore $FF06
.define NO_BLANK_DISPLAY        0
; do not enable interrupts after decompression
.define SFX_NO_CLI              0
; start BASIC program after decompression
.define SFX_RUN_BASIC_PROGRAM   0

sfxStartAddress = $100d

        .setcpu "6502"
        .code

lzMatchReadAddrLow = $2b
lzMatchReadAddrHigh = $2c
prvDistanceTablePos = $2d
prvDistanceLowTable = $2e
prvDistanceHighTable = $32
huffmanInitTmp = $2e
huffTableWriteAddrLow = $2f
huffTableWriteAddrHigh = $30
huffmanDecodedValueLow = $32
huffmanDecodedValueHigh = $33
huffSymbolsRemainingLow = $34
huffSymbolsRemainingHigh = $35
shiftRegister = $36
crcValue = $36
readAddrLow = $37
readAddrHigh = $38
readByteBuffer = $39
bytesRemainingLow = $3a
bytesRemainingHigh = $3b
decompressWriteAddrLow = $3c
decompressWriteAddrHigh = $3d
tmpLow = $3e
tmpHigh = $3f
huffmanSymCntTable = $40
huffmanOffsetLowTable = $50
huffmanOffsetHighTable = $60
loadEndAddrLow = $9d
loadEndAddrHigh = $9e

borderColor = $ff19

decompressCode1Size = decompressCode1End - decompressCode1Start
decompressCode2Size = decompressCode2End - decompressCode2Start
decompressCode3Size = decompressCode3End - decompressCode3Start
tmpLength1 = (decompressCode1Size >= decompressCode3Size) * decompressCode1Size
tmpLength2 = (decompressCode3Size > decompressCode1Size) * decompressCode3Size
decompressCodeCopyLength = tmpLength1 + tmpLength2
decompressCode1Start_ = sfxLoaderEnd
decompressCode1End_ = decompressCode1Start_ + decompressCode1Size
decompressCode2Start_ = decompressCode1End_
decompressCode2End_ = decompressCode2Start_ + decompressCode2Size
decompressCode3Start_ = decompressCode2End_
decompressCode3End_ = decompressCode3Start_ + decompressCode3Size
decompressCode2Offset = decompressCodeCopyLength - decompressCode2Size

        .word $1001, $100b, $000a
        .byte $9e, .sprintf("%d", sfxStartAddress)
        .res sfxStartAddress - $100a, $00
        .org sfxStartAddress

        .proc main
        .if SFX_C16_MODE <> 0
        lda #$00
        sta readAddrLow
        lda #$40
        sta readAddrHigh
        .endif
        sei
        cld
        ldx #decompressCodeCopyLength
l1:     lda decompressCode1Start_ - 1, x
        sta decompressCode1Start - 1, x
        lda decompressCode2Start_ - decompressCode2Offset - 1, x
        sta decompressCode2Start - decompressCode2Offset - 1, x
        lda decompressCode3Start_ - 1, x
        sta decompressCode3Start - 1, x
        lda $01, x
        sta $0941, x
        dex
        bne l1
        .if NO_BLANK_DISPLAY = 0
        lda $ff06                       ; save TED registers
        stx $ff06
        .endif
        .if NO_CRC_CHECK = 0
        stx crcValue
        .endif
        stx $ff3f
        dex
        txs
        .if NO_BLANK_DISPLAY = 0
        pha
        .endif
        .if NO_BORDER_EFFECT = 0 && NO_COLOR_MEMORY_CLEAR = 0
        lda borderColor
        pha
        .endif
        lda loadEndAddrLow
        sec
        sbc #<decompressCode3End_
        tay
        beq l5
        ldx #<decompressCode3End_
        stx loadEndAddrLow
        bcc l2
        inc loadEndAddrHigh
l2:     eor #$ff
        sec
        adc readAddrLow
        sta readAddrLow
        bcs l4
l3:     dec readAddrHigh
l4:     dec loadEndAddrHigh
:       dey
        lda (loadEndAddrLow), y
        sta (readAddrLow), y
        .if NO_CRC_CHECK = 0
        eor crcValue
        asl
        adc #$c4
        sta crcValue
        .endif
        tya
        bne :-
l5:     lda loadEndAddrHigh
        cmp #>decompressCode3End_
        bne l3
l6:     lda #$80                        ; NOTE: this also initializes the
        .if NO_CRC_CHECK = 0
        cmp crcValue                    ; shift register (which is the same
        bne l7                          ; variable)
        .else
        sta shiftRegister
        .endif
        .if NO_READ_BUFFER = 0
        ldx #$04                        ; skip CRC byte
        jmp decompressData + 2
        .else
        jmp decompressData
        .endif
l7:
        .if NO_CRC_CHECK = 0
        sty $ff3e                       ; reset machine on CRC error
        jmp ($fffc)
        .endif
        .endproc

sfxLoaderEnd:

; -----------------------------------------------------------------------------

        .org $0124

decompressCode1Start:

        .proc decompressData
l1:     ldx #$03                        ; read address and 65536 - data length
l2:     stx prvDistanceTablePos
        jsr read8Bits
        ldx prvDistanceTablePos
        sta bytesRemainingLow, x
        dex
        bpl l2
        ldx #$02
        jsr readXBits
        lsr
        pha                             ; save last block flag
        lda #<read8Bits
        sta readCharAddrLow
        bcc l3                          ; is compression enabled ?
        jsr huffmanInit
l3:
        .if NO_BORDER_EFFECT = 0
        inc borderColor                 ; border effect at LZ match
        .endif
l4:     jsr huffmanDecode1              ; read next character
        bcs l6
        sta (decompressWriteAddrLow), y ; store decompressed data
        inc decompressWriteAddrLow
        beq l17
l5:     inc bytesRemainingLow
        bne l4
l16:    inc bytesRemainingHigh
        bne l3                          ; border effect at 256 byte blocks
        pla                             ; check last block flag
        beq l1                          ; continue with next block
        jmp sfxDecompressEnd            ; done decompression, clean up and exit
l17:    inc decompressWriteAddrHigh
        bcc l5
l6:     cmp #$3c                        ; LZ match code
        bcs l12
        cmp #$08
        bcc l8
        sty tmpHigh                     ; read offset
        jsr readLZMatchParameterBits
        ldx prvDistanceTablePos         ; store in recent offset buffer
        ldy tmpHigh
        sta prvDistanceLowTable, x
        sty prvDistanceHighTable, x
        dex
        bpl l7
        ldx #$03
l7:     stx prvDistanceTablePos
l8:     ldx #$00
l9:     stx deltaValue
        eor #$ff                        ; calculate match address
        adc decompressWriteAddrLow
        sta lzMatchReadAddrLow
        tya
        eor #$ff
        adc decompressWriteAddrHigh
        sta lzMatchReadAddrHigh
        ldy #$00                        ; read match length code
        ldx #$2f
l10:    jsr huffmanDecode2
        cmp #$08
        bcc l11
        jsr readLZMatchParameterBits    ; read extra bits if length >= 10 bytes
l11:    tax
        inx                             ; adjust length to 2..256 range
        inx
        .byte $a9
l14:    iny
        lda (lzMatchReadAddrLow), y
        clc
        adc #$00
        sta (decompressWriteAddrLow), y
        dex
        bne l14
        tya
        sec
        adc decompressWriteAddrLow
        sta decompressWriteAddrLow
        bcc l15
        inc decompressWriteAddrHigh
l15:    tya
        ldy #$00
        sec
        adc bytesRemainingLow
        sta bytesRemainingLow
        bcc l3
        bcs l16
l12:    cmp #$40                        ; check special match codes
        bcs l13
        and #$03                        ; LZ match with delta value
        pha
        ldx #$07
        jsr readXBits
        sbc #$3f
        adc #$00
        tax
        pla
        bcc l9
l13:    adc prvDistanceTablePos         ; LZ match with recent offset
        and #$03
        tax
        lda prvDistanceLowTable, x
        ldy prvDistanceHighTable, x
        bcc l8
        .endproc

readCharAddrLow = decompressData::l4 + 1
readLengthAddrLow = decompressData::l10 + 1
deltaValue = decompressData::l14 + 5

addrTable:
        .byte <(readCharAddrLow - decompressData)
        .byte <(readLengthAddrLow - decompressData)
        .byte <read9Bits
        .byte <read5Bits
        .byte <huffmanDecode1
        .byte <huffmanDecode2
        .byte $00, $30
        .byte $00, $44
        .byte $08, $09

decompressCode1End:

; -----------------------------------------------------------------------------

        .org $0b60

decompressCode2Start:

        .proc huffmanInit
        tya
        jsr l1
        lda #$01
l1:     asl shiftRegister
        bne :+
        jsr readCompressedByte
:       bcs l3
        tay
        lda addrTable + 2, y
l2:     ldx addrTable, y
        sta decompressData, x
        ldy #$00
        rts
l3:     ora #$08
        tay
        ldx #$04
l4:     sta huffmanInitTmp - 1, x
        dey
        dey
        lda addrTable + 4, y
        dex
        bne l4
        jsr l2
        ldx huffmanInitTmp
l5:     lda #$ff
        sta huffmanDecodedValueLow
        asl
        sta huffmanDecodedValueHigh
        jsr gammaDecode
        sbc #$00                        ; gammaDecode returns with C=0
        sta huffSymbolsRemainingLow
        sta huffmanSymCntTable, x
        bne l7
        ora tmpHigh
        beq l9                          ; unused code length?
l6:     bne l6                          ; symCnt == 256 is not supported
l7:     jsr gammaDecode
        adc huffmanDecodedValueLow
        sta huffmanDecodedValueLow
        lda tmpHigh
        adc huffmanDecodedValueHigh
        sta huffmanDecodedValueHigh
        sta (huffTableWriteAddrLow), y
        inc huffTableWriteAddrHigh
        inc huffTableWriteAddrHigh
        lda huffmanDecodedValueLow
        sta (huffTableWriteAddrLow), y
        inc huffTableWriteAddrLow
        beq l8
        dec huffTableWriteAddrHigh
l8:     dec huffTableWriteAddrHigh
        dec huffSymbolsRemainingLow
        bne l7
l9:     lda huffTableWriteAddrLow
        sta huffmanOffsetLowTable, x
        lda huffTableWriteAddrHigh
        sta huffmanOffsetHighTable, x
        dec huffmanOffsetHighTable, x
        inx
        txa
        and #$0f
        bne l5
        rts
        .endproc

decompressCode2End:

; -----------------------------------------------------------------------------

        .org $0335

decompressCode3Start:

        .proc huffmanDecode1
        ldx #$ff
        .endproc

        .proc huffmanDecode2
        tya
l1:     inx
        asl shiftRegister
        beq l3
l2:     rol
        sec
        sbc huffmanSymCntTable, x
        bcs l1
        adc huffmanOffsetLowTable, x
        sta tmpLow
        tya
        adc huffmanOffsetHighTable, x
        sta tmpHigh
        eor #$02
        cmp (tmpLow), y
        sta tmpHigh
        lda (tmpLow), y
        rts
l3:     jsr readCompressedByte
        jmp l2
        .endproc

        .proc read9Bits
        ldx #$09
        .byte $2c
        .endproc

        .proc read8Bits
        ldx #$08
        .byte $2c
        .endproc

        .proc read5Bits
        ldx #$05
        .endproc

        .proc readXBits
        tya
l1:     asl shiftRegister
        bne l2
        jsr readCompressedByte
l2:     rol
        dex
        bne l1
        rts
        .endproc

        .proc readCompressedByte
        sta tmpLow
        inc readAddrLow
        bne l1
        inc readAddrHigh
l1:
        .if NO_READ_BUFFER = 0
        lda readByteBuffer
        rol
        sta shiftRegister
        lda (readAddrLow), y
        sta readByteBuffer
        .else
        lda (readAddrLow), y
        rol
        sta shiftRegister
        .endif
        lda tmpLow
        rts
        .endproc

        .proc gammaDecode
        lda #$01
        sty tmpHigh
l1:     asl shiftRegister
        bne l2
        jsr readCompressedByte
l2:     bcc gammaDecode - 1
        asl shiftRegister
        bne l3
        jsr readCompressedByte
l3:     rol
        rol tmpHigh
        bcc l1
        .endproc

        .proc readLZMatchParameterBits
        pha
        lsr
        lsr
        tax
        dex
        pla
        and #$03
        ora #$04
l1:     asl shiftRegister
        bne l2
        jsr readCompressedByte
l2:     rol
        rol tmpHigh
        dex
        bne l1
        rts
        .endproc

        .proc sfxDecompressEnd
        ldx #$95
l1:     lda $096a, x                    ; restore zeropage variables
        sta $2a, x
        dex
        bne l1
        .if NO_COLOR_MEMORY_CLEAR = 0
        ldy #$04
        lda $053b                       ; clear color memory
l2:     sta $0800, x
        inx
        bne l2
        inc l2 + 2
        dey
        bne l2
        .endif
        .if NO_BORDER_EFFECT = 0 && NO_COLOR_MEMORY_CLEAR = 0
        pla
        sta borderColor                 ; restore TED registers
        .endif
        .if NO_BLANK_DISPLAY = 0
        pla
        sta $ff06
        .endif
        .if NO_ROM_ENABLE_RESTORE = 0
        stx $ff3e
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

decompressCode3End:

