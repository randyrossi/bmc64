#include "semaphore.h"

#if defined(__aarch64__)
// The Pi 4 and 5 (64-bit): the same counting semaphore with GCC's atomic
// builtins, waiting with WFE and waking with SEV as below.

void sem_dec(uint32_t* semaphore) {
  for (;;) {
    uint32_t value = __atomic_load_n(semaphore, __ATOMIC_ACQUIRE);
    if (value == 0) {
      asm volatile("wfe");
      continue;
    }
    if (__atomic_compare_exchange_n(semaphore, &value, value - 1, 0,
                                    __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {
      return;
    }
  }
}

void sem_inc(uint32_t* semaphore) {
  __atomic_add_fetch(semaphore, 1, __ATOMIC_RELEASE);
  asm volatile("dsb sy\n\tsev" ::: "memory");
}

#else

void sem_dec(uint32_t* semaphore) {
#ifndef RASPI_LITE
    asm volatile (
      "1:  LDREX    r1, [r0]\n"
      "    CMP	    r1, #0\n"
      "    BEQ     2f             \n"
      "    SUB     r1, #1        \n"
      "    STREX   r2, r1, [r0]  \n"
      "    CMP     r2, #0        \n"
      "    BNE     1b            \n"
      "    DMB                   \n"
      "    B       3f\n"
      "2: \n" 
      "    wfe \n"
      "    B       1b\n"
      "3:\n"
    );
#endif
}

void sem_inc(uint32_t* semaphore) {
#ifndef RASPI_LITE
    asm volatile (
      "1:   LDREX   r1, [r0]\n"
      "     ADD     r1, #1\n"
      "     STREX   r2, r1, [r0]\n"
      "     CMP     r2, #0\n"
      "     BNE     1b\n"
      "     CMP     r0, #1\n"
      "     DMB\n"
      "     BGE     2f\n"
      "     B       3f\n"
      "2:\n"
      "     DSB\n"
      "     SEV\n"
      "3:\n"
    );
#endif
}

#endif
