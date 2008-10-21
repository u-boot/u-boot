/*
 * Most of this taken from Redboot hal_platform_setup.h with cleanup
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>
#include <asm/arch/pxa-regs.h>

DRAM_SIZE:  .long   CONFIG_SYS_DRAM_SIZE

/* wait for coprocessor write complete */
   .macro CPWAIT reg
   mrc  p15,0,\reg,c2,c0,0
   mov  \reg,\reg
   sub  pc,pc,#4
   .endm

   .macro SET_LED val
   ldr   r6, =CRADLE_LED_CLR_REG
   ldr   r7, =0
   str   r7, [r6]
   ldr   r6, =CRADLE_LED_SET_REG
   ldr   r7, =\val
   str   r7, [r6]
   .endm


.globl lowlevel_init
lowlevel_init:

    mov      r10, lr

    /* Set up GPIO pins first */

   ldr      r0,   =GPSR0
   ldr      r1,   =CONFIG_SYS_GPSR0_VAL
   str      r1,   [r0]

   ldr      r0,   =GPSR1
   ldr      r1,   =CONFIG_SYS_GPSR1_VAL
   str      r1,   [r0]

   ldr      r0,   =GPSR2
   ldr      r1,   =CONFIG_SYS_GPSR2_VAL
   str      r1,   [r0]

   ldr      r0,   =GPCR0
   ldr      r1,   =CONFIG_SYS_GPCR0_VAL
   str      r1,   [r0]

   ldr      r0,   =GPCR1
   ldr      r1,   =CONFIG_SYS_GPCR1_VAL
   str      r1,   [r0]

   ldr      r0,   =GPCR2
   ldr      r1,   =CONFIG_SYS_GPCR2_VAL
   str      r1,   [r0]

   ldr      r0,   =GRER0
   ldr      r1,   =CONFIG_SYS_GRER0_VAL
   str      r1,   [r0]

   ldr      r0,   =GRER1
   ldr      r1,   =CONFIG_SYS_GRER1_VAL
   str      r1,   [r0]

   ldr      r0,   =GRER2
   ldr      r1,   =CONFIG_SYS_GRER2_VAL
   str      r1,   [r0]

   ldr      r0,   =GFER0
   ldr      r1,   =CONFIG_SYS_GFER0_VAL
   str      r1,   [r0]

   ldr      r0,   =GFER1
   ldr      r1,   =CONFIG_SYS_GFER1_VAL
   str      r1,   [r0]

   ldr      r0,   =GFER2
   ldr      r1,   =CONFIG_SYS_GFER2_VAL
   str      r1,   [r0]

   ldr      r0,   =GPDR0
   ldr      r1,   =CONFIG_SYS_GPDR0_VAL
   str      r1,   [r0]

   ldr      r0,   =GPDR1
   ldr      r1,   =CONFIG_SYS_GPDR1_VAL
   str      r1,   [r0]

   ldr      r0,   =GPDR2
   ldr      r1,   =CONFIG_SYS_GPDR2_VAL
   str      r1,   [r0]

   ldr      r0,   =GAFR0_L
   ldr      r1,   =CONFIG_SYS_GAFR0_L_VAL
   str      r1,   [r0]

   ldr      r0,   =GAFR0_U
   ldr      r1,   =CONFIG_SYS_GAFR0_U_VAL
   str      r1,   [r0]

   ldr      r0,   =GAFR1_L
   ldr      r1,   =CONFIG_SYS_GAFR1_L_VAL
   str      r1,   [r0]

   ldr      r0,   =GAFR1_U
   ldr      r1,   =CONFIG_SYS_GAFR1_U_VAL
   str      r1,   [r0]

   ldr      r0,   =GAFR2_L
   ldr      r1,   =CONFIG_SYS_GAFR2_L_VAL
   str      r1,   [r0]

   ldr      r0,   =GAFR2_U
   ldr      r1,   =CONFIG_SYS_GAFR2_U_VAL
   str      r1,   [r0]

   /* enable GPIO pins */
   ldr      r0,   =PSSR
   ldr      r1,   =CONFIG_SYS_PSSR_VAL
   str      r1,   [r0]

   SET_LED 1

   ldr    r3, =MSC1             /* low - bank 2 Lubbock Registers / SRAM */
   ldr    r2, =CONFIG_SYS_MSC1_VAL     /* high - bank 3 Ethernet Controller */
   str    r2, [r3]              /* need to set MSC1 before trying to write to the HEX LEDs */
   ldr    r2, [r3]              /* need to read it back to make sure the value latches (see MSC section of manual) */


