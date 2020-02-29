#include "timer.h"

void waitUsec(uint32_t microseconds)
{
    // get the current counter frequency
    unsigned long f;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(f));

    // read the current counter
    unsigned long t;
    asm volatile("mrs %0, cntpct_el0" : "=r"(t));

    // calculate expire value for counter
    t += ((f / 1000) * microseconds) / 1000;
    unsigned long r;
    do {
        asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    } while (r < t);
}
