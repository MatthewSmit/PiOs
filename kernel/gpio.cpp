#include "gpio.h"

#include "BCM2837.h"

void setGpioPinMode(uint32_t pin, GpioMode mode) {
    auto bank = pin / 10;
    pin %= 10;
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

bool getGpioPin(uint32_t pin) {
    if (pin < 32) {
        return *GPLEV0 & (1u << pin);
    }
    return *GPLEV1 & (1u << (pin - 32));
}
