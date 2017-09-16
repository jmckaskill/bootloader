.global setup
.global interrupt
.global _start

.code 32

_start:
    // disable interrupts
    cpsid if

    // setup stack
    ldr r0, stack_top
    mov sp, r0

    bl setup

    // setup irq stack
    mov r0, sp
    msr cpsr_c, #0x12
    mov sp, r0
    msr cpsr_c, #0x13

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
    stmfd sp!,{r0-r4,r12,lr}
    bl interrupt
    // reset the interrupt controller
    mov r0, #1
    ldr r1, hw_intc_control
    str r0, [r1]
    // need to synchronize after resetting the peripherals
    // this way it's processed before the arm core returns from the interrupt
    dsb
    ldmfd sp!,{r0-r4,r12,lr}
    subs PC, lr, #4

stack_top:
    .word 0x402FFFFC;
hw_intc_control:
    .word 0x48200048;

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

    
