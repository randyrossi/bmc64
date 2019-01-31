
VICE CIA timer emulation
========================
(30.12.2010, A. Fachat)

This text briefly describes some essentials on how the CIA timer emulation in VICE works.


Context
-------

The CIA timer emulation works within the VICE framework. As such the main infrastructure
component is the "alarm" framework. The "alarm" framework provides an internal timer
mechanism with which the VICE components (chip emulations mostly) can request that a
callback function is called at a specific clock cycle.

In the CIA emulation this is for example used to call ciacore_intta() and ciacore_inttb()
when the timer underflows.

Each alarm state is stored in a alarm_s struct, embedded in the cia_context_s struct, that
is created for each CIA.


General Considerations
----------------------

In VICE not every cycle is emulated cycle-by-cycle. Instead when a timer is set, an alarm
is set to when the timer will underflow and possibly generate an interrupt. This as well
as other things are done to improve performance by 
- avoiding computations that are not necessary (e.g. register values that are never read)
- reducing the number of calls from the main CPU loop, which trash cache performance

As not every cycle is emulated by-cycle, the CIA state in general contains "old" values.
In case a register is read or written to, thus the function cia_update_ta() resp. 
cia_update_tb() is called to compute the actual register values. The parameter "rclk"
given to these is the current clock value that the (old) timer state has to be
updated to. The update happens in a loop that rolls-forward the missed alarm calls
(in case there are any - e.g. when IRQ is disabled), then a call to cia_do_update_ta() 
resp. cia_do_update_tb(). These are also called from the alarm functions.

In these update functions, then the CIA timer is updated by calling ciat_update().

In other parts, mostly when a register is set, the CIA alarm is set explicitely by calling
ciat_set_alarm().


Timer emulation
---------------

The timer emulation is separated into functions that work on the ciat_s struct. It contains
the latch and counter values, clock values, alarm state, and most importantly the main
timer state as a ciat_tstate_t.

This main timer state basically is a 16-bit word that contains the following state bits:

#define CIAT_CR_MASK    0x039
#define CIAT_CR_START   0x001
#define CIAT_CR_ONESHOT 0x008
#define CIAT_CR_FLOAD   0x010
#define CIAT_PHI2IN     0x020
#define CIAT_STEP       0x004

#define CIAT_COUNT2     0x002
#define CIAT_COUNT3     0x040
#define CIAT_COUNT      0x800
#define CIAT_LOAD1      0x080
#define CIAT_ONESHOT0   0x100
#define CIAT_ONESHOT    0x1000
#define CIAT_LOAD       0x200
#define CIAT_OUT        0x400

Some of these signals are inspired as in Wolfgang Lorenz' documentation of the PC64 
emulator "A software model of the cia6526". In the development process, these have
been developed and extended further.


Cycle-by-cycle emulation
------------------------

This is the main timer emulation step that is used to do cycle-by-cycle timer
emulation (basically in both ciat_update() and ciat_set_alarm(), only the latter
looks ahead into the "future" to compute when the next alarm would be). I.e. it
advances a CIA timer by a single clock cycle:

	if (...) {
	    /* warp mode, see below */
	} else {
            /* inc */
            if (state->cnt && (t & CIAT_COUNT3)) {
                state->cnt--;
            }
            t = ciat_table[t];
            state->clk ++;
	}

        if ((state->cnt == 0) && (t & CIAT_COUNT3)) {
            t |= CIAT_LOAD | CIAT_OUT;
            n++;
        }
        if (t & CIAT_LOAD) {
            state->cnt = state->latch;
            t &= ~CIAT_COUNT3;
        }
        if ( (t & CIAT_OUT)
                && (t & (CIAT_ONESHOT | CIAT_ONESHOT0))) {
            t &= ~(CIAT_CR_START | CIAT_COUNT2);
        }


As you can see the first part (with the /* inc */) decrements the counter, and advances 
the CIA state ("t") by looking it up in the ciat_table[]. Then counter underflow and 
load is handled.

The ciat_table[] contains the standard "next" state for each cycle. It is initialized
in ciat_init_table().

    for (i = 0; i < CIAT_TABLEN; i ++) {

        tmp = i & (CIAT_CR_START
                | CIAT_CR_ONESHOT
                | CIAT_PHI2IN);

        if ((i & CIAT_CR_START) && (i & CIAT_PHI2IN))
            tmp |= CIAT_COUNT2;
        if ((i & CIAT_COUNT2) || ((i & CIAT_STEP) && (i & CIAT_CR_START)))
            tmp |= CIAT_COUNT3;
        if (i & CIAT_COUNT3)
            tmp |= CIAT_COUNT;

        if (i & CIAT_CR_FLOAD)
            tmp |= CIAT_LOAD1;
        if (i & CIAT_LOAD1)
            tmp |= CIAT_LOAD;

        if (i & CIAT_CR_ONESHOT)
            tmp |= CIAT_ONESHOT0;
        if (i & CIAT_ONESHOT0)
            tmp |= CIAT_ONESHOT;

        ciat_table[i] = tmp;
    }

In this loop the loop variable "i" loops through all possible state values.
The "tmp" value is computed as the state value of the following cycle.

Thus CIAT_LOAD is a delayed CIAT_LOAD1 which in turn is a delayed version of CIAT_CR_FLOAD.
CIAT_ONESHOT is a delayed CIAT_ONESHOT0, which is a delayed CIAT_CR_ONESHOT.

