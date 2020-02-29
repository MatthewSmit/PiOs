#pragma once

#include "ktypes.h"

extern uint64_t mmioBase;

#define TIMER_BASE (mmioBase + 0x00003000)
#define DMA_BASE (mmioBase + 0x00007000)
#define INTERRUPT_BASE (mmioBase + 0x0000B000)
#define MAILBOX_BASE (mmioBase + 0x0000B880)
#define TIMER2_BASE (mmioBase + 0x0000B000)
#define CLOCK_BASE (mmioBase + 0x00100000)
#define GPIO_BASE (mmioBase + 0x00200000)
#define UART0_BASE (mmioBase + 0x00201000)
#define PCM_BASE (GPIO_BASE + 0x00003000)
#define EMMC_BASE (mmioBase + 0x00300000)

// Mini Uart/2 SPI Masters
#define AUX_BASE (mmioBase + 0x00215000)
#define AUX_IRQ ((volatile uint32_t*)(AUX_BASE + 0x00))
#define AUX_ENABLES ((volatile uint32_t*)(AUX_BASE + 0x04))
#define AUX_MU_IO_REG ((volatile uint32_t*)(AUX_BASE + 0x40))
#define AUX_MU_IER_REG ((volatile uint32_t*)(AUX_BASE + 0x44))
#define AUX_MU_IIR_REG ((volatile uint32_t*)(AUX_BASE + 0x48))
#define AUX_MU_LCR_REG ((volatile uint32_t*)(AUX_BASE + 0x4C))
#define AUX_MU_MCR_REG ((volatile uint32_t*)(AUX_BASE + 0x50))
#define AUX_MU_LSR_REG ((volatile uint32_t*)(AUX_BASE + 0x54))
#define AUX_MU_MSR_REG ((volatile uint32_t*)(AUX_BASE + 0x58))
#define AUX_MU_SCRATCH ((volatile uint32_t*)(AUX_BASE + 0x5C))
#define AUX_MU_CNTL_REG ((volatile uint32_t*)(AUX_BASE + 0x60))
#define AUX_MU_STAT_REG ((volatile uint32_t*)(AUX_BASE + 0x64))
#define AUX_MU_BAUD_REG ((volatile uint32_t*)(AUX_BASE + 0x68))
#define AUX_SPI0_CNTL0_REG ((volatile uint32_t*)(AUX_BASE + 0x80))
#define AUX_SPI0_CNTL1_REG ((volatile uint32_t*)(AUX_BASE + 0x84))
#define AUX_SPI0_STAT_REG ((volatile uint32_t*)(AUX_BASE + 0x88))
#define AUX_SPI0_IO_REG ((volatile uint32_t*)(AUX_BASE + 0x90))
#define AUX_SPI0_PEEK_REG ((volatile uint32_t*)(AUX_BASE + 0x94))
#define AUX_SPI1_CNTL0_REG ((volatile uint32_t*)(AUX_BASE + 0xC0))
#define AUX_SPI1_CNTL1_REG ((volatile uint32_t*)(AUX_BASE + 0xC4))
#define AUX_SPI1_STAT_REG ((volatile uint32_t*)(AUX_BASE + 0xC8))
#define AUX_SPI1_IO_REG ((volatile uint32_t*)(AUX_BASE + 0xD0))
#define AUX_SPI1_PEEK_REG ((volatile uint32_t*)(AUX_BASE + 0xD4))

static inline void memory_read_barrier() {
    asm("DMB LD");
}

static inline void memory_write_barrier() {
    asm("DMB ST");
}

static inline void delay(uint32_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
    : "=r"(count): [count]"0"(count) : "cc");
}
