#pragma once

#include "ktypes.h"

class Emmc {
public:
    static void initialise();

//    static bool isCardPresent();
//
//    static void reset();

    static int readBlock(uint64_t lba, const uint8_t* buffer, uint64_t length);

    static int writeBlock(uint64_t lba, uint8_t* buffer, uint64_t length);
};