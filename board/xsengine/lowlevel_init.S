#include <config.h>
#include <version.h>
#include <asm/arch/pxa-regs.h>

DRAM_SIZE:  .long   CFG_DRAM_SIZE

.globl lowlevel_init
lowlevel_init:

   mov      r10, lr

/* ---- GPIO INITIALISATION ---- */
/* Set up GPIO pins first (3 groups [31:0] [63:32] [80:64]) */

   /* General purpose set registers */
   ldr      r0,   =GPSR0
   ldr      r1,   =CFG_GPSR0_VAL
   str      r1,   [r0]
   ldr      r0,   =GPSR1
   ldr      r1,   =CFG_GPSR1_VAL
   str      r1,   [r0]
   ldr      r0,   =GPSR2
   ldr      r1,   =CFG_GPSR2_VAL
   str      r1,   [r0]

   /* General purpose clear registers */
   ldr      r0,   =GPCR0
   ldr      r1,   =CFG_GPCR0_VAL
   str      r1,   [r0]
   ldr      r0,   =GPCR1
   ldr      r1,   =CFG_GPCR1_VAL
   str      r1,   [r0]
   ldr      r0,   =GPCR2
   ldr      r1,   =CFG_GPCR2_VAL
   str      r1,   [r0]

   /* General rising edge registers */
   ldr      r0,   =GRER0
   ldr      r1,   =CFG_GRER0_VAL
   str      r1,   [r0]
   ldr      r0,   =GRER1
   ldr      r1,   =CFG_GRER1_VAL
   str      r1,   [r0]
   ldr      r0,   =GRER2
   ldr      r1,   =CFG_GRER2_VAL
   str      r1,   [r0]

   /* General falling edge registers */
   ldr      r0,   =GFER0
   ldr      r1,   =CFG_GFER0_VAL
   str      r1,   [r0]
   ldr      r0,   =GFER1
   ldr      r1,   =CFG_GFER1_VAL
   str      r1,   [r0]
   ldr      r0,   =GFER2
   ldr      r1,   =CFG_GFER2_VAL
   str      r1,   [r0]

   /* General edge detect registers */
   ldr      r0,   =GPDR0
   ldr      r1,   =CFG_GPDR0_VAL
   str      r1,   [r0]
   ldr      r0,   =GPDR1
   ldr      r1,   =CFG_GPDR1_VAL
   str      r1,   [r0]
   ldr      r0,   =GPDR2
   ldr      r1,   =CFG_GPDR2_VAL
   str      r1,   [r0]

   /* General alternate function registers */
   ldr      r0,   =GAFR0_L		/* [0:15] */
   ldr      r1,   =CFG_GAFR0_L_VAL
   str      r1,   [r0]
   ldr      r0,   =GAFR0_U		/* [31:16] */
   ldr      r1,   =CFG_GAFR0_U_VAL
   str      r1,   [r0]
   ldr      r0,   =GAFR1_L		/* [47:32] */
   ldr      r1,   =CFG_GAFR1_L_VAL
   str      r1,   [r0]
   ldr      r0,   =GAFR1_U		/* [63:48] */
   ldr      r1,   =CFG_GAFR1_U_VAL
   str      r1,   [r0]
   ldr      r0,   =GAFR2_L		/* [79:64] */
   ldr      r1,   =CFG_GAFR2_L_VAL
   str      r1,   [r0]
   ldr      r0,   =GAFR2_U		/* [80] */
   ldr      r1,   =CFG_GAFR2_U_VAL
   str      r1,   [r0]

   /* General purpose direction registers */
   ldr      r0,   =GPDR0
   ldr      r1,   =CFG_GPDR0_VAL
   str      r1,   [r0]
   ldr      r0,   =GPDR1
   ldr      r1,   =CFG_GPDR1_VAL
   str      r1,   [r0]
   ldr      r0,   =GPDR2
   ldr      r1,   =CFG_GPDR2_VAL
   str      r1,   [r0]

   /* Power manager sleep status */
   ldr      r0,   =PSSR
   ldr      r1,   =CFG_PSSR_VAL
   str      r1,   [r0]

