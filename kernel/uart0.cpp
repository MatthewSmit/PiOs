#include "uart0.h"

#include "BCM2837.h"
#include "gpio.h"
#include "mailbox.h"
#include "uart1.h"

#define UART0_DR ((volatile uint32_t*)(UART0_BASE + 0x00))
#define UART0_RSRECR ((volatile uint32_t*)(UART0_BASE + 0x04))
#define UART0_FR ((volatile uint32_t*)(UART0_BASE + 0x18))
#define UART0_ILPR ((volatile uint32_t*)(UART0_BASE + 0x20))
#define UART0_IBRD ((volatile uint32_t*)(UART0_BASE + 0x24))
#define UART0_FBRD ((volatile uint32_t*)(UART0_BASE + 0x28))
#define UART0_LCRH ((volatile uint32_t*)(UART0_BASE + 0x2C))
#define UART0_CR ((volatile uint32_t*)(UART0_BASE + 0x30))
#define UART0_IFLS ((volatile uint32_t*)(UART0_BASE + 0x34))
#define UART0_IMSC ((volatile uint32_t*)(UART0_BASE + 0x38))
#define UART0_RIS ((volatile uint32_t*)(UART0_BASE + 0x3C))
#define UART0_MIS ((volatile uint32_t*)(UART0_BASE + 0x40))
#define UART0_ICR ((volatile uint32_t*)(UART0_BASE + 0x44))
#define UART0_DMACR ((volatile uint32_t*)(UART0_BASE + 0x48))
#define UART0_ITCR ((volatile uint32_t*)(UART0_BASE + 0x80))
#define UART0_ITIP ((volatile uint32_t*)(UART0_BASE + 0x84))
#define UART0_ITOP ((volatile uint32_t*)(UART0_BASE + 0x88))
#define UART0_TDR ((volatile uint32_t*)(UART0_BASE + 0x8C))

extern uint64_t __end;

void Uart0::initialise(Uart0Pins pins) {
    memory_write_barrier();

    *UART0_CR = 0;

    // Setup UART clock
    // TODO: use memory
    auto* buffer = (MailboxTagBuffer*)&__end;
    buffer->size = 9 * 4;
    buffer->code = 0;

    auto tag = buffer->tag;
    tag[0].type = MailboxTagType::SetClockRate;
    tag[0].size = 12;
    tag[0].code = 8;
    tag[0].buffer[0] = 2; // UART clock
    tag[0].buffer[1] = 4000000; // 4 Mhz
    tag[0].buffer[2] = 0; // No turbo

    tag = (MailboxTag*)((uint64_t)tag + 12 + 12);
    tag[0].type = MailboxTagType::End;

    if (!Mailbox::callTag(buffer)) {
        Uart1::write("Error calling mailbox tag from UART0\n");
        return;
    }

    // Enable Uart0 on GPIO
    switch (pins) {
        case Uart0Pins::GPIO_14_15_16_17:
            setGpioPin(1, 4, GpioMode::Function0);
            setGpioPin(1, 5, GpioMode::Function0);
            setGpioPinUpDown((1u << 14u) | (1u << 15u), 0, GpioPinupMode::Off);
            break;

        case Uart0Pins::GPIO_30_31_32_33:
            setGpioPin(3, 2, GpioMode::Function3);
            setGpioPin(3, 3, GpioMode::Function3);
            setGpioPinUpDown(0, (1u << 0u) | (1u << 1u), GpioPinupMode::Off);
            break;

        case Uart0Pins::GPIO_36_37_38_39:
            setGpioPin(3, 6, GpioMode::Function2);
            setGpioPin(3, 7, GpioMode::Function2);
            setGpioPinUpDown(0, (1u << 4u) | (1u << 5u), GpioPinupMode::Off);
            break;
    }

    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 3u << 5u; // 8n1
    *UART0_CR = 0x301;     // enable Tx, Rx, FIFO

    memory_read_barrier();
}

char Uart0::read() {
    // While there is no data
    while (*UART0_FR & 0x10u) {
        asm("nop");
    }

    return (char)*UART0_DR;
}

void Uart0::write(char value) {
    // While we can't send
    while (*UART0_FR & 0x20u) {
        asm("nop");
    }

    *UART0_DR = value;
}

void Uart0::write(const char* value) {
    while (*value) {
        if (*value == '\n') {
            write('\r');
        }
        write(*value++);
    }
}
