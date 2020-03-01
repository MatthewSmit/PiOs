#include "ktypes.h"

#include "emmc.h"
#include "mailbox.h"
#include "uart0.h"
#include "uart1.h"

uint64_t mmioBase;

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

void startBootstrap();

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

void detectVersion() {
    uint64_t reg;
    asm volatile("mrs %0, midr_el1" : "=r" (reg));

    switch ((reg >> 4u) & 0xFFFu) {
        // RPI 1
        case 0xB76: mmioBase = 0x20000000; break;

        // RPI 2/3
        case 0xC07:
        case 0xD03: mmioBase = 0x3F000000; break;

        // RPI 4
        case 0xD08: mmioBase = 0xFE000000; break;

        default:    mmioBase = 0x20000000; break;
    }
}

uint64_t ptr;

void* createBlock() {
    auto block = (void*)ptr;
    ptr += 40960;
    return block;
}

extern "C" void* memset(void* ptr, int value, size_t num) {
    uint8_t* tmp = static_cast<uint8_t*>(ptr);
    for (auto i = 0; i < num; i++) {
        tmp[i] = value;
    }
    return ptr;
}

extern "C" void* memcpy(void* destination, const void* source, size_t num) {
    const uint8_t* src = static_cast<const uint8_t*>(source);
    uint8_t* dst = static_cast<uint8_t*>(destination);
    for (auto i = 0; i < num; i++) {
        dst[i] = src[i];
    }
    return destination;
}

enum class ThreadState {
    Stopped,
    Ready,
    Running,
};

typedef __attribute__((__vector_size__(16))) int8_t __neon_int8x16_t;

struct Registers {
    uint64_t generalPurpose[31];
    __neon_int8x16_t simd[31];
    uint64_t nzcv;
    uint64_t daif;
    uint64_t pc;
    uint64_t sp;

//    TTBR0_EL1 and TTBR0.
//    Thread Process ID (TPIDxxx) Registers.
//    Address Space ID (ASID).
};

struct Thread {
    ThreadState state;
    Registers registers;
};

Thread threads[3];
uint32_t currentThread;

void threadYield();

void threadInitialise() {
    ptr = (uint64_t)&__end;
    currentThread = 0;
    threads[0].state = ThreadState::Running;
}

void threadCreateStub() {
    Uart0::write("XXXX: \n");
    Uart0::write("XXXX2: ");
    while (true);
    void (*actualFunction)();
    asm volatile("mov %0, x30" : "=r" (actualFunction));
    actualFunction();
    threads[currentThread].state = ThreadState::Stopped;
    threadYield();
}

void threadCreate(void (*func)(uint32_t), uint32_t i, Thread& thread) {
    thread.state = ThreadState::Stopped;
    thread.registers = Registers{};
    thread.registers.generalPurpose[0] = i;
    thread.registers.generalPurpose[30] = (uint64_t)func;
    thread.registers.sp = (uint64_t)createBlock() + 40960;
    thread.registers.pc = (uint64_t)threadCreateStub;
}

void threadDelete(Thread& thread) {
    thread.state = ThreadState::Stopped;
}

void saveState(Thread& thread, void* returnPC) {
    // Since we're yielding, all GP registers are either caller saved (thus saved already) or callee saved(and we can discard them)
    thread.registers.pc = (uint64_t)returnPC;
    asm volatile("mov %0, sp" : "=r" (thread.registers.sp));
}

