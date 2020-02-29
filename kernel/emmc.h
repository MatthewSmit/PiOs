#pragma once

#include "ktypes.h"

class Emmc {
public:
    static void initialise();

    static bool isCardPresent();

    static void reset();
};