#pragma once

#include "ktypes.h"
#include "BCM2837.h"

#define GPFSEL0 ((volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPFSEL1 ((volatile uint32_t*)(GPIO_BASE + 0x04))
#define GPFSEL2 ((volatile uint32_t*)(GPIO_BASE + 0x08))
#define GPFSEL3 ((volatile uint32_t*)(GPIO_BASE + 0x0C))
#define GPFSEL4 ((volatile uint32_t*)(GPIO_BASE + 0x10))
#define GPFSEL5 ((volatile uint32_t*)(GPIO_BASE + 0x14))
#define GPSET0 ((volatile uint32_t*)(GPIO_BASE + 0x1C))
#define GPSET1 ((volatile uint32_t*)(GPIO_BASE + 0x20))
#define GPCLR0 ((volatile uint32_t*)(GPIO_BASE + 0x28))
#define GPCLR1 ((volatile uint32_t*)(GPIO_BASE + 0x2C))
#define GPLEV0 ((volatile uint32_t*)(GPIO_BASE + 0x34))
#define GPLEV1 ((volatile uint32_t*)(GPIO_BASE + 0x38))
#define GPEDS0 ((volatile uint32_t*)(GPIO_BASE + 0x40))
#define GPEDS1 ((volatile uint32_t*)(GPIO_BASE + 0x44))
#define GPREN0 ((volatile uint32_t*)(GPIO_BASE + 0x4C))
#define GPREN1 ((volatile uint32_t*)(GPIO_BASE + 0x50))
#define GPFEN0 ((volatile uint32_t*)(GPIO_BASE + 0x58))
#define GPFEN1 ((volatile uint32_t*)(GPIO_BASE + 0x5C))
#define GPHEN0 ((volatile uint32_t*)(GPIO_BASE + 0x64))
#define GPHEN1 ((volatile uint32_t*)(GPIO_BASE + 0x68))
#define GPLEN0 ((volatile uint32_t*)(GPIO_BASE + 0x70))
#define GPLEN1 ((volatile uint32_t*)(GPIO_BASE + 0x74))
#define GPAREN0 ((volatile uint32_t*)(GPIO_BASE + 0x7C))
#define GPAREN1 ((volatile uint32_t*)(GPIO_BASE + 0x80))
#define GPAFEN0 ((volatile uint32_t*)(GPIO_BASE + 0x88))
#define GPAFEN1 ((volatile uint32_t*)(GPIO_BASE + 0x8C))
#define GPPUD ((volatile uint32_t*)(GPIO_BASE + 0x94))
#define GPPUDCLK0 ((volatile uint32_t*)(GPIO_BASE + 0x98))
#define GPPUDCLK1 ((volatile uint32_t*)(GPIO_BASE + 0x9C))

enum class GpioMode {
    Input = 0,
    Output = 1,
    Function0 = 4,
    Function1 = 5,
    Function2 = 6,
    Function3 = 7,
    Function4 = 3,
    Function5 = 2,
};

enum class GpioPinupMode {
    Off = 0,
    PullDown = 1,
    PullUp = 2,
};

void setGpioPinMode(uint32_t pin, GpioMode mode);
void setGpioPinUpDown(uint32_t pins0, uint32_t pins1, GpioPinupMode mode);
bool getGpioPin(uint32_t pin);