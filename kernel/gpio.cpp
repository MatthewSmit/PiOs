#include "gpio.h"

#include "BCM2837.h"

static inline void delay(uint32_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
        : "=r"(count): [count]"0"(count) : "cc");
}

void setGpioPin(uint32_t bank, uint32_t pin, GpioMode mode) {
    const auto reg = (volatile uint32_t*)(GPIO_BASE + bank * 4);
    *reg = (*reg & ~(7u << pin * 3)) | ((uint32_t)mode << pin * 3);
}

void setGpioPinUpDown(uint32_t pins0, uint32_t pins1, GpioPinupMode mode) {
    *GPPUD = (uint32_t)mode;

    delay(150);

    *GPPUDCLK0 = pins0;
    *GPPUDCLK1 = pins1;

    delay(150);

    *GPPUD = 0;
    *GPPUDCLK0 = 0;
    *GPPUDCLK1 = 0;
}