/*********************************************************************
    Initlialize Memory Controller

    See PXA250 Operating System Developer's Guide

    pause for 200 uSecs- allow internal clocks to settle
    *Note: only need this if hard reset... doing it anyway for now
*/

    @ Step 1
   @ ---- Wait 200 usec
   ldr r3, =OSCR       @ reset the OS Timer Count to zero
   mov r2, #0
   str r2, [r3]
   ldr r4, =0x300         @ really 0x2E1 is about 200usec, so 0x300 should be plenty
1:
   ldr r2, [r3]
   cmp r4, r2
   bgt 1b

   SET_LED 2

mem_init:
	@ get memory controller base address
	ldr     r1,  =MEMC_BASE


@****************************************************************************
@  Step 2
@

   @ Step 2a
   @ write msc0, read back to ensure data latches
   @
   ldr     r2,   =CONFIG_SYS_MSC0_VAL
   str     r2,   [r1, #MSC0_OFFSET]
   ldr     r2,   [r1, #MSC0_OFFSET]

   @ write msc1
   ldr     r2,  =CONFIG_SYS_MSC1_VAL
   str     r2,  [r1, #MSC1_OFFSET]
   ldr     r2,  [r1, #MSC1_OFFSET]

   @ write msc2
   ldr     r2,  =CONFIG_SYS_MSC2_VAL
   str     r2,  [r1, #MSC2_OFFSET]
   ldr     r2,  [r1, #MSC2_OFFSET]

   @ Step 2b
   @ write mecr
   ldr     r2,  =CONFIG_SYS_MECR_VAL
   str     r2,  [r1, #MECR_OFFSET]

   @ write mcmem0
   ldr     r2,  =CONFIG_SYS_MCMEM0_VAL
   str     r2,  [r1, #MCMEM0_OFFSET]

   @ write mcmem1
   ldr     r2,  =CONFIG_SYS_MCMEM1_VAL
   str     r2,  [r1, #MCMEM1_OFFSET]

   @ write mcatt0
   ldr     r2,  =CONFIG_SYS_MCATT0_VAL
   str     r2,  [r1, #MCATT0_OFFSET]

   @ write mcatt1
   ldr     r2,  =CONFIG_SYS_MCATT1_VAL
   str     r2,  [r1, #MCATT1_OFFSET]

   @ write mcio0
   ldr     r2,  =CONFIG_SYS_MCIO0_VAL
   str     r2,  [r1, #MCIO0_OFFSET]

   @ write mcio1
   ldr     r2,  =CONFIG_SYS_MCIO1_VAL
   str     r2,  [r1, #MCIO1_OFFSET]

   /*SET_LED 3 */

   @ Step 2c
   @ fly-by-dma is defeatured on this part
   @ write flycnfg
   @ldr     r2,  =CONFIG_SYS_FLYCNFG_VAL
   @str     r2,  [r1, #FLYCNFG_OFFSET]

/* FIXME Does this sequence really make sense */
#ifdef REDBOOT_WAY
   @ Step 2d
   @ get the mdrefr settings
   ldr     r3,  =CONFIG_SYS_MDREFR_VAL

   @ extract DRI field (we need a valid DRI field)
   @
   ldr     r2,  =0xFFF

   @ valid DRI field in r3
   @
   and     r3,  r3,  r2

   @ get the reset state of MDREFR
   @
   ldr     r4,  [r1, #MDREFR_OFFSET]

   @ clear the DRI field
   @
   bic     r4,  r4,  r2

   @ insert the valid DRI field loaded above
   @
   orr     r4,  r4,  r3

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]

   @ *Note: preserve the mdrefr value in r4 *

   /*SET_LED 4 */

@****************************************************************************
@  Step 3
@
@ NO SRAM

   mov   pc, r10


@****************************************************************************
@  Step 4
@

   @ Assumes previous mdrefr value in r4, if not then read current mdrefr

   @ clear the free-running clock bits
   @ (clear K0Free, K1Free, K2Free
   @
   bic     r4,  r4,  #(0x00800000 | 0x01000000 | 0x02000000)

   @ set K0RUN for CPLD clock
   @
   orr   r4,  r4,  #0x00002000

   @ set K1RUN if bank 0 installed
   @
   orr   r4,  r4,  #0x00010000

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]
   ldr     r4,  [r1, #MDREFR_OFFSET]

   @ deassert SLFRSH
   @
   bic     r4,  r4,  #0x00400000

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]

   @ assert E1PIN
   @
   orr     r4,  r4,  #0x00008000

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]
   ldr     r4,  [r1, #MDREFR_OFFSET]
   nop
   nop
#else
   @ Step 2d
   @ get the mdrefr settings
   ldr     r3,  =CONFIG_SYS_MDREFR_VAL

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]

   @  Step 4

   @ set K0RUN for CPLD clock
   @
   orr   r4,  r4,  #0x00002000

   @ set K1RUN for bank 0
   @
   orr   r4,  r4,  #0x00010000

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]
   ldr     r4,  [r1, #MDREFR_OFFSET]

   @ deassert SLFRSH
   @
   bic     r4,  r4,  #0x00400000

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]

   @ assert E1PIN
   @
   orr     r4,  r4,  #0x00008000

   @ write back mdrefr
   @
   str     r4,  [r1, #MDREFR_OFFSET]
   ldr     r4,  [r1, #MDREFR_OFFSET]
   nop
   nop
#endif

   @ Step 4d
   @ fetch platform value of mdcnfg
   @
   ldr     r2,  =CONFIG_SYS_MDCNFG_VAL

   @ disable all sdram banks
   @
   bic     r2,  r2,  #(MDCNFG_DE0 | MDCNFG_DE1)
   bic     r2,  r2,  #(MDCNFG_DE2 | MDCNFG_DE3)

   @ program banks 0/1 for bus width
   @
   bic   r2,  r2,  #MDCNFG_DWID0      @0=32-bit

   @ write initial value of mdcnfg, w/o enabling sdram banks
   @
   str     r2,  [r1, #MDCNFG_OFFSET]

   @ Step 4e
   @ pause for 200 uSecs
   @
   ldr r3, =OSCR       @ reset the OS Timer Count to zero
   mov r2, #0
   str r2, [r3]
   ldr r4, =0x300			@ really 0x2E1 is about 200usec, so 0x300 should be plenty
1:
   ldr r2, [r3]
   cmp r4, r2
   bgt 1b

   /*SET_LED 5 */

   /* Why is this here??? */
   mov    r0, #0x78                @turn everything off
   mcr    p15, 0, r0, c1, c0, 0      @(caches off, MMU off, etc.)

   @ Step 4f
   @ Access memory *not yet enabled* for CBR refresh cycles (8)
   @ - CBR is generated for all banks

   ldr     r2, =CONFIG_SYS_DRAM_BASE
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]

   @ Step 4g
   @get memory controller base address
   @
   ldr     r1,  =MEMC_BASE

   @fetch current mdcnfg value
   @
   ldr     r3,  [r1, #MDCNFG_OFFSET]

   @enable sdram bank 0 if installed (must do for any populated bank)
   @
   orr     r3,  r3,  #MDCNFG_DE0

   @write back mdcnfg, enabling the sdram bank(s)
   @
   str     r3,  [r1, #MDCNFG_OFFSET]

   @ Step 4h
   @ write mdmrs
   @
   ldr     r2,  =CONFIG_SYS_MDMRS_VAL
   str     r2,  [r1, #MDMRS_OFFSET]

   @ Done Memory Init

   /*SET_LED 6 */

   @********************************************************************
   @ Disable (mask) all interrupts at the interrupt controller
   @

   @ clear the interrupt level register (use IRQ, not FIQ)
   @
   mov     r1, #0
   ldr     r2,  =ICLR
   str     r1,  [r2]

   @ Set interrupt mask register
   @
   ldr     r1,  =CONFIG_SYS_ICMR_VAL
   ldr     r2,  =ICMR
   str     r1,  [r2]

   @ ********************************************************************
   @ Disable the peripheral clocks, and set the core clock
   @

	@ Turn Off ALL on-chip peripheral clocks for re-configuration
	@
   ldr     r1,  =CKEN
   mov     r2,  #0
   str     r2,  [r1]

   @ set core clocks
   @
   ldr     r2,  =CONFIG_SYS_CCCR_VAL
   ldr     r1,  =CCCR
   str     r2,  [r1]

#ifdef ENABLE32KHZ
   @ enable the 32Khz oscillator for RTC and PowerManager
   @
   ldr     r1,  =OSCC
   mov     r2,  #OSCC_OON
   str     r2,  [r1]

   @ NOTE:  spin here until OSCC.OOK get set,
   @        meaning the PLL has settled.
   @
60:
   ldr     r2, [r1]
   ands    r2, r2, #1
   beq     60b
#endif

	@ Turn on needed clocks
	@
   ldr     r1,  =CKEN
   ldr     r2,  =CONFIG_SYS_CKEN_VAL
   str     r2,  [r1]

   /*SET_LED 7 */

/* Is this needed???? */
#define NODEBUG
#ifdef NODEBUG
   /*Disable software and data breakpoints */
   mov   r0,#0
   mcr   p15,0,r0,c14,c8,0  /* ibcr0 */
   mcr   p15,0,r0,c14,c9,0  /* ibcr1 */
   mcr   p15,0,r0,c14,c4,0  /* dbcon */

   /*Enable all debug functionality */
   mov   r0,#0x80000000
   mcr   p14,0,r0,c10,c0,0  /* dcsr */

#endif

   /*SET_LED 8 */

   mov   pc, r10

@ End lowlevel_init
