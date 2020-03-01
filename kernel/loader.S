// AArch64 mode
 
// To keep this in the first portion of the binary.
.section ".text.boot"
 
// Make _start global.
.globl _start
 
    .org 0x0
// Entry point for the kernel. Registers are not defined as in AArch32.
_start:
    // read cpu id, stop slave cores
    mrs x1, mpidr_el1
    and x1, x1, #3
    cmp x1, #0
    bne halt

test_EL3:
    // set stack before our code
    ldr     x1, =_start

    // set up EL1
    mrs     x3, CurrentEL
    and     x3, x3, #12 // clear reserved bits

    // running at EL3?
    cmp     x3, #12
    bne     test_EL2
    // should never be executed, just for completeness
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, set_stack
    msr     elr_el3, x2
    eret

    // running at EL2?
test_EL2:
    cmp     x3, #4
    beq     set_stack
    msr     sp_el1, x1
    // enable CNTP for EL1
    mrs     x3, cnthctl_el2
    orr     x3, x3, #3
    msr     cnthctl_el2, x3
    msr     cntvoff_el2, xzr
    // enable AArch64 in EL1
    mov     x3, #(1 << 31)      // AArch64
    orr     x3, x3, #(1 << 1)   // SWIO hardwired on Pi3
    msr     hcr_el2, x3
    mrs     x3, hcr_el2
    // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, set_stack
    msr     elr_el2, x2
    eret

set_stack:
    mov     sp, x1
 
    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, bss_cleared
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

bss_cleared:
    // jump to C code, should not return
    bl      main

// Halt the core
halt:
    wfe
    b halt
