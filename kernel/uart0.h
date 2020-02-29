#pragma once

#include "ktypes.h"

enum class Uart0Pins {
    GPIO_14_15_16_17,
    GPIO_30_31_32_33,
    GPIO_36_37_38_39,
};

class Uart0 {
public:
    static void initialise(Uart0Pins pins);

    static char read();

    static void write(char value);
    static void write(const char* value);
    static void write(uint32_t value);
    static void write(uint64_t value);
};