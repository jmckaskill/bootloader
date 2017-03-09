// ******************************************************************************
//
// init.S - Init code routines
//
// ******************************************************************************
//
// Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ******************************************************************************

#include <hw_gpio_v2.h>
#include <hw_cm_per.h>

// ****************************** Global Symbols*********************************
        .global _start
        .global _stack                  
        .global _bss_start
        .global _bss_end
        .global main

// ************************ Internal Definitions ********************************
//
// Define the stack sizes for different modes. The user/system mode will use
// the rest of the total stack size
//
        .set  UND_STACK_SIZE, 0x8
        .set  ABT_STACK_SIZE, 0x8
        .set  FIQ_STACK_SIZE, 0x8
        .set  IRQ_STACK_SIZE, 0x1000
        .set  SVC_STACK_SIZE, 0x8

//
// to set the mode bits in CPSR for different modes
//        
        .set  MODE_USR, 0x10            
        .set  MODE_FIQ, 0x11
        .set  MODE_IRQ, 0x12
        .set  MODE_SVC, 0x13
        .set  MODE_ABT, 0x17
        .set  MODE_UND, 0x1B
        .set  MODE_SYS, 0x1F            

        .equ  I_F_BIT, 0xC0               

// **************************** Code Seection ***********************************
        .text

//
// This code is assembled for ARM instructions
//
        .code 32

// ******************************************************************************
//
// ******************************************************************************
//
// The reset handler in StarterWare is named as 'Entry'.
// The reset handler sets up the stack pointers for all the modes. The FIQ and
// IRQ shall be disabled during this. Then clear the BSS sections and finally
// switch to the function calling the main() function.
//
_start:
    ldr	r0, prcm_base

	movs	r1, 2
	str	r1, [ r0, #CM_PER_L4LS_CLKSTCTRL ]
	orrs	r1, 1 << 18		// debounce clock enable
	str	r1, [ r0, #CM_PER_GPIO1_CLKCTRL ]
	str	r1, [ r0, #CM_PER_GPIO2_CLKCTRL ]

    // wait for clocks to come online
1:	ldr	r1, [ r0, #CM_PER_GPIO1_CLKCTRL ]
	tst	r1, 3 << 16
	bne	1b
1:	ldr	r1, [ r0, #CM_PER_GPIO2_CLKCTRL ]
	tst	r1, 3 << 16
	bne	1b

    ldr	r0, io1_base	// leds are 1.21-24

	movs	r1, 2
	str	r1, [ r0, #GPIO_SYSCONFIG ] // enable wakeup on input
1:	ldr	r1, [ r0, #GPIO_SYSSTATUS ]
	tst	r1, 1 // check that the internal reset is finished
	beq	1b

	movs	r1, 0x10
	str	r1, [ r0, #GPIO_SYSCONFIG ] // set smart-idle

	movs	r1, 0
	str	r1, [ r0, #GPIO_CTRL ] // disable clock gating

	mvns	r1, 0xf << 21
	str	r1, [ r0, #GPIO_OE ] // set pins as output
	movs	r1, 1 << 21
	str	r1, [ r0, #GPIO_SETDATAOUT ] // set them


//
// Set up the Stack for Undefined mode
//
         LDR   r0, =_stack                     @ Read the stack address
         //MOV   r0,sp
         MSR   cpsr_c, #MODE_UND|I_F_BIT       @ switch to undef  mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0, r0, #UND_STACK_SIZE         @ give stack space
//
// Set up the Stack for abort mode
//        
         MSR   cpsr_c, #MODE_ABT|I_F_BIT       @ Change to abort mode
         MOV   sp, r0                          @ write the stack pointer
         SUB   r0,r0, #ABT_STACK_SIZE          @ give stack space
//
// Set up the Stack for FIQ mode
//       
         MSR   cpsr_c, #MODE_FIQ|I_F_BIT       @ change to FIQ mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #FIQ_STACK_SIZE          @ give stack space
//
// Set up the Stack for IRQ mode
//       
         MSR   cpsr_c, #MODE_IRQ|I_F_BIT       @ change to IRQ mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #IRQ_STACK_SIZE          @ give stack space
//
// Set up the Stack for SVC mode
//        
         MSR   cpsr_c, #MODE_SVC|I_F_BIT       @ change to SVC mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #SVC_STACK_SIZE          @ give stack space
//
// Set up the Stack for USer/System mode
//      
         MSR   cpsr_c, #MODE_SYS|I_F_BIT       @ change to system mode
         MOV   sp,r0                           @ write the stack pointer

// Invalidate and Enable Branch Prediction  
         //MOV     r0, #0
         //MCR     p15, #0, r0, c7, c5, #6
         //ISB
         //MRC     p15, #0, r0, c1, c0, #0
         //ORR     r0, r0, #0x00000800
         //MCR     p15, #0, r0, c1, c0, #0

//
// Enable Neon/VFP Co-Processor
//
		//MRC p15, #0, r1, c1, c0, #2           @ r1 = Access Control Register
		//ORR r1, r1, #(0xf << 20)              @ enable full access for p10,11
		//MCR p15, #0, r1, c1, c0, #2           @ Access Control Register = r1
		//MOV r1, #0
		//MCR p15, #0, r1, c7, c5, #4           @flush prefetch buffer
		//MOV r0,#0x40000000
		//FMXR FPEXC, r0                        @ Set Neon/VFP Enable bit

//
// Clear the BSS section here
//
Clear_Bss_Section:

         LDR   r0, =_bss_start                 @ Start address of BSS
         LDR   r1, =(_bss_end - 0x04)          @ End address of BSS
         MOV   r2, #0  
Loop: 
         //STR   r2, [r0], #4                    @ Clear one word in BSS
         CMP   r0, r1
         BLE   Loop                            @ Clear till BSS end

//
// Enter the start_boot function. The execution still happens in system mode.
//
Enter_main:
        push {r0}
         LDR   r10,=main
         MOV   lr,pc                           @ Dummy return from start_boot
         BX    r10                             @ Branch to start_boot
         SUB   pc, pc, #0x08                   @ looping
    b led2

led2:
    ldr	r0, io1_base	// leds are 1.21-24
    movs	r1, 1 << 22
	str	r1, [ r0, #GPIO_SETDATAOUT ] // set them
    b led2

prcm_base:
	.word	0x44e00000
io1_base:
	.word	0x4804c000
io2_base:
	.word	0x481ac000

//
// End of the file
//
         .end
    
    



