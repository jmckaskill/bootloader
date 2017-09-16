/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
*/
/* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/* ld script for StarterWare AM335x. */

/*
** The stack is kept at end of the image, and its size is 128 MB.
** The heap section is placed above the stack to support I/O
** operations using semihosting. The size of the section is 2KB.
*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/* entrypoint (in ARM mode, at start of image) */
ENTRY(_start)

SECTIONS {
    /* For use in user space we need to mmap the base address */
    _base = 0x402f0000;

    /* For peripheral boot, ROM places image at hardcoded addr 0x402f0400 */
    . = 0x402f0400;

    .text : {
        KEEP(*(.text._start))
        *(.text)
        *(.text.*)
    }

    .rodata : {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata.*)
        *(.ARM.attributes)
    }

    .data : {
        . = ALIGN(4);
        *(.data*)
        *(.ARM.exidx)
    }

    .bss : {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    }

    /* SRAM is 0x402F0400 - 0x402FFFFF
     * OCMC0 is 0x40300000 - 0x4030FFFF
     * SRAM is used for the program (bottom) and stack (top)
     * OCMC0 is used for the ethernet data buffers
     * ethernet data buffers is 43 * x5F0 = xFF50
     */

    _stack_top   = 0x402FFFFC;
    g_eth_frames = 0x40300000;

	HW_CM_PER		  = 0x44E00000;
	HW_CM_WAKEUP      = 0x44E00400;
    HW_CONTROL        = 0x44E10000;
    HW_GPIO_0		  = 0x44E07000;
    HW_GPIO_1		  = 0x4804C000;
    HW_GPIO_2		  = 0x481AC000;
    HW_GPIO_3		  = 0x481AE000;
    HW_UART_4         = 0x481A8000;
    HW_UART_4_CFGA    = 0x481A8000;
    HW_UART_4_CFGB    = 0x481A8000;
    HW_UART_4_OP      = 0x481A8000;
    HW_DMTIMER_1MS    = 0x44E31000;
    HW_CM_DPLL        = 0x44E00500;
    HW_INTC           = 0x48200000;
    HW_MDIO           = 0x4A101000;
    HW_CPSW_WR        = 0x4A101200;
    HW_CPSW_SS        = 0x4A100000;
    HW_CPSW_PORT_0    = 0x4A100100;
    HW_CPSW_PORT_1    = 0x4A100200;
    HW_CPSW_PORT_2    = 0x4A100300;
    HW_CPSW_CPDMA     = 0x4A100800;
    HW_CPSW_STATERAM  = 0x4A100A00;
    HW_CPSW_ALE       = 0x4A100D00;
    HW_CPSW_SL_1      = 0x4A100D80;
    HW_CPSW_SL_2      = 0x4A100DC0;
    HW_BUFFER_DESCRIPTORS = 0x4A102000;

    .symtab : {
        *(.symtab)
    }

    .strtab : {
        *(.strtab)
    }

    .shstrtab : {
        *(.shstrtab)
    }

    /DISCARD/ : {
        *(.comment)
    }

/*
    /DISCARD/ : {
        //*(.ARM.exidx)
        //*(.ARM.attributes)
		*(.comment .comment.*)
		*(.symtab .symtab.*)
		*(.strtab .strtab.*)
    }
    */

	/* catch any unknown sections and yell loudly about them */
    /*
    .orphan : { INPUT_SECTION_FLAGS (SHF_ALLOC) *(*) } :NONE
	ASSERT( SIZEOF(.orphan) == 0, "unassigned sections!" )
    */ 
}

