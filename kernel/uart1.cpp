#include "uart1.h"

#include "BCM2837.h"
#include "gpio.h"

void Uart1::initialise(Uart1Pins pins) {
    memory_write_barrier();

    // Enable Uart1 on GPIO
    switch (pins) {
        case Uart1Pins::GPIO_14_15_16_17:
            setGpioPinMode(14, GpioMode::Function5);
            setGpioPinMode(15, GpioMode::Function5);
            setGpioPinUpDown((1u << 14u) | (1u << 15u), 0, GpioPinupMode::Off);
            break;

        case Uart1Pins::GPIO_30_31_32_33:
            setGpioPinMode(32, GpioMode::Function5);
            setGpioPinMode(33, GpioMode::Function5);
            setGpioPinUpDown(0, (1u << 0u) | (1u << 1u), GpioPinupMode::Off);
            break;

        case Uart1Pins::GPIO_40_41_42_43:
            setGpioPinMode(40, GpioMode::Function5);
            setGpioPinMode(41, GpioMode::Function5);
            setGpioPinUpDown(0, (1u << 8u) | (1u << 9u), GpioPinupMode::Off);
            break;
    }

    // Enables mini uart
    *AUX_ENABLES = *AUX_ENABLES | 1u;

    *AUX_MU_IER_REG = 0;
    *AUX_MU_CNTL_REG = 0;
    *AUX_MU_LCR_REG = 1; // 8 bit mode
    *AUX_MU_MCR_REG = 0;
    *AUX_MU_IIR_REG = 0xc6;
    *AUX_MU_BAUD_REG = 270; // apparently 115200 baud

    *AUX_MU_CNTL_REG = 3; // Enables Tx, Rx

    memory_read_barrier();
}

char Uart1::read() {
    // While there is no data
    while (!(*AUX_MU_LSR_REG & 0x01u)) {
        asm("nop");
    }

    return (char)*AUX_MU_IO_REG;
}

void Uart1::write(char value) {
    // While we can't send
    while (!(*AUX_MU_LSR_REG & 0x20u)) {
        asm("nop");
    }

    *AUX_MU_IO_REG = value;
}

void Uart1::write(const char* value) {
    while (*value) {
        if (*value == '\n') {
            write('\r');
        }
        write(*value++);
    }
}
