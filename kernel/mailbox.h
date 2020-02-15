#pragma once

#include "ktypes.h"
#include "BCM2837.h"

#define MAILBOX_READ ((volatile uint32_t*)(MAILBOX_BASE + 0x00))
#define MAILBOX_POLL ((volatile uint32_t*)(MAILBOX_BASE + 0x10))
#define MAILBOX_SENDER ((volatile uint32_t*)(MAILBOX_BASE + 0x14))
#define MAILBOX_STATUS ((volatile uint32_t*)(MAILBOX_BASE + 0x18))
#define MAILBOX_CONFIG ((volatile uint32_t*)(MAILBOX_BASE + 0x1C))
#define MAILBOX_WRITE ((volatile uint32_t*)(MAILBOX_BASE + 0x20))

enum class MailboxStatus : uint32_t {
    Empty = 0x40000000,
    Full = 0x80000000,
};

enum class MailboxChannel {
    Power = 0x00,
    Framebuffer = 0x01,
    VUart = 0x02,
    VCHIQ = 0x03,
    LED = 0x04,
    Buttons = 0x05,
    Touchscreen = 0x06,
    Counter = 0x07,
    Tags = 0x08,
    VCTags = 0x09,
};

enum class MailboxTagType {
    End = 0x00000000,
    GetFirmwareVersion = 0x00000001,
    GetBoardModel = 0x00010001,
    GetBoardRevision = 0x00010002,
    GetBoardMACAddress = 0x00010003,
    GetBoardSerial = 0x00010004,
    GetARMMemory = 0x00010005,
    GetVCMemory = 0x00010006,
    GetClocks = 0x00010007,
    GetCommandLine = 0x00050001,
    GetDMAChannels = 0x00060001,
    GetPowerState = 0x00020001,
    GetTiming = 0x00020002,
    SetPowerState = 0x00028001,
    GetClockState = 0x00030001,
    SetClockState = 0x00038001,
    GetClockRate = 0x00030002,
    SetClockRate = 0x00038002,
    GetMaxClockRate = 0x00030004,
    GetMinClockRate = 0x00030007,
    GetTurbo = 0x00030009,
    SetTurbo = 0x00038009,
    GetVoltage = 0x00030003,
    SetVoltage = 0x00038003,
    GetMaxVoltage = 0x00030005,
    GetMinVoltage = 0x00030008,
    GetTemperature = 0x00030006,
    GetMaxTemperature = 0x0003000A,
    AllocateMemory = 0x0003000C,
    LockMemory = 0x0003000D,
    UnlockMemory = 0x0003000E,
    ReleaseMemory = 0x0003000F,
    ExecuteCode = 0x00030010,
    GetDispmanxResourceMemoryHandle = 0x00030014,
    GetEDIDBlock = 0x00030020,
    AllocateFramebuffer = 0x00040001,
    ReleaseFramebuffer = 0x00048001,
    BlankScreen = 0x00040002,
    GetPhysicalDisplaySize = 0x00040003,
    TestPhysicalDisplaySize = 0x00044003,
    SetPhysicalDisplaySize = 0x00048003,
    GetVirtualDisplaySize = 0x00040004,
    TestVirtualDisplaySize = 0x00044004,
    SetVirtualDisplaySize = 0x00048004,
    GetDisplayDepth = 0x00040005,
    TestDisplayDepth = 0x00044005,
    SetDisplayDepth = 0x00048005,
    GetPixelOrder = 0x00040006,
    TestPixelOrder = 0x00044006,
    SetPixelOrder = 0x00048006,
    GetAlphaMode = 0x00040007,
    TestAlphaMode = 0x00044007,
    SetAlphaMode = 0x00048007,
    GetPitch = 0x00040008,
    GetVirtualOffset = 0x00040009,
    TestVirtualOffset = 0x00044009,
    SetVirtualOffset = 0x00048009,
    GetOverscan = 0x0004000A,
    TestOverscan = 0x0004400A,
    SetOverscan = 0x0004800A,
    GetPalette = 0x0004000B,
    TestPalette = 0x0004400B,
    SetPalette = 0x0004800B,
    SetCursorInfo = 0x00008010,
    SetCursorState = 0x00008011,
};

struct MailboxTag {
    MailboxTagType type;
    uint32_t size;
    uint32_t code;
    uint32_t buffer[0];
};

struct alignas(16) MailboxTagBuffer {
    uint32_t size;
    uint32_t code;
    MailboxTag tag[1];
};

class Mailbox {
public:
    static uint32_t read(MailboxChannel channel);
    static void write(MailboxChannel channel, uint32_t data);

    static bool callTag(MailboxTagBuffer* buffer);
};