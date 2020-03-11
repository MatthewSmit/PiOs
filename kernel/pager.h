#pragma once

#include "ktypes.h"

enum class ATagType {
    None = 0x00000000,
    Core = 0x54410001,
    Memory = 0x54410002,
};

struct ATagMemory {
    uint32_t Size;
    uint32_t Start;
};

struct ATag {
    uint32_t size;
    ATagType type;
    union {
        ATagMemory memory;
    };
};

class Pager {
public:
    static constexpr auto PAGE_SIZE = 4096U;
    static constexpr auto PAGE_SIZE_BITS = 12U;

    static void initialise(ATag* atags, uint64_t end);

    static void* getPage();

    static void freePage(void* page);
};