/*
 * Most of this taken from Redboot hal_platform_setup.h with cleanup
 *
 * NOTE: I haven't clean this up considerably, just enough to get it
 * running. See hal_platform_setup.h for the source. See
 * board/cradle/memsetup.S for another PXA250 setup that is
 * much cleaner.
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

DRAM_SIZE:  .long   CFG_DRAM_SIZE

/* wait for coprocessor write complete */
   .macro CPWAIT reg
   mrc  p15,0,\reg,c2,c0,0
   mov  \reg,\reg
   sub  pc,pc,#4
   .endm


.globl memsetup
memsetup:

    mov      r10, lr

    /* Set up GPIO pins first */

	ldr		r0,	=GPSR0
	ldr		r1,	=CFG_GPSR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPSR1
	ldr		r1,	=CFG_GPSR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPSR2
	ldr		r1,	=CFG_GPSR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR0
	ldr		r1,	=CFG_GPCR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR1
	ldr		r1,	=CFG_GPCR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR2
	ldr		r1,	=CFG_GPCR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR0
	ldr		r1,	=CFG_GPDR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR1
	ldr		r1,	=CFG_GPDR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR2
	ldr		r1,	=CFG_GPDR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR0_L
	ldr		r1,	=CFG_GAFR0_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR0_U
	ldr		r1,	=CFG_GAFR0_U_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR1_L
	ldr		r1,	=CFG_GAFR1_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR1_U
	ldr		r1,	=CFG_GAFR1_U_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR2_L
	ldr		r1,	=CFG_GAFR2_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR2_U
	ldr		r1,	=CFG_GAFR2_U_VAL
	str		r1,   [r0]

   /* enable GPIO pins */
	ldr		r0,	=PSSR
	ldr		r1,	=CFG_PSSR_VAL
	str		r1,   [r0]

   ldr    r3, =MSC1		    		/* low - bank 2 Lubbock Registers / SRAM */
   ldr    r2, =CFG_MSC1_VAL			/* high - bank 3 Ethernet Controller */
   str    r2, [r3]  				/* need to set MSC1 before trying to write to the HEX LEDs */
   ldr    r2, [r3]  				/* need to read it back to make sure the value latches (see MSC section of manual) */

   ldr    r1, =LED_BLANK
   mov    r0, #0xFF
   str    r0, [r1]    		/* turn on hex leds */

loop:
   ldr		r0, =0xB0070001
   ldr    	r1, =_LED
   str    	r0, [r1]    							/* hex display */

/*********************************************************************
    Initlialize Memory Controller
	 The sequence below is based on the recommended init steps detailed
	 in the EAS, chapter 5 (Chapter 10, Operating Systems Developers Guide)


    pause for 200 uSecs- allow internal clocks to settle
	 *Note: only need this if hard reset... doing it anyway for now
*/

	@ ---- Wait 200 usec
	ldr r3, =OSCR       @ reset the OS Timer Count to zero
	mov r2, #0
	str r2, [r3]
	ldr r4, =0x300			@ really 0x2E1 is about 200usec, so 0x300 should be plenty
1:
	ldr r2, [r3]
	cmp r4, r2
	bgt 1b

mem_init:
        @ get memory controller base address
        ldr     r1,  =MEMC_BASE

