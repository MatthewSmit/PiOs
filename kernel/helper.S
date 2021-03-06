.section ".text"

.globl changeState
changeState:
// Saves all general purpose registers (except x18) into the stack.
.saveState:
    sub sp, sp, #(256)
    stp x0, x1, [sp, #0 * 16]
    stp x2, x3, [sp, #1 * 16]
    stp x4, x5, [sp, #2 * 16]
    stp x6, x7, [sp, #3 * 16]
    stp x8, x9, [sp, #4 * 16]
    stp x10, x11, [sp, #5 * 16]
    stp x12, x13, [sp, #6 * 16]
    stp x14, x15, [sp, #7 * 16]
    stp x16, x17, [sp, #8 * 16]
    stp x18, x19, [sp, #9 * 16]
    stp x20, x21, [sp, #10 * 16]
    stp x22, x23, [sp, #11 * 16]
    stp x24, x25, [sp, #12 * 16]
    stp x26, x27, [sp, #13 * 16]
    stp x28, x29, [sp, #14 * 16]
    str x30, [sp, #15 * 16]

// Saves the old program counter & stack pointer to addressess in x0/x1
.saveOldPCSP:
    adr x8, .return
    str x8, [x0]
    mov x8, sp
    str x8, [x1]

// Loads the new program counter & stack pointer from x2/x3
.loadPCSP:
    mov sp, x3
    mov x18, x2

// Restores saved general purpose registers
.loadState:
    ldp x0, x1, [sp, #0 * 16]
    ldp x2, x3, [sp, #1 * 16]
    ldp x4, x5, [sp, #2 * 16]
    ldp x6, x7, [sp, #3 * 16]
    ldp x8, x9, [sp, #4 * 16]
    ldp x10, x11, [sp, #5 * 16]
    ldp x12, x13, [sp, #6 * 16]
    ldp x14, x15, [sp, #7 * 16]
    ldp x16, x17, [sp, #8 * 16]
    ldr x19, [sp, #9 * 16 + 8]
    ldp x20, x21, [sp, #10 * 16]
    ldp x22, x23, [sp, #11 * 16]
    ldp x24, x25, [sp, #12 * 16]
    ldp x26, x27, [sp, #13 * 16]
    ldp x28, x29, [sp, #14 * 16]
    ldr x30, [sp, #15 * 16]
    add sp, sp, #(256)
    // Jump to the saved program counter
    br x18

// This is the address that we jump back to
.return:
    ret
