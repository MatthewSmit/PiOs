#pragma clang section bss=".text.boot" data=".text.boot" rodata=".text.boot" text=".text.boot"

#include "ktypes.h"

extern uint64_t __end;
extern uint64_t __data_start;

constexpr auto PAGESIZE = 4096;

static constexpr uint64_t highStart = 0xFFFFFFFFC0000000;

#define PT_PAGE     0b11        // 4k granule
#define PT_BLOCK    0b01        // 2M granule
// accessibility
#define PT_KERNEL   (0<<6)      // privileged, supervisor EL1 access only
#define PT_USER     (1<<6)      // unprivileged, EL0 access allowed
#define PT_RW       (0<<7)      // read-write
#define PT_RO       (1<<7)      // read-only
#define PT_AF       (1<<10)     // accessed flag
#define PT_NX       (1UL<<54)   // no execute
// shareability
#define PT_OSH      (2<<8)      // outter shareable
#define PT_ISH      (3<<8)      // inner shareable
// defined in MAIR register
#define PT_MEM      (0<<2)      // normal memory
#define PT_DEV      (1<<2)      // device MMIO
#define PT_NC       (2<<2)      // non-cachable

#define TTBR_CNP    1

static uint64_t getMmioBaseFromVersion() {
    uint64_t reg;
    asm volatile("mrs %0, midr_el1" : "=r" (reg));

    switch ((reg >> 4u) & 0xFFFu) {
        // RPI 1
        case 0xB76: return 0x20000000;

            // RPI 2/3
        case 0xC07:
        case 0xD03: return 0x3F000000;

            // RPI 4
        case 0xD08: return 0xFE000000;

        default:    return 0x20000000;
    }
}

static void copiedCreateTables(uint64_t bootstrapEnd) {
    unsigned long data_page = ((uint64_t)&__data_start - highStart) / PAGESIZE;
    auto* paging = (unsigned long*)bootstrapEnd;

    // TTBR0, identity L1
    paging[0]=(unsigned long)((unsigned char*)bootstrapEnd+2*PAGESIZE) |    // physical address
              PT_PAGE |     // it has the "Present" flag, which must be set, and we have area in it mapped by pages
              PT_AF |       // accessed flag. Without this we're going to have a Data Abort exception
              PT_USER |     // non-privileged
              PT_ISH |      // inner shareable
              PT_MEM;       // normal memory

    // identity L2 2M blocks
    auto b = getMmioBaseFromVersion() >> 21;
    for(auto r=0;r<512;r++)
        paging[2*512+r]=(unsigned long)((r<<21)) |  // physical address
                        PT_BLOCK |    // map 2M block
                        PT_AF |       // accessed flag
                        PT_NX |       // no execute
                        PT_USER |     // non-privileged
                        (r>=b? PT_OSH|PT_DEV : PT_ISH|PT_MEM); // different attributes for device memory

    // identity L2, first 2M block
    paging[2*512]=(unsigned long)((unsigned char*)bootstrapEnd+3*PAGESIZE) | // physical address
                  PT_PAGE |     // we have area in it mapped by pages
                  PT_AF |       // accessed flag
                  PT_USER |     // non-privileged
                  PT_ISH |      // inner shareable
                  PT_MEM;       // normal memory

    // identity L3
    for(auto r = 0; r < 512; r++)
        paging[3*512+r]=(unsigned long)(r*PAGESIZE) |   // physical address
                        PT_PAGE |     // map 4k
                        PT_AF |       // accessed flag
                        PT_USER |     // non-privileged
                        PT_ISH |      // inner shareable
                        ((r<0x80||r>=data_page)? PT_RW|PT_NX : PT_RO); // different for code and data

    // TTBR1, kernel L1
    paging[512+511]=(unsigned long)((unsigned char*)bootstrapEnd+4*PAGESIZE) | // physical address
                    PT_PAGE |     // we have area in it mapped by pages
                    PT_AF |       // accessed flag
                    PT_KERNEL |   // privileged
                    PT_ISH |      // inner shareable
                    PT_MEM;       // normal memory

    // kernel L2
    for(auto r=0;r<512;r++) {
        paging[4*512+r]=(unsigned long)((r<<21)) |  // physical address
                        PT_BLOCK |    // map 2M block
                        PT_AF |       // accessed flag
                        PT_NX |       // no execute
                        PT_USER |     // non-privileged
                        (r>=b? PT_OSH|PT_DEV : PT_ISH|PT_MEM); // different attributes for device memory
    }

    // identity L2, first 2M block
    paging[4*512]=(unsigned long)((unsigned char*)bootstrapEnd+5*PAGESIZE) | // physical address
                  PT_PAGE |     // we have area in it mapped by pages
                  PT_AF |       // accessed flag
                  PT_USER |     // non-privileged
                  PT_ISH |      // inner shareable
                  PT_MEM;       // normal memory

    // identity L3
    for(auto r = 0; r < 512; r++)
        paging[5*512+r]=(unsigned long)(r*PAGESIZE) |   // physical address
                        PT_PAGE |     // map 4k
                        PT_AF |       // accessed flag
                        PT_USER |     // non-privileged
                        PT_ISH |      // inner shareable
                        ((r<0x80||r>=data_page)? PT_RW|PT_NX : PT_RO); // different for code and data
}

