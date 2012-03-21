/*
 * Memory Setup - initialize memory controller(s) for devices required
 *                to boot and relocate
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


/* memory controller */
#define BCRX_DEFAULT	(0x0000fbe0)
#define BCRX_MW_8	(0x00000000)
#define BCRX_MW_16	(0x10000000)
#define BCRX_MW_32	(0x20000000)
#define BCRX_PME	(0x08000000)
#define BCRX_WP		(0x04000000)
#define BCRX_WST2_SHIFT	(11)
#define BCRX_WST1_SHIFT	(5)
#define BCRX_IDCY_SHIFT	(0)

/* Bank0 Async Flash */
#define BCR0		(0x80002000)
#define BCR0_FLASH	(BCRX_MW_32 | (0x08<<BCRX_WST2_SHIFT) | (0x0E<<BCRX_WST1_SHIFT))

/* Bank1 Open */
#define BCR1	(0x80002004)

/* Bank2 Not used (EEPROM?) */
#define BCR2	(0x80002008)

/* Bank3 Not used */
#define BCR3	(0x8000200C)

/* Bank4 PC Card1 */

/* Bank5 PC Card2 */

/* Bank6 CPLD IO Controller Peripherals (slow) */
#define BCR6		(0x80002018)
#define BCR6_CPLD_SLOW	(BCRX_DEFAULT | BCRX_MW_16)

/* Bank7 CPLD IO Controller Peripherals (fast) */
#define BCR7		(0x8000201C)
#define BCR7_CPLD_FAST	(BCRX_MW_16 | (0x16<<BCRX_WST2_SHIFT) | (0x16<<BCRX_WST1_SHIFT) | (0x2<<BCRX_IDCY_SHIFT))

/* SDRAM */
#define GBLCNFG		(0x80002404)
#define GC_CKE		(0x80000000)
#define GC_CKSD		(0x40000000)
#define GC_LCR		(0x00000040)
#define GC_SMEMBURST	(0x00000020)
#define GC_MRS		(0x00000002)
#define GC_INIT		(0x00000001)

#define GC_CMD_NORMAL		(GC_CKE)
#define GC_CMD_MODE		(GC_CKE | GC_MRS)
#define GC_CMD_SYNCFLASH_LOAD	(GC_CKE | GC_MRS | GC_LCR)
#define GC_CMD_PRECHARGEALL	(GC_CKE | GC_INIT)
#define GC_CMD_NOP		(GC_CKE | GC_INIT | GC_MRS)

#define RFSHTMR		(0x80002408)
#define RFSHTMR_INIT	(10)	/* period=100 ns, HCLK=100Mhz, (2048+1-15.6*66) */
#define RFSHTMR_NORMAL	(1500)	/* period=15.6 us, HCLK=100Mhz, (2048+1-15.6*66) */

#define SDCSCX_BASE		(0x80002410)
#define SDCSCX_DEFAULT		(0x01220008)
#define SDCSCX_AUTOPC		(0x01000000)
#define SDCSCX_RAS2CAS_2	(0x00200000)
#define SDCSCX_RAS2CAS_3	(0x00300000)
#define SDCSCX_WBL		(0x00080000)
#define SDCSCX_CASLAT_8		(0x00070000)
#define SDCSCX_CASLAT_7		(0x00060000)
#define SDCSCX_CASLAT_6		(0x00050000)
#define SDCSCX_CASLAT_5		(0x00040000)
#define SDCSCX_CASLAT_4		(0x00030000)
#define SDCSCX_CASLAT_3		(0x00020000)
#define SDCSCX_CASLAT_2		(0x00010000)
#define SDCSCX_2KPAGE		(0x00000040)
#define SDCSCX_SROMLL		(0x00000020)
#define SDCSCX_SROM512		(0x00000010)
#define SDCSCX_4BNK		(0x00000008)
#define SDCSCX_2BNK		(0x00000000)
#define SDCSCX_EBW_16		(0x00000004)
#define SDCSCX_EBW_32		(0x00000000)

#define SDRAM_BASE		(0xC0000000)
#define SDCSC_BANK_OFFSET	(0x10000000)

