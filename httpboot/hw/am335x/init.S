.global setup
.global interrupt
.global _start

.code 32

.section .init
_start:
    // disable interrupts
    cpsid if

    ldr sp, =_stack_top
    bl setup

    // set the exception base address
    ldr r0, =exception_table
    mcr p15, 0, r0, c12, c0, 0

    // reenable interrupts
    // everything from now on is through interrupts
    cpsie if

loop:
    wfi
    b loop

irq_entry:
    ldr sp, =_stack_top
    push {r0-r4,r12,lr}
    bl interrupt
    // need to synchronize after resetting the peripherals
    // this way it's processed before the arm core returns from the interrupt
    dmb
    pop {r0-r4,r12,lr}
    subs PC, lr, #4


// unused exception entries just loop at that address
// that way we can attach a debugger and see where it got to
.align 5 // 32 bytes
exception_table:
reset:
    b reset
undefined_instruction:
    b undefined_instruction
software_interrupt:
    b software_interrupt
prefetch_abort:
    b prefetch_abort
data_abort:
    b data_abort
reserved:
    b reserved
irq:
    b irq_entry
fiq:
    b fiq

    
