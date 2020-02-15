#include "ktypes.h"

#include "mailbox.h"
#include "uart0.h"
#include "uart1.h"

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

void uart_puti(uint32_t value) {
    static const char lookup[]
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    };

    for (auto i = 0u; i < 8; i++) {
        auto chr = (value >> ((7 - i) * 4)) & 0x0F;
        Uart0::write(lookup[chr]);
    }
}

void uart_putl(uint64_t value) {
    static const char lookup[]
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    };

    for (auto i = 0u; i < 16; i++) {
        auto chr = (value >> ((15 - i) * 4)) & 0x0F;
        Uart0::write(lookup[chr]);
    }
}

void get_memory_information(ATag* tag, void*& start, void*& end) {
    while (tag->type != ATagType::None) {
        if (tag->type == ATagType::Memory) {
            start = (void*)tag->memory.Start;
            end = (void*)((uint64_t)tag->memory.Start + tag->memory.Size);
            break;
        }
        tag = (ATag*)((uint64_t)tag + (tag->size * 4));
    }
}

extern uint64_t __end;

extern "C" void main(uint64_t atags) {
    Uart1::initialise(Uart1Pins::GPIO_30_31_32_33);
    Uart0::initialise(Uart0Pins::GPIO_14_15_16_17);
    Uart0::write("Hello, kernel World!\n");

    void* memoryStart;
    void* memoryEnd;
    get_memory_information((ATag*)atags, memoryStart, memoryEnd);
    uart_putl((uint64_t)memoryStart);
    Uart0::write("\n");
    uart_putl((uint64_t)memoryEnd);
    Uart0::write("\n");
    uart_putl((uint64_t)&__end);
    Uart0::write("\n");

    // TODO: use memory
    auto* buffer = (MailboxTagBuffer*)&__end;
    buffer->size = 8 * 4;
    buffer->code = 0;

    auto tag = buffer->tag;
    tag[0].type = MailboxTagType::GetBoardSerial;
    tag[0].size = 8;
    tag[0].code = 8;
    tag[0].buffer[0] = 0;
    tag[0].buffer[1] = 0;

    tag = (MailboxTag*)((uint64_t)tag + 8 + 12);
    tag[0].type = MailboxTagType::End;

    if (Mailbox::callTag(buffer)) {
        Uart0::write("Serial: ");
        uart_putl(*(uint64_t*)buffer->tag[0].buffer);
        Uart0::write('\n');
    } else {
        Uart0::write("Error calling mailbox tag\n");
    }

    Uart0::write("Waiting\n");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        Uart0::write(Uart0::read());
    }
#pragma clang diagnostic pop
}