
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

startAddr = $e504

        .setcpu "6502"
        .code
        .byte <startAddr
        .byte >startAddr
        .org startAddr

        .export decompressDataBlock
        .export decompressData
        .export decompressFLI

; base address of zeropage variables
.if NO_ZP_SAVE_RESTORE = 0
zpBaseAddress = $02
.else
zpBaseAddress = $de
.endif
zpBytesUsed = $0a

inputDataStartAddrLow = zpBaseAddress + $00
inputDataStartAddrHigh = zpBaseAddress + $01
inputDataEndAddrLow = zpBaseAddress + $02
inputDataEndAddrHigh = zpBaseAddress + $03
copyAddrLow = zpBaseAddress + $04
copyAddrHigh = zpBaseAddress + $05
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

zpSaveBuffer = $0347 - zpBytesUsed
slotBitsTable = $0347                   ; 3 * 51 bytes
slotBaseLowTable = $037a
slotBaseHighTable = $03ad

borderColor = $ff19

; -----------------------------------------------------------------------------

compressedFLIDataSizeMSB = $17fe
compressedFLIDataSizeLSB = $17ff
compressedFLIDataStart = $1800
compressedFLIDataEnd = $e504

        .proc decompressFLI
        ldx #$e6
l1:     dex
        lda fliAddrTable - $e0, x
        sta $00, x
        ; NOTE: this assumes that the LSB of 'compressedFLIDataStart' is zero,
        ; and all other bytes to be copied are non-zero
        bne l1
        ldy compressedFLIDataSizeLSB
        lda #>compressedFLIDataStart
        clc
        adc ($02, x)
        sty $02, x
        sta $03, x
        jmp decompressData
        .endproc

; -----------------------------------------------------------------------------

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
        adc #$ff                        ; = deltaValue
        sec
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
deltaValue = decompressDataBlock_::l4 + 5
writeByte = decompressDataBlock_::l5

        .proc copyLZMatch
        jsr readEncodedValue            ; LZ match: decode length value
        pha
        ldx decodedValueHigh
        stx tmpHigh
        bne l4
        lsr                             ; check length
        bne l4                          ; 3 bytes or longer ?
        bcs l3                          ; 2 bytes ?
        jsr read2Bits                   ; match length = 1 byte, 2 prefix bits
        adc #$07
l1:     tax
        jsr readEncodedValue            ; decode offset value
        ldx #$ff
l2:     stx deltaValue
        eor #$ff                        ; calculate read address
        adc decompressWriteAddrLow
        sta lzMatchReadAddrLow
        lda decompressWriteAddrHigh
        sbc decodedValueHigh            ; assume that the read address never
        sta lzMatchReadAddrHigh         ; overflows, so carry is always set
        pla
        tax
        bcs copyData + 2                ; skip 'sta tmpHigh'
l3:     lda #$20                        ; match length = 2 bytes, 3 prefix bits
        jsr readBits
        adc #$0b
        bcc l1
l4:     lda #$08                        ; = offs3PrefixSize, variable prefix
        jsr readBits                    ; size for 3-byte or longer matches
        beq l5
        adc #$12
        bcc l1
l5:     jsr read8Bits                   ; LZ match with delta value
        tax                             ; save delta value - 1
        lda #$08
        jsr readBits                    ; read offset (1..32, 5 bits)
        clc
        bcc l2
        .endproc

offs3PrefixSize = copyLZMatch::l4 + 1

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

; -----------------------------------------------------------------------------

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
        adc #$13
        sta tableEnd                    ; = $17, $1b, $23, or $33
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

        .proc read8Bits
        lda #$01
        .byte $2c
        .endproc

        .proc read2Bits
        lda #$40
        .endproc

        .proc readBits
l1:     asl shiftRegister
        bne :+
        jsr readCompressedByte
:       rol
        bcc l1
        rts
        .endproc

; -----------------------------------------------------------------------------

        .proc decompressData
        php
        sei
        cld
        .if NO_ZP_SAVE_RESTORE = 0
        ldy #<zpBytesUsed
:       lda zpBaseAddress - 1, y        ; save zeropage variables
        sta zpSaveBuffer - 1, y
        dey
        bne :-
        .else
        ldy #$00
        .endif
        .if NO_BLANK_DISPLAY = 0
        lda $ff06                       ; save TED registers
        pha
        sty $ff06
        .endif
        .if NO_ROM_ENABLE_RESTORE = 0
        lda $ff13
        eor #$01
        and #$01
        pha
        .endif
        .if BORDER_EFFECT_TYPE <> 0
        lda borderColor
        pha
        .endif
        .if NO_CRC_CHECK = 0
        sty crcValue
        .endif
        sty $ff3f
l1:     lda $00, x
        sta slotBitsTable, y            ; used as temporary buffer
        inx
        iny
        cpy #$06
        bne l1
l2:     lda slotBitsTable - 1, y
        sta inputDataStartAddrLow - 1, y
        dey
        bne l2
        lda inputDataEndAddrLow
        sec
        sbc inputDataStartAddrLow
        tay
        beq l6
        ldx inputDataStartAddrLow
        stx inputDataEndAddrLow
        bcc l3
        inc inputDataEndAddrHigh
l3:     eor #$ff
        sec
        adc copyAddrLow
        sta copyAddrLow
        bcs l5
l4:     dec copyAddrHigh
l5:     dec inputDataEndAddrHigh
:       dey
        lda (inputDataEndAddrLow), y
        sta (copyAddrLow), y
        .if NO_CRC_CHECK = 0
        eor crcValue
        asl
        adc #$ac
        sta crcValue
        .endif
        tya
        bne :-
l6:     lda inputDataEndAddrHigh
        cmp inputDataStartAddrHigh
        bne l4
l7:     lda #$80                        ; NOTE: this also initializes the
        .if NO_CRC_CHECK = 0
        cmp crcValue                    ; shift register (which is the same
        beq l8                          ; variable)
;       sty $ff3e                       ; reset machine on CRC error
        jmp ($fffc)
        .else
        sta shiftRegister
        .endif
l8:     lda copyAddrLow
        sta readAddrLow
        lda copyAddrHigh
        sta readAddrHigh
        .if NO_READ_BUFFER = 0
        jsr read8Bits                   ; skip CRC byte
        .endif
l9:     jsr decompressDataBlock         ; decompress all data blocks
        beq l9
        .if BORDER_EFFECT_TYPE <> 0
        pla                             ; restore TED registers
        sta borderColor
        .endif
        .if NO_ROM_ENABLE_RESTORE = 0
        pla
        tax
        sta $ff3e, x
        .endif
        .if NO_ZP_SAVE_RESTORE = 0
        ldx #<zpBytesUsed               ; restore zeropage variables
l10:    lda zpSaveBuffer - 1, x
        sta zpBaseAddress - 1, x
        dex
        bne l10
        .endif
        .if NO_BLANK_DISPLAY = 0
        pla
        sta $ff06
        .endif
        plp
        rts
        .endproc

fliAddrTable:
        .word compressedFLIDataStart
        .word compressedFLIDataSizeMSB
        .word compressedFLIDataEnd

