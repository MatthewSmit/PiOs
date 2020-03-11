#include "pager.h"
#include "uart0.h"

static uint64_t numberPages;
static uint64_t* physicalMap;

static int findFirstZero(uint64_t value)
{
    uint64_t count;
    uint64_t inverted;

    asm volatile("rbit %1, %2\n"
                 "clz %0, %1\n" :
                 "=r"(count), "=&r"(inverted) :
                 "r"(~value));
    return count;
}

static void markPage(uint64_t pageIndex, bool inUse) {
    auto offset = pageIndex / 64;
    auto bitOffset = pageIndex % 64;
    if (inUse) {
        physicalMap[offset] |= 1ULL << bitOffset;
    } else {
        physicalMap[offset] &= ~(1ULL << bitOffset);
    }
}


void get_memory_information(ATag* tag, uint64_t& start, uint64_t& end) {
    while (tag->type != ATagType::None) {
        if (tag->type == ATagType::Memory) {
            start = tag->memory.Start;
            end = ((uint64_t)tag->memory.Start + tag->memory.Size);
            break;
        }
        tag = (ATag*)((uint64_t)tag + (tag->size * 4));
    }
}

void Pager::initialise(ATag *atags, uint64_t end) {
    uint64_t memoryStart;
    uint64_t memoryEnd;
    get_memory_information((ATag*)atags, memoryStart, memoryEnd);
    Uart0::write(memoryEnd);
    Uart0::write("\n");
    Uart0::write(end);
    Uart0::write("\n");

    numberPages = memoryEnd >> PAGE_SIZE_BITS;
    Uart0::write(numberPages);
    Uart0::write("\n");

    physicalMap = (uint64_t*)end;
    end += ((numberPages / 64) + PAGE_SIZE) & ~(PAGE_SIZE - 1ULL);

    Uart0::write(end);
    Uart0::write("\n");

    for (auto i = 0ULL; i < end >> PAGE_SIZE_BITS; i++) {
        markPage(i, true);
    }
}

void* Pager::getPage() {
    for (auto i = 0ULL; i < numberPages; i++) {
        if (physicalMap[i] != 0xFFFFFFFFFFFFFFFF) {
            auto bit = findFirstZero(physicalMap[i]);
            markPage(i * 64 + bit, true);
            return reinterpret_cast<void*>((i * 64 + bit) << PAGE_SIZE_BITS);
        }
    }

    return nullptr;
}

void Pager::freePage(void* page) {
    // TODO: Verify is not before end of kernal binary
    markPage((uint64_t)page >> PAGE_SIZE_BITS, false);
}
