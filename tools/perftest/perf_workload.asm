; perf_workload.asm - deterministic C64 workloads for the BMC64 performance
; test suite (docs/architecture/PERFORMANCE_TEST_PLAN.md).
;
; One source, several programs, selected with xa -D defines (see Makefile) -
;
;   (none)      idle    - screen up, CPU spinning, nothing else. Baseline.
;   VIC         vic     - 8 expanded multicolour sprites, a CIA timer IRQ every
;                         ~62 cycles that rewrites $D011/$D020/$D021 (forced
;                         bad line + colour change on nearly every raster line).
;   SID         sid     - all 3 voices on SID 1 - combined waveform, sync, ring
;                         mod, filter routing with resonance, per-frame sweeps.
;   SID + DUAL  sid2    - as above on SID 1 ($D400) AND SID 2 ($D420). Needs
;                         dual SID enabled with SID 2 at $D420, otherwise the
;                         second set of writes aliases onto SID 1.
;   VIC + SID   stress  - both at once.
;
; Everything is autonomous (no input, no media, no timing dependence on the
; host), so the same program does the same emulated work on every run and on
; every emulator core. The audio is deliberately harsh - turn the volume down.
;
; Build - xa -DVIC -DSID -o perf_stress.prg perf_workload.asm

zp_last = $fb                   ; last raster line seen by the IRQ handler

; Copies of the SID registers the per-frame sweep changes. The SID registers
; are write-only - reading one returns the decayed data bus, not the value last
; written - so the sweep keeps its own copy and only ever writes the SID.
; Indexed by X ($00 for SID 1, $20 for SID 2), so each SID gets its own copy.
sh_fc   = $c000                 ; filter cutoff high ($d416)
sh_p1   = $c001                 ; voice 1 frequency low ($d400)
sh_p2   = $c002                 ; voice 2 frequency low ($d407)
sh_p3   = $c003                 ; voice 3 frequency low ($d40e)

; xa writes no PRG load address, so emit it ourselves at $07ff. The code
; follows the 12 byte BASIC stub directly, so start = $080d = 2061.
*=$07ff
        .word $0801             ; PRG load address
        .word basend            ; BASIC - 10 SYS 2061
        .word 10
        .byt $9e
        .asc "2061"
        .byt 0
basend  .word 0

start   jsr $e544               ; clear screen (KERNAL, before we bank it out)
        sei
        lda #$7f                ; no CIA interrupts, ack anything pending
        sta $dc0d
        sta $dd0d
        lda $dc0d
        lda $dd0d
        lda #$35                ; I/O visible, KERNAL and BASIC banked out
        sta $01
        lda #<nmi               ; RAM vectors (KERNAL ROM is gone)
        sta $fffa
        lda #>nmi
        sta $fffb
        lda #<irq
        sta $fffe
        lda #>irq
        sta $ffff
        lda #$00
        sta $d020
        sta $d021
        sta zp_last

#ifdef SID
        ldx #$00
        jsr sidinit
#ifdef DUAL
        ldx #$20
        jsr sidinit
#endif
#endif

#ifdef VIC
        jsr vicinit
        cli
main    jmp main                ; all the work happens in the IRQ
#else
#ifdef SID
main    lda $d012               ; wait for raster line 255, then tick
        cmp #$ff
        bne main
        jsr frametick
wait2   lda $d012               ; ...and for it to be over
        cmp #$ff
        beq wait2
        jmp main
#else
main    jmp main                ; idle
#endif
#endif

nmi     rti

; ---------------------------------------------------------------------------
; Once per frame - sweep the SID filter and pitches, nudge the sprites.
; ---------------------------------------------------------------------------
frametick
#ifdef SID
        ldx #$00
        jsr sidmod
#ifdef DUAL
        ldx #$20
        jsr sidmod
#endif
#endif
#ifdef VIC
        inc $d000
        inc $d002
        inc $d004
        inc $d006
        inc $d008
        inc $d00a
        inc $d00c
        inc $d00e
#endif
        rts