/* ---- MEMORY INITIALISATION ---- */
/* Initialize Memory Controller, see PXA250 Operating System Developer's Guide */
/* pause for 200 uSecs- allow internal clocks to settle */
   ldr r3, =OSCR	/* reset the OS Timer Count to zero */
   mov r2, #0
   str r2, [r3]
   ldr r4, =0x300  	/* really 0x2E1 is about 200usec, so 0x300 should be plenty */
1:
   ldr r2, [r3]
   cmp r4, r2
   bgt 1b

mem_init:
/* get memory controller base address */
   ldr     r1,  =MEMC_BASE

/* ---- FLASH INITIALISATION ---- */
/* Write MSC0 and read back to ensure data change is accepted by cpu */
   ldr     r2,   =CFG_MSC0_VAL
   str     r2,   [r1, #MSC0_OFFSET]
   ldr     r2,   [r1, #MSC0_OFFSET]

/* ---- SDRAM INITIALISATION ---- */
/* get the MDREFR settings */
   ldr     r2,  =CFG_MDREFR_VAL
   str     r2,  [r1, #MDREFR_OFFSET]

/* fetch platform value of MDCNFG */
   ldr     r2,  =CFG_MDCNFG_VAL

/* disable all sdram banks */
   bic     r2,  r2,  #(MDCNFG_DE0 | MDCNFG_DE1)
   bic     r2,  r2,  #(MDCNFG_DE2 | MDCNFG_DE3)

/* write initial value of MDCNFG, w/o enabling sdram banks */
   str     r2,  [r1, #MDCNFG_OFFSET]

/* pause for 200 uSecs */
   ldr r3, =OSCR	/* reset the OS Timer Count to zero */
   mov r2, #0
   str r2, [r3]
   ldr r4, =0x300  	/* about 200 usec */
1:
   ldr r2, [r3]
   cmp r4, r2
   bgt 1b

/* Access memory *not yet enabled* for CBR refresh cycles (8) */
/* CBR is generated for all banks */

   ldr     r2, =CFG_DRAM_BASE
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]
   str     r2, [r2]

/* get memory controller base address */
   ldr     r2,  =MEMC_BASE

/* Enable SDRAM bank 0 in MDCNFG register */
   ldr     r2,  [r1, #MDCNFG_OFFSET]
   orr     r2,  r2,  #MDCNFG_DE0
   str     r2,  [r1, #MDCNFG_OFFSET]

/* write MDMRS to trigger an MSR command to all enabled SDRAM banks */
   ldr     r2,  =CFG_MDMRS_VAL
   str     r2,  [r1, #MDMRS_OFFSET]

/* ---- INTERRUPT INITIALISATION ---- */
/* Disable (mask) all interrupts at the interrupt controller */
/* clear the interrupt level register (use IRQ, not FIQ) */
   mov     r1, #0
   ldr     r2,  =ICLR
   str     r1,  [r2]

/* Set interrupt mask register */
   ldr     r1,  =CFG_ICMR_VAL
   ldr     r2,  =ICMR
   str     r1,  [r2]

/* ---- CLOCK INITIALISATION ---- */
/* Disable the peripheral clocks, and set the core clock */

/* Turn Off ALL on-chip peripheral clocks for re-configuration */
   ldr     r1,  =CKEN
   mov     r2,  #0
   str     r2,  [r1]

/* set core clocks */
   ldr     r2,  =CFG_CCCR_VAL
   ldr     r1,  =CCCR
   str     r2,  [r1]

#ifdef ENABLE32KHZ
/* enable the 32Khz oscillator for RTC and PowerManager */
   ldr     r1,  =OSCC
   mov     r2,  #OSCC_OON
   str     r2,  [r1]

/* NOTE:  spin here until OSCC.OOK get set, meaning the PLL has settled. */
60:
   ldr     r2, [r1]
   ands    r2, r2, #1
   beq     60b
#endif

/* Turn on needed clocks */
   ldr     r1,  =CKEN
   ldr     r2,  =CFG_CKEN_VAL
   str     r2,  [r1]

   mov   pc, r10