CIAT_COUNT2 is set when the timer is started and counts phi2 (CIAT_CR_START and CIAT_PHI2IN).
CIAT_COUNT3 then is set when CIA_COUNT2 is set in the previous cycle, or 
the timer is running (CIAT_CR_START) and does not count Phi2 but uses CIAT_STEP as trigger.
What you can see is that CIAT_COUNT is a delayed version of CIAT_COUNT3.


Warp mode
---------

But when the timer is computed cycle-by-cycle, how is the current performance achieved?
Well, by NOT computing the timer cycle-by-cycle - at least most of the time. The cycle-by-cycle
emulation shown above is correct, but in fact it computes a lot of unnecessary cycles.

As the above code is within a while loop, the "increment" part
could be simplified by substracting the clock difference from cnt...
... if there would not be any underflow, or other conditions. 

So to get to an appropriate speed, without sacrificing correctness, the above loop
is used for the "difficult" parts of the timer handling. However, in the loop
there is a check for a specific timer condition, where the timer state can be
"warped" to a later clock value without problems.

This is implemented by the somewhat scary^D^D^D^D^Dcomplicated if condition
before the "inc" part (in ciat_update()):

        if ( ((t & (CIAT_CR_START | CIAT_CR_FLOAD | CIAT_LOAD1
                | CIAT_PHI2IN | CIAT_COUNT2 | CIAT_COUNT3 | CIAT_COUNT
                | CIAT_LOAD))
                == (CIAT_CR_START | CIAT_PHI2IN | CIAT_COUNT2 | CIAT_COUNT3
                | CIAT_COUNT))
            && ( ((t & CIAT_CR_ONESHOT) && (t & CIAT_ONESHOT0)
                                                && (t & CIAT_ONESHOT))
                || ( (!(t & CIAT_CR_ONESHOT)) && (!(t & CIAT_ONESHOT0))
                                                && (!(t & CIAT_ONESHOT)))) )
        {
            /* warp counting */
            if (state->clk + state->cnt > cclk) {
                state->cnt -= ((WORD)(cclk - state->clk));
                state->clk = cclk;
            } else {
                if (t & (CIAT_CR_ONESHOT | CIAT_ONESHOT0)) {
                    state->clk = state->clk + state->cnt;
                    state->cnt = 0;
                } else {
                    /* overflow clk <= cclk */
                    state->clk = state->clk + state->cnt;
                    state->cnt = 0;
                    /* n++; */
                    if (((WORD)(cclk - state->clk)) >= state->latch + 1) {
                        m = (cclk - state->clk) / (state->latch + 1);
                        n += m;
                        state->clk += m * (state->latch + 1);
                    }
                }
                /* here we have cnt=0 and clk <= cclk */
            }
        } else
        if ( (!(t & (CIAT_COUNT2 | CIAT_COUNT3 | CIAT_COUNT)))
            && ((!(t & CIAT_CR_START))
                || (!(t & (CIAT_PHI2IN | CIAT_STEP))))
            && (!(t & (CIAT_CR_FLOAD | CIAT_LOAD1 | CIAT_LOAD)))
            && ( ((t & CIAT_CR_ONESHOT) && (t & CIAT_ONESHOT0)
                                                && (t & CIAT_ONESHOT))
                || ( (!(t & CIAT_CR_ONESHOT)) && (!(t & CIAT_ONESHOT0))
                                                && (!(t & CIAT_ONESHOT)))) ) {
            /* warp stopped */
            state->clk = cclk;
        } else
        if ( (t == (CIAT_COUNT | CIAT_OUT | CIAT_LOAD | CIAT_PHI2IN
                        | CIAT_COUNT2 | CIAT_CR_START))
                && (state->cnt == 1)
                && (state->latch == 1) ) {
            /* when latch=1 and cnt=1 this warps up to clk */
            m = (int)((cclk - state->clk) & (CLOCK)~1);
            if (m) {
                state->clk += m;
                n += (m >> 1);
            } else {
                t = ciat_table[t];
                state->clk ++;
            }
        } else {
            /* inc */
	    ...

The first condition for warp counting defines that 
	CIAT_CR_START,
	CIAT_PHI2IN,
	CIAT_COUNT2,
	CIAT_COUNT3 and 
	CIAT_COUNT must be set, while
	CIAT_CR_FLOAD (force load), as well as
	CIAT_LOAD1 and 
	CIAT_LOAD must be clear.
	plus 
	CIAT_CR_ONESHOT, CIAT_ONESHOT0 and CIAT_ONESHOT
	are either all set 
	or all clear.

What this tells us that counter counts Phi2, is running and not shortly
after load (CIAT_COUNT/2/3), and not shortly before load (CIAT_CR_FLOAD,
CIAT_LOAD/1). It is also not somewhere in the middle of some one-shot
handling (CIAT_ONESHOT/0, CIAT_CR_ONESHOT).
Thus a "very normal" operation. Then the counter is advanced to the
current time (if it would not underflow) or to the underflow if it does
(with one-shot handling).

The second condition defines that
	CIAT_COUNT2, CIAT_COUNT3, CIAT_COUNT are all clear,
	and [ the timer is not running (CIAT_CR_START)
		or is currently not running (single step, but not at this time,
			(CIAT_PHI2IN, CIAT_STEP)
	    ]
	and [ the timer is not just loaded (CIAT_CR_FLOAD, CIAT_LOAD/1)
	    ]
	and [ CIAT_CR_ONESHOT/CIAT_ONESHOT/0 are either all set or all clear
	    ]

I.e. the timer does not change, and thus the timer can simply be advanced.

            /* warp stopped */
            state->clk = cclk;


The third if condition catches a performance problem when the counter as well as
the latch are set to 1 - which would eliminate warping otherwise and would make
the emulator really compute the timer for each cycle...

		