void restoreState(Thread& thread) {
//    asm volatile("mov x0, %0" : : "r" (thread.registers.generalPurpose[0]));
//    asm volatile("mov x1, %0" : : "r" (thread.registers.generalPurpose[1]));
//    asm volatile("mov x2, %0" : : "r" (thread.registers.generalPurpose[2]));
//    asm volatile("mov x3, %0" : : "r" (thread.registers.generalPurpose[3]));
//    asm volatile("mov x4, %0" : : "r" (thread.registers.generalPurpose[4]));
//    asm volatile("mov x5, %0" : : "r" (thread.registers.generalPurpose[5]));
//    asm volatile("mov x6, %0" : : "r" (thread.registers.generalPurpose[6]));
//    asm volatile("mov x7, %0" : : "r" (thread.registers.generalPurpose[7]));
    uint64_t sp;
    asm volatile("mov %0, sp" : "=r" (sp));
    Uart0::write("Original SP: ");
    Uart0::write(sp);
    Uart0::write('\n');
    Uart0::write("To set SP: ");
    Uart0::write(thread.registers.sp);
    Uart0::write('\n');

    asm volatile("mov sp, %0" : : "r" (thread.registers.sp));

//    asm volatile("mov %0, sp" : "=r" (sp));
//    Uart0::write("New SP: ");
//    Uart0::write(sp);
//    Uart0::write('\n');

    asm volatile("mov x9, %0; br x9" : : "r" (thread.registers.pc));
}

void threadYield() {
    saveState(threads[currentThread], &&onRestore);
    if (threads[currentThread].state == ThreadState::Running) {
        threads[currentThread].state = ThreadState::Ready;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        currentThread++;
        if (currentThread >= 3) {
            currentThread = 0;
        }
        if (threads[currentThread].state == ThreadState::Ready) {
            restoreState(threads[currentThread]);
        }
    }
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-label"
    onRestore:
    return;
#pragma clang diagnostic pop
}

void threadStart(Thread& thread) {
    thread.state = ThreadState::Ready;
}

void threadJoin(Thread& thread) {
    while (thread.state != ThreadState::Stopped) {
        threadYield();
    }
}

void threadFunc(uint32_t arg0) {
    for (auto i = 0u; i < 10; i++) {
        Uart0::write("From Thread ");
        Uart0::write(arg0);
        Uart0::write("; ");
        Uart0::write(i);
        Uart0::write('\n');
        threadYield();
    }
}

void threadTest() {
    threadInitialise();
    threadCreate(threadFunc, 0, threads[1]);
    threadCreate(threadFunc, 1, threads[2]);

    threadStart(threads[1]);
    threadStart(threads[2]);

    threadJoin(threads[1]);
    threadJoin(threads[2]);

    threadDelete(threads[1]);
    threadDelete(threads[2]);
}

extern "C" void main(uint64_t atags) {
    detectVersion();

    Uart1::initialise(Uart1Pins::GPIO_30_31_32_33);
    Uart0::initialise(Uart0Pins::GPIO_14_15_16_17);
    Uart0::write("Hello, kernel World!\n");

//    Emmc::initialise();

    void* memoryStart;
    void* memoryEnd;
    get_memory_information((ATag*)atags, memoryStart, memoryEnd);
    Uart0::write((uint64_t)memoryStart);
    Uart0::write("\n");
    Uart0::write((uint64_t)memoryEnd);
    Uart0::write("\n");
    Uart0::write((uint64_t)&__end);
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
        Uart0::write(*(uint64_t*)buffer->tag[0].buffer);
        Uart0::write('\n');
    } else {
        Uart0::write("Error calling mailbox tag\n");
    }

    threadTest();
//    startBootstrap();

    Uart0::write("Waiting\n");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        Uart0::write(Uart0::read());
    }
#pragma clang diagnostic pop
}

//void startFileManager() {
//    uint64_t size;
//    auto rawFile = loadFileInternal("/apps/internal/fileManager/fileManager", size);
//    ProcessData processData{};
//    processData.data = rawFile;
//    processData.size = size;
//    processData.pid = 1;
//    processData.permissions.memory = ~0ULL;
//    processData.permissions.file = ~0ULL;
//    createProcess(&processData);
//    waitUntilReady(&processData);
//    freeInternal(rawFile);
//}
//
//void startBootstrap() {
//    // TODO: Setup IPC
//
//    // TODO: Create process table
//
//    // TODO: Handle memory somehow
//
//    startFileManager();
//
//    // TODO: Start Serial Process
//
//    // TODO: Start GPU Process
//
//    // TODO: Start User Stuff
//}