#ifdef SID
; X = SID base offset from $D400 ($00 for SID 1, $20 for SID 2)
sidmod  inc sh_fc,x             ; filter cutoff sweep
        lda sh_fc,x
        sta $d416,x
        inc sh_p1,x             ; voice 1 pitch up
        lda sh_p1,x
        sta $d400,x
        dec sh_p2,x             ; voice 2 pitch down
        lda sh_p2,x
        sta $d407,x
        inc sh_p3,x             ; voice 3 pitch up
        lda sh_p3,x
        sta $d40e,x
        rts

sidinit lda #$00                ; voice 1
        sta sh_p1,x
        sta $d400,x
        lda #$1c
        sta $d401,x
        lda #$00
        sta $d402,x
        lda #$08
        sta $d403,x
        lda #$00
        sta $d405,x             ; attack/decay 0
        lda #$f0
        sta $d406,x             ; sustain 15
        lda #$00                ; voice 2
        sta sh_p2,x
        sta $d407,x
        lda #$22
        sta $d408,x
        lda #$00
        sta $d409,x
        lda #$04
        sta $d40a,x
        lda #$00
        sta $d40c,x
        lda #$f0
        sta $d40d,x
        lda #$80                ; voice 3
        sta sh_p3,x
        sta $d40e,x
        lda #$15
        sta $d40f,x
        lda #$00
        sta $d413,x
        lda #$f0
        sta $d414,x
        lda #$00                ; filter - cutoff, resonance 15, all voices
        sta $d415,x
        lda #$40
        sta sh_fc,x
        sta $d416,x
        lda #$f7
        sta $d417,x
        lda #$18                ; low-pass, volume 8
        sta $d418,x
        lda #$61                ; v1 saw+pulse (combined) + gate
        sta $d404,x
        lda #$23                ; v2 saw + sync + gate
        sta $d40b,x
        lda #$15                ; v3 triangle + ring mod + gate
        sta $d412,x
        rts
#endif

#ifdef VIC
; Fill the screen and colour RAM with a busy pattern, set up 8 sprites and
; start a CIA1 timer A interrupt with a period close to one raster line.
vicinit ldy #$00
fill    tya
        sta $0400,y
        sta $0500,y
        sta $0600,y
        sta $0700,y
        sta $d800,y
        sta $d900,y
        sta $da00,y
        sta $db00,y
        iny
        bne fill

        ldy #62                 ; sprite shape - every 2-bit colour combination
        lda #$1b
sd      sta $2000,y
        dey
        bpl sd

        ldx #7
sp      lda #$80                ; sprite pointer -> $2000
        sta $07f8,x
        lda xtab,x
        sta spx
        txa
        asl
        tay
        lda spx
        sta $d000,y
        lda ytab,x
        sta $d001,y
        txa
        clc
        adc #1
        sta $d027,x             ; sprite colours 1..8
        dex
        bpl sp

        lda #$02
        sta $d025               ; shared multicolours
        lda #$07
        sta $d026
        lda #$ff
        sta $d015               ; enable, multicolour, expand X and Y
        sta $d01c
        sta $d017
        sta $d01d

        lda #<61                ; CIA1 timer A - ~62 cycle period, continuous
        sta $dc04
        lda #>61
        sta $dc05
        lda #$81
        sta $dc0d               ; timer A interrupt on
        lda #$11
        sta $dc0e               ; start, force load
        rts

spx     .byt 0
xtab    .byt $30,$50,$70,$90,$b0,$d0,$f0,$60
ytab    .byt $70,$78,$80,$88,$90,$98,$a0,$a8

; Timer IRQ - force a bad line and change colours for the next raster line.
; Detect the start of a new frame by the raster line wrapping round, not by
; an exact match, since a handler can straddle lines.
irq     pha
        txa
        pha
        tya
        pha
        lda $dc0d               ; acknowledge CIA1
        lda $d012
        tax
        sta $d021
        sta $d020
        clc
        adc #1
        and #$07
        ora #$18                ; text mode, 25 rows, screen on, Y scroll
        sta $d011
        cpx zp_last
        bcs nowrap
        jsr frametick
nowrap  stx zp_last
        pla
        tay
        pla
        tax
        pla
        rti
#else
irq     rti
#endif
