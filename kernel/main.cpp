#include "ktypes.h"

#include "mailbox.h"
#include "uart0.h"
#include "uart1.h"
#include "pager.h"

uint64_t mmioBase;

void debugPrint() {
    // Semihosting interface
    auto tmp = "Hello Debug\r\n\0";
    asm volatile("mov x0, #0x4\n"
                 "mov x1, %0\n"
                 "hlt #0xF000"
    :
    : "r"(tmp));
}

void queryEL() {
    uint64_t el;
    asm volatile ("mrs %0, CurrentEL" : "=r" (el));

    Uart0::write("Current EL is: ");
    Uart0::write((el >> 2) & 3);
    Uart0::write("\n");
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
    // Either dead or unborn.
    Stopped,
    // Ready to run.
    Ready,
    // Is running.
    Running,
};

typedef __attribute__((__vector_size__(16))) int8_t __neon_int8x16_t;

struct Registers {
    uint64_t gp[32];
};

static_assert(sizeof(Registers) == 256);

struct Thread {
    ThreadState state;
    uint64_t pc;
    uint64_t sp;
};

constexpr auto NUMBER_THREADS = 3;
Thread threads[NUMBER_THREADS];
uint32_t currentThread;

void threadYield();

void threadInitialise() {
    currentThread = 0;
    threads[0].state = ThreadState::Running;
}

void threadCreateStub() {
    void (*actualFunction)();
    asm volatile("mov %0, x7" : "=r" (actualFunction));
    actualFunction();
    threads[currentThread].state = ThreadState::Stopped;
    threadYield();
}

void threadCreate(void (*func)(uint32_t), uint32_t i, Thread& thread) {
    thread.state = ThreadState::Stopped;

    // Allocates a stack of 10 pages.
    // TODO: Allocate more than 1 page
    thread.sp = (uint64_t)Pager::getPage() + 4096 - sizeof(Registers);
    thread.pc = (uint64_t)threadCreateStub;

    // Fake generate a stack frame. Arguments 0-6 will be user supplied, argument 7 will be the real function to call.
    auto registers = (Registers*)(void*)thread.sp;
    registers->gp[0] = i;
    registers->gp[7] = (uint64_t)func;
}

void threadDelete(Thread& thread) {
    // TODO: will need error checking here eventually
    // TODO: free stack buffer
    thread.state = ThreadState::Stopped;
}

extern "C" void changeState(uint64_t* oldPc, uint64_t* oldSp, uint64_t pc, uint64_t sp);

void threadYield() {
    if (threads[currentThread].state == ThreadState::Running) {
        threads[currentThread].state = ThreadState::Ready;
    }

    auto oldThreadId = currentThread;

    // Very simple round-robin scheduler.
    // Will likely need atomic state access (or locking) when multi processors implemented.
    while (true) {
        currentThread++;
        if (currentThread >= NUMBER_THREADS) {
            currentThread = 0;
        }

        if (threads[currentThread].state == ThreadState::Ready) {
            changeState(&threads[oldThreadId].pc, &threads[oldThreadId].sp, threads[currentThread].pc, threads[currentThread].sp);
            return;
        }
    }
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
    threadCreate(threadFunc, 0x0BAD, threads[1]);
    threadCreate(threadFunc, 0x0CAF, threads[2]);

    threadStart(threads[1]);
    threadStart(threads[2]);

    threadJoin(threads[1]);
    threadJoin(threads[2]);

    threadDelete(threads[1]);
    threadDelete(threads[2]);
}

extern "C" uint64_t setupPaging();

extern "C" void main(uint64_t atags, uint64_t dataEnd) {
    // Bootstrap maps MMIO to this address (-2GB)
    mmioBase = 0xFFFFFFFF80000000;

    Pager::initialise((ATag*)atags, dataEnd);

    Uart1::initialise(Uart1Pins::GPIO_30_31_32_33);
    Uart0::initialise(Uart0Pins::GPIO_14_15_16_17);
    Uart0::write("Hello, kernel World!\n");

//    Emmc::initialise();

    queryEL();

    static_assert(sizeof(MailboxTagBuffer) < Pager::PAGE_SIZE);
    auto buffer = (MailboxTagBuffer*)Pager::getPage();
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
    Pager::freePage(buffer);

//    threadTest();
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