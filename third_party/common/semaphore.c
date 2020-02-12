#include "semaphore.h"

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