static void copiedSetRegisters(uint64_t bootstrapEnd) {
    // check for 4k granule and at least 36 bits physical address bus */
    unsigned long r;
    asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));
    auto b=r&0xF;
    if(r&(0xF<<28)/*4k*/ || b<1/*36 bits*/) {
//        Uart1::write("ERROR: 4k granule or 36 bit address space not supported\n");
//        return;
    }

    // first, set Memory Attributes array, indexed by PT_MEM, PT_DEV, PT_NC in our example
    r=  (0xFF << 0) |    // AttrIdx=0: normal, IWBWA, OWBWA, NTR
        (0x04 << 8) |    // AttrIdx=1: device, nGnRE (must be OSH too)
        (0x44 <<16);     // AttrIdx=2: non cacheable
    asm volatile ("msr mair_el1, %0" : : "r" (r));

    // next, specify mapping characteristics in translate control register
    r=  (0b00LL << 37) | // TBI=0, no tagging
        (b << 32) |      // IPS=autodetected
        (0b10LL << 30) | // TG1=4k
        (0b11LL << 28) | // SH1=3 inner
        (0b01LL << 26) | // ORGN1=1 write back
        (0b01LL << 24) | // IRGN1=1 write back
        (0b0LL  << 23) | // EPD1 enable higher half
        (25LL   << 16) | // T1SZ
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL  << 7) |  // EPD0 enable lower half
        (25LL   << 0);   // T0SZ
    asm volatile ("msr tcr_el1, %0; isb" : : "r" (r));

    // tell the MMU where our translation tables are. TTBR_CNP bit not documented, but required
    // lower half, user space
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((unsigned long)bootstrapEnd + TTBR_CNP));
    // upper half, kernel space
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((unsigned long)bootstrapEnd + TTBR_CNP + PAGESIZE));

    // finally, toggle some bits in system control register to enable page translation
    asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (r));
    r|=0xC00800;     // set mandatory reserved bits
    r&=~((1<<25) |   // clear EE, little endian translation tables
         (1<<24) |   // clear E0E
         (1<<19) |   // clear WXN
         (1<<12) |   // clear I, no instruction cache
         (1<<4) |    // clear SA0
         (1<<3) |    // clear SA
         (1<<2) |    // clear C, no cache at all
         (1<<1));    // clear A, no aligment check
    r|=  (1<<0);     // set M, enable MMU
    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));

}

extern "C" uint64_t setupPaging() {
    //You must set CPUECTLR.SMPEN to 1 before the caches and MMU are enabled
    asm volatile("mrs x1, S3_1_c15_c2_1\n"
                 "and x1, x1, %0\n"
                 "msr S3_1_c15_c2_1, x1"
                 :
                 : "I"(1U << 6U));

    auto bootstrapEnd = (uint64_t)&__end - highStart;

    /* create MMU translation tables at _end */
    copiedCreateTables(bootstrapEnd);

    /* okay, now we have to set system registers to enable MMU */
    copiedSetRegisters(bootstrapEnd);

    return bootstrapEnd + PAGESIZE * 10;
}