@****************************************************************************
@  Step 1
@

        @ write msc0, read back to ensure data latches
        @
        ldr     r2,   =CFG_MSC0_VAL
        str     r2,   [r1, #MSC0_OFFSET]
        ldr     r2,   [r1, #MSC0_OFFSET]

        @ write msc1
        ldr     r2,  =CFG_MSC1_VAL
        str     r2,  [r1, #MSC1_OFFSET]
        ldr     r2,  [r1, #MSC1_OFFSET]

        @ write msc2
        ldr     r2,  =CFG_MSC2_VAL
        str     r2,  [r1, #MSC2_OFFSET]
        ldr     r2,  [r1, #MSC2_OFFSET]

        @ write mecr
        ldr     r2,  =CFG_MECR_VAL
        str     r2,  [r1, #MECR_OFFSET]

        @ write mcmem0
        ldr     r2,  =CFG_MCMEM0_VAL
        str     r2,  [r1, #MCMEM0_OFFSET]

        @ write mcmem1
        ldr     r2,  =CFG_MCMEM1_VAL
        str     r2,  [r1, #MCMEM1_OFFSET]

        @ write mcatt0
        ldr     r2,  =CFG_MCATT0_VAL
        str     r2,  [r1, #MCATT0_OFFSET]

        @ write mcatt1
        ldr     r2,  =CFG_MCATT1_VAL
        str     r2,  [r1, #MCATT1_OFFSET]

        @ write mcio0
        ldr     r2,  =CFG_MCIO0_VAL
        str     r2,  [r1, #MCIO0_OFFSET]

        @ write mcio1
        ldr     r2,  =CFG_MCIO1_VAL
        str     r2,  [r1, #MCIO1_OFFSET]

        @-------------------------------------------------------
        @ 3rd bullet, Step 1
        @

        @ get the mdrefr settings
        ldr     r3,  =CFG_MDREFR_VAL_100

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

@****************************************************************************
@  Step 2
@
        /* This should be for SRAM, why is it commented out??? */

        @ fetch sxcnfg value
        @
        @ldr     r2,  =0
        @ write back sxcnfg
        @str     r2,  [r1, #SXCNFG_OFFSET]

/*        @if sxcnfg=0, don't program for synch-static memory */
        @cmp     r2,  #0
        @beq     1f

        @program sxmrs
        @ldr     r2,  =SXMRS_SETTINGS
        @str     r2,  [r1, #SXMRS_OFFSET]


@****************************************************************************
@  Step 3
@

        @ Assumes previous mdrefr value in r4, if not then read current mdrefr

        @ clear the free-running clock bits
        @ (clear K0Free, K1Free, K2Free
        @
        bic     r4,  r4,  #(0x00800000 | 0x01000000 | 0x02000000)

        @ set K1RUN if bank 0 installed
        @
        orr   r4,  r4,  #0x00010000



#ifdef THIS
@<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<
@<!<!<!<!<!<!<!<!<!<!<!  Begin INSERT 1    <!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ Lubbock: Allow the user to select the {T/R/M} with predetermined
        @   SDCLK.  Based on Table 3-1 in PXA250 and PXA210 Dev Man.
        @
        @  * = Must set MDREFR.K1DB2 to halve the MemClk for desired SDCLK[1]
        @
        @   S25, S26 used to provide all 400 MHz BIN values for Cotulla (0,0 - 1,3)
        @   S25, S26 used to provide all 200 MHz BIN values for Sabinal
        @
        @   S23: Force the halving of MemClk when deriving SDCLK[1]
        @        DOT: no override  !DOT: halve (if not already forced half)
/*        @        *For certain MemClks, SDCLK's derivation is forced to be halved */
        @
        @   S24: Run/Turbo.
        @        DOT: Run mode   !DOT: Turbo mode
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        @
        @ Allow the user to control K1DB2 where applicable
        @
        @ Get the value of S23:          @ 1 = DOT (unity), 0 = !DOT (halve it)
        @
        @ DOT:   set K1DB2     (SDCLD = MemClk)
        @ !DOT:  clear K1DB2   (SDCLK = MemClk/2)
        @
        @ldr r2, =FPGA_REGS_BASE_PHYSICAL

        bl GET_S23                          @ r3, r2                    @ get the value of S23 in R0, i put the base adx of fpga in r3

        cmp      r3, #0x0                 @ is !DOT?
        orreq    r4, r4,  #0x00020000     @ SDClk[1] = MemClk/2
        bicne    r4, r4,  #0x00020000     @ SDClk[1] = MemClk

        @
        @ Next, we need to look for S25,S26 selections that necessitate the
        @  halving of MemClk to derive SDCLK[1]: (S25,S26)={03-0C, 10-13}
        @ Override above S23-based selection accordingly.
        @
        ldr r2, =FPGA_REGS_BASE_PHYSICAL
        bl  GET_S25                           @ r0, r2
                                       @ get the value of S25 in R0, i put the base adx of fpga in r2



        ldr r2, =FPGA_REGS_BASE_PHYSICAL
        BL  GET_S26                              @ r3, r2
                                      @ get the value of S26 in R1, i put the base adx of fpga in r2

        orr     r0, r0, r3               @ concatenate S25 & S26 vals
        and     r0, r0, #0xFF

        @ Set K1DB2 for the frequencies that require it
        @
        cmp     r0, #0x03
        cmpne   r0, #0x04
        cmpne   r0, #0x05
        cmpne   r0, #0x06
        cmpne   r0, #0x07
        cmpne   r0, #0x08
        cmpne   r0, #0x09
        cmpne   r0, #0x0A
        cmpne   r0, #0x0B
        cmpne   r0, #0x0C
        cmpne   r0, #0x10
        cmpne   r0, #0x11
        cmpne   r0, #0x12
        cmpne   r0, #0x13
        orreq   r4, r4,  #0x00020000     @ SDCLK[1] = (MemClk)/2 for 03 - 0C @ 10 - 13

        @
        @ *Must make MSC0&1 adjustments now for MEMClks > 100MHz.
        @
        @ Adjust MSC0 for MemClks > 100 MHz
        @
        ldreq   r0,   [r1, #MSC0_OFFSET]
        ldreq   r3,   =0x7F007F00
        biceq   r0,   r0, r3                @ clear MSC0[14:12, 11:8] (RRR, RDN)
        ldreq   r3,   =0x46004600
        orreq   r0,   r0, r3                @ set   MSC0[14, 10:9]  (doubling RRR, RDN)
        streq   r0,   [r1, #MSC0_OFFSET]
        ldreq   r0,   [r1, #MSC0_OFFSET]    @ read it back to ensure that the data latches

        @
        @ Adjust MSC1.LH for MemClks > 100 MHz
        @
        ldreq   r0,   [r1, #MSC1_OFFSET]
        ldreq   r3,   =0x7FF0
        biceq   r0,   r0, r3               @ clear MSC1[14:12, 11:8, 7:4] (RRR, RDN, RDF)
        ldreq   r3,   =0x4880
        orreq   r0,   r0, r3               @ set   MSC1[14, 11, 7]  (doubling RRR, RDN, RDF)
        streq   r0,   [r1, #MSC1_OFFSET]
        ldreq   r0,   [r1, #MSC1_OFFSET]   @ read it back to ensure that the data latches

        @                                                                   @
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#endif

@<!<!<!<!<!<!<!<!<!<!<!    End INSERT 1    <!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<
@<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<


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


@****************************************************************************
@  Step 4
@

        @ fetch platform value of mdcnfg
        @
        ldr     r2,  =CFG_MDCNFG_VAL

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

@ ****************************************************************************
@  Step 5
@

        @ pause for 200 uSecs
        @
    	ldr r3, =OSCR   @reset the OS Timer Count to zero
    	mov r2, #0
	    str r2, [r3]
	    ldr r4, =0x300			@really 0x2E1 is about 200usec, so 0x300 should be plenty
1:
	    ldr r2, [r3]
	    cmp r4, r2
	    bgt 1b


@****************************************************************************
@  Step 6
@

	    mov    r0, #0x78				    @turn everything off
      mcr    p15, 0, r0, c1, c0, 0		@(caches off, MMU off, etc.)


@ ****************************************************************************
@  Step 7
@
        @ Access memory *not yet enabled* for CBR refresh cycles (8)
        @ - CBR is generated for all banks

	    ldr     r2, =CFG_DRAM_BASE
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]


@ ****************************************************************************
@  Step 8: NOP (enable dcache if you wanna... we dont)
@


@ ****************************************************************************
@  Step 9
@


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


@****************************************************************************
@  Step 10
@

        @ write mdmrs
        @
        ldr     r2,  =CFG_MDMRS_VAL
        str     r2,  [r1, #MDMRS_OFFSET]


@****************************************************************************
@  Step 11: Final Step
@

@INITINTC
        @********************************************************************
        @ Disable (mask) all interrupts at the interrupt controller
        @

        @ clear the interrupt level register (use IRQ, not FIQ)
        @
        mov     r1, #0
        ldr     r2,  =ICLR
        str     r1,  [r2]

        @ mask all interrupts at the controller
        @
        ldr     r2,  =ICMR
        str     r1,  [r2]


@INITCLKS
        @ ********************************************************************
        @ Disable the peripheral clocks, and set the core clock
        @ frequency (hard-coding at 398.12MHz for now).
        @

		@ Turn Off ALL on-chip peripheral clocks for re-configuration
		@ *Note: See label 'ENABLECLKS' for the re-enabling
		@
        ldr     r1,  =CKEN
        mov     r2,  #0
        str     r2,  [r1]


        @ default value in case no valid rotary switch setting is found
        ldr     r2, =(CCCR_L27 | CCCR_M2 | CCCR_N10)        @ DEFAULT: {200/200/100}


        @... and write the core clock config register
        @
        ldr     r1,  =CCCR
        str     r2,  [r1]

/*        @ enable the 32Khz oscillator for RTC and PowerManager
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
*/

@OSCC_OON_DONE


#ifdef  A0_COTULLA
    @****************************************************************************
    @ !!! Take care of A0 Errata Sighting #4 --
    @ after a frequency change, the memory controller must be restarted
    @

        @ get memory controller base address
        ldr     r1,  =MEMC_BASE

        @ get the current state of MDREFR
        @
        ldr     r2,  [r1, #MDREFR_OFFSET]

        @ clear E0PIN, E1PIN
        @
        bic     r3,  r2,  #(MDREFR_E0PIN | MDREFR_E1PIN)

        @ write MDREFR with E0PIN, E1PIN cleared (disable sdclk[0,1])
        @
        str     r3,  [r1, #MDREFR_OFFSET]

        @ then write MDREFR with E0PIN, E1PIN set (enable sdclk[0,1])
        @
        str     r2,  [r1, #MDREFR_OFFSET]

        @ get the current state of MDCNFG
        @
        ldr     r3,  [r1, #MDCNFG_OFFSET]

        @ disable all SDRAM banks
        @
        bic     r3,  r3,  #(MDCNFG_DE0 | MDCNFG_DE1)
        bic     r3,  r3,  #(MDCNFG_DE2 |  MDCNFG_DE3)

        @ write back MDCNFG
        @
        ldr     r3,  [r1, #MDCNFG_OFFSET]

	    @ Access memory not yet enabled for CBR refresh cycles (8)
        @ - CBR is generated for *all* banks
	    ldr     r2, =CFG_DRAM_BASE
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]

        @ fetch current mdcnfg value
        @
        ldr     r3,  [r1, #MDCNFG_OFFSET]

        @ enable sdram bank 0 if installed
        @
        orr     r3,  r3,  #MDCNFG_DE0

        @ write back mdcnfg, enabling the sdram bank(s)
        @
        str     r3,  [r1, #MDCNFG_OFFSET]

        @ write mdmrs
        @
        ldr     r2,  =CFG_MDMRS_VAL
        str     r2,  [r1, #MDMRS_OFFSET]



    /*    @ errata: don't enable auto power-down */
        @ get current value of mdrefr
        @ldr     r3,  [r1, #MDREFR_OFFSET]
        @ enable auto-power down
        @orr     r3,  r3,  #MDREFR_APD
        @write back mdrefr
        @str     r3,  [r1, #MDREFR_OFFSET]

#endif A0_Cotulla


  ldr     r0, =0x000C0dE3
  ldr    	r1, =_LED
  str    	r0, [r1]    		/* hex display */

@ ^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%
@ ^%^%^%^%^%^%^%^%^%   above could be replaced by prememLLI ^%^%^%^%^%^%^%^%^%
@ ^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%^%


	/* Save SDRAM size */
    ldr     r1, =DRAM_SIZE
	 str	   r8, [r1]

    ldr     r0, =0xC0DE0006
    ldr    	r1, =_LED
    str    	r0, [r1]    		/* hex display */

	/* Interrupt init */
	/* Mask all interrupts */
    ldr	r0, =ICMR /* enable no sources */
	mov r1, #0
    str r1, [r0]

#define NODEBUG
#ifdef NODEBUG
	/*Disable software and data breakpoints */
	mov	r0,#0
	mcr	p15,0,r0,c14,c8,0  /* ibcr0 */
	mcr	p15,0,r0,c14,c9,0  /* ibcr1 */
	mcr	p15,0,r0,c14,c4,0  /* dbcon */

	/*Enable all debug functionality */
	mov	r0,#0x80000000
	mcr	p14,0,r0,c10,c0,0  /* dcsr */

#endif

	ldr     r0, =0xBEEF001D
    ldr    	r1, =_LED
    str    	r0, [r1]    		/* hex display */

	mov	pc, r10

@ End memsetup

@ %%%%%%%%%%%   Useful subroutines
GET_S23:
    @ This macro will read S23 and return its value in r3
    @ r2 contains the base address of the Lubbock user registers
    ldr r2, =FPGA_REGS_BASE_PHYSICAL

    /*@ read S23's value */
    ldr     r3, [r2, #USER_SWITCHES_OFFSET]

    @ mask out irrelevant bits
    and     r3, r3, #0x200

    @ get bit into position 0
    mov     r3, r3, LSR #9

    mov     pc, lr
@ End GET_S23


GET_S24:
    @ This macro will read S24 and return its value in r0
    @ r2 contains the base address of the Lubbock user registers
    ldr r2, =FPGA_REGS_BASE_PHYSICAL

    /*@ read S24's value */
    ldr     r0, [r2, #USER_SWITCHES_OFFSET]

    @ mask out irrelevant bits
    and     r0, r0, #0x100

    @ get bit into position 0
    mov     r0, r0, LSR #8

    mov     pc, lr
@ End GET_S23


GET_S25:
    @ This macro will read rotary S25 and return its value in r0
    @ r2 contains the base address of the Lubbock user registers
    @ read the user switches register
    ldr     r0, [r2, #USER_SWITCHES_OFFSET]

    @ mask out irrelevant bits
    and     r0, r0, #0xF0

    mov     pc, lr
@ End subroutine


GET_S26:
    @ This macro will read rotary S26 and return its value in r3
    @ r2 contains the base address of the Lubbock user registers
    @ read the user switches register
    ldr     r3, [r2, #USER_SWITCHES_OFFSET]

    @ mask out irrelevant bits
    and     r3, r3, #0x0F

    mov     pc, lr
@ End subroutine GET_S26


