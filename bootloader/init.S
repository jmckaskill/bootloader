.global setup
.global interrupt
.global _bss_start
.global _bss_end
.global _stack_top
.global _start

.set MODE_MASK, 0xC0
.set MODE_IRQ, 0x12
.set MODE_SUPER, 0x13

.code 32

_start:
    // disable interrupts
    cpsid if

    // setup stack
    ldr r0, =_stack_top
    mov sp, r0

    bl setup

    // setup irq stack
    msr cpsr_c, #MODE_IRQ|MODE_MASK
    ldr r0, =_stack_top
    mov sp, r0
    msr cpsr_c, #MODE_SUPER|MODE_MASK

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


    
    
