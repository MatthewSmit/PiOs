#pragma once

enum class Uart1Pins {
    GPIO_14_15_16_17,
    GPIO_30_31_32_33,
    GPIO_40_41_42_43,
};

class Uart1 {
public:
    static void initialise(Uart1Pins pins);

    static char read();

    static void write(char value);
    static void write(const char* value);
};