/*
 * The SDRAM DEVICE MODE PROGRAMMING VALUE
 */
#define BURST_LENGTH_4		(2 << 10)
#define BURST_LENGTH_8		(3 << 10)
#define WBURST_LENGTH_BL	(0 << 19)
#define WBURST_LENGTH_SINGLE	(1 << 19)
#define CAS_2			(2 << 14)
#define CAS_3			(3 << 14)
#define BAT_SEQUENTIAL		(0 << 13)
#define BAT_INTERLEAVED		(1 << 13)
#define OPM_NORMAL		(0 << 17)
#define SDRAM_DEVICE_MODE	(WBURST_LENGTH_BL|OPM_NORMAL|CAS_3|BAT_SEQUENTIAL|BURST_LENGTH_4)


#define TIMER1_BASE	(0x80000C00)

/*
 * special lookup flags
 */
#define DO_MEM_DELAY	1
#define DO_MEM_READ	2

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

.globl lowlevel_init
lowlevel_init:
	mov	r9, lr	@ save return address

	/* memory control configuration */
	/* make r0 relative the current location so that it */
	/* reads INITMEM_DATA out of FLASH rather than memory ! */
	/*   r0 = current word pointer */
	/*   r1 = end word location, one word past last actual word */
	/*   r3 = address for writes, special lookup flags */
	/*   r4 = value for writes, delay constants, or read addresses */
	/*   r2 = location for mem reads */

	ldr	r0, =INITMEM_DATA
	ldr	r1, _TEXT_BASE
	sub	r0, r0, r1
	add	r1, r0, #112

mem_loop:
	cmp	r1, r0
	moveq	pc, r9		@ Done

	ldr	r3, [r0], #4	@ Fetch Destination Register Address, or 1 for delay
	ldr	r4, [r0], #4	@ value

	cmp	r3, #DO_MEM_DELAY
	bleq	mem_delay
	beq	mem_loop
	cmp	r3, #DO_MEM_READ
	ldreq	r2, [r4]
	beq	mem_loop
	str	r4, [r3]	@ normal register/ram store
	b	mem_loop

mem_delay:
	ldr	r5, =TIMER1_BASE
	mov	r6, r4, LSR #1	@ timer resolution is ~2us
	str	r6, [r5]
	mov	r6, #0x88	@ using 508.469KHz clock, enable
	str	r6, [r5, #8]
0:	ldr	r6, [r5, #4]	@ timer value
	cmp	r6, #0
	bne	0b
	mov	r6, #0		@ disable timer
	str	r6, [r5, #8]
	mov	pc, lr

	.ltorg
/* the literal pools origin */

INITMEM_DATA:
	.word	BCR0
	.word	BCR0_FLASH
	.word	BCR6
	.word	BCR6_CPLD_SLOW
	.word	BCR7
	.word	BCR7_CPLD_FAST
	.word	SDCSCX_BASE
	.word	(SDCSCX_RAS2CAS_3 | SDCSCX_CASLAT_3 | SDCSCX_SROMLL | SDCSCX_4BNK | SDCSCX_EBW_32)
	.word	GBLCNFG
	.word	GC_CMD_NOP
	.word	DO_MEM_DELAY
	.word	200
	.word	GBLCNFG
	.word	GC_CMD_PRECHARGEALL
	.word	RFSHTMR
	.word	RFSHTMR_INIT
	.word	DO_MEM_DELAY
	.word	8
	.word	RFSHTMR
	.word	RFSHTMR_NORMAL
	.word	GBLCNFG
	.word	GC_CMD_MODE
	.word	DO_MEM_READ
	.word	(SDRAM_BASE | SDRAM_DEVICE_MODE)
	.word	GBLCNFG
	.word	GC_CMD_NORMAL
	.word	SDCSCX_BASE
	.word	(SDCSCX_AUTOPC | SDCSCX_RAS2CAS_3 | SDCSCX_CASLAT_3 | SDCSCX_SROMLL | SDCSCX_4BNK | SDCSCX_EBW_32)
