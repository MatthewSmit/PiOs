#include "mailbox.h"

uint32_t Mailbox::read(MailboxChannel channel) {
    while (true) {
        while (*MAILBOX_STATUS & (uint32_t)MailboxStatus::Empty) {
            asm volatile("nop");
        }

        const auto value = *MAILBOX_READ;
        if ((value & 0x0Fu) != (uint32_t)channel) {
            continue;
        }

        return value & 0xFFFFFFF0;
    }
}

void Mailbox::write(MailboxChannel channel, uint32_t data) {
    const auto value = (data & 0xFFFFFFF0u) | ((uint32_t)channel & 0x0Fu);
    while (*MAILBOX_STATUS & (uint32_t)MailboxStatus::Full) {
        asm volatile("nop");
    }

    *MAILBOX_WRITE = value;
}

bool Mailbox::callTag(MailboxTagBuffer* buffer) {
    write(MailboxChannel::Tags, (uint32_t)(uint64_t)buffer);
    read(MailboxChannel::Tags);
    return buffer->code == 0x80000000;
}
