/*****************************************************************************
 * (C) Copyright 2003;  Tundra Semiconductor Corp.
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
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: tsi108_init.c
 *
 * Originator: Alex Bounine
 *
 * DESCRIPTION:
 * Initialization code for the Tundra Tsi108 bridge chip
 *---------------------------------------------------------------------------*/

#include <common.h>
#include <74xx_7xx.h>
#include <config.h>
#include <version.h>
#include <asm/processor.h>
#include <tsi108.h>

DECLARE_GLOBAL_DATA_PTR;

extern void mpicInit (int verbose);

/*
 * Configuration Options
 */

typedef struct {
	ulong upper;
	ulong lower;
} PB2OCN_LUT_ENTRY;

PB2OCN_LUT_ENTRY pb2ocn_lut1[32] = {
	/* 0 - 7 */
	{0x00000000, 0x00000201}, /* PBA=0xE000_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE100_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE200_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE300_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE400_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE500_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE600_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE700_0000 -> PCI/X (Byte-Swap) */

	/* 8 - 15 */
	{0x00000000, 0x00000201}, /* PBA=0xE800_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xE900_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xEA00_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xEB00_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xEC00_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xED00_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xEE00_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xEF00_0000 -> PCI/X (Byte-Swap) */

	/* 16 - 23 */
	{0x00000000, 0x00000201}, /* PBA=0xF000_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF100_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF200_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF300_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF400_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF500_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF600_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF700_0000 -> PCI/X (Byte-Swap) */
	/* 24 - 31 */
	{0x00000000, 0x00000201}, /* PBA=0xF800_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000201}, /* PBA=0xF900_0000 -> PCI/X (Byte-Swap) */
	{0x00000000, 0x00000241}, /* PBA=0xFA00_0000 -> PCI/X  PCI I/O (Byte-Swap + Translate) */
	{0x00000000, 0x00000201}, /* PBA=0xFB00_0000 -> PCI/X  PCI Config (Byte-Swap) */

	{0x00000000, 0x02000240}, /* PBA=0xFC00_0000 -> HLP */
	{0x00000000, 0x01000240}, /* PBA=0xFD00_0000 -> HLP */
	{0x00000000, 0x03000240}, /* PBA=0xFE00_0000 -> HLP */
	{0x00000000, 0x00000240}  /* PBA=0xFF00_0000 -> HLP : (Translation Enabled + Byte-Swap)*/
};

#ifdef CONFIG_SYS_CLK_SPREAD
typedef struct {
	ulong ctrl0;
	ulong ctrl1;
} PLL_CTRL_SET;

/*
 * Clock Generator SPLL0 initialization values
 * PLL0 configuration table for various PB_CLKO freq.
 * Uses pre-calculated values for Fs = 30 kHz, D = 0.5%
 * Fout depends on required PB_CLKO. Based on Fref = 33 MHz
 */

static PLL_CTRL_SET pll0_config[8] = {
	{0x00000000, 0x00000000},	/* 0: bypass */
	{0x00000000, 0x00000000},	/* 1: reserved */
	{0x00430044, 0x00000043},	/* 2: CG_PB_CLKO = 183 MHz */
	{0x005c0044, 0x00000039},	/* 3: CG_PB_CLKO = 100 MHz */
	{0x005c0044, 0x00000039},	/* 4: CG_PB_CLKO = 133 MHz */
	{0x004a0044, 0x00000040},	/* 5: CG_PB_CLKO = 167 MHz */
	{0x005c0044, 0x00000039},	/* 6: CG_PB_CLKO = 200 MHz */
	{0x004f0044, 0x0000003e}	/* 7: CG_PB_CLKO = 233 MHz */
};
#endif	/* CONFIG_SYS_CLK_SPREAD */

/*
 * Prosessor Bus Clock (in MHz) defined by CG_PB_SELECT
 * (based on recommended Tsi108 reference clock 33MHz)
 */
static int pb_clk_sel[8] = { 0, 0, 183, 100, 133, 167, 200, 233 };

/*
 * get_board_bus_clk ()
 *
 * returns the bus clock in Hz.
 */
unsigned long get_board_bus_clk (void)
{
	ulong i;

	/* Detect PB clock freq. */
	i = in32(CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET + CG_PWRUP_STATUS);
	i = (i >> 16) & 0x07;	/* Get PB PLL multiplier */

	return pb_clk_sel[i] * 1000000;
}

/*
 * board_early_init_f ()
 *
 * board-specific initialization executed from flash
 */

int board_early_init_f (void)
{
	ulong i;

	gd->mem_clk = 0;
	i = in32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET +
			CG_PWRUP_STATUS);
	i = (i >> 20) & 0x07;	/* Get GD PLL multiplier */
	switch (i) {
	case 0:	/* external clock */
		printf ("Using external clock\n");
		break;
	case 1:	/* system clock */
		gd->mem_clk = gd->bus_clk;
		break;
	case 4:	/* 133 MHz */
	case 5:	/* 166 MHz */
	case 6:	/* 200 MHz */
		gd->mem_clk = pb_clk_sel[i] * 1000000;
		break;
	default:
		printf ("Invalid DDR2 clock setting\n");
		return -1;
	}
	printf ("BUS: %lu MHz\n", get_board_bus_clk() / 1000000);
	printf ("MEM: %lu MHz\n", gd->mem_clk / 1000000);
	return 0;
}

/*
 * board_early_init_r() - Tsi108 initialization function executed right after
 * relocation. Contains code that cannot be executed from flash.
 */

int board_early_init_r (void)
{
	ulong temp, i;
	ulong reg_val;
	volatile ulong *reg_ptr;

	reg_ptr =
		(ulong *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PB_REG_OFFSET + 0x900);

	for (i = 0; i < 32; i++) {
		*reg_ptr++ = 0x00000201;	/* SWAP ENABLED */
		*reg_ptr++ = 0x00;
	}

	__asm__ __volatile__ ("eieio");
	__asm__ __volatile__ ("sync");

	/* Setup PB_OCN_BAR2: size 256B + ENable @ 0x0_80000000 */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PB_REG_OFFSET + PB_OCN_BAR2,
		0x80000001);
	__asm__ __volatile__ ("sync");

	/* Make sure that OCN_BAR2 decoder is set (to allow following immediate
	 * read from SDRAM)
	 */

	temp = in32(CONFIG_SYS_TSI108_CSR_BASE + TSI108_PB_REG_OFFSET + PB_OCN_BAR2);
	__asm__ __volatile__ ("sync");

	/*
	 * Remap PB_OCN_BAR1 to accomodate PCI-bus aperture and EPROM into the
	 * processor bus address space. Immediately after reset LUT and address
	 * translation are disabled for this BAR. Now we have to initialize LUT
	 * and switch from the BOOT mode to the normal operation mode.
	 *
	 * The aperture defined by PB_OCN_BAR1 startes at address 0xE0000000
	 * and covers 512MB of address space. To allow larger aperture we also
	 * have to relocate register window of Tsi108
	 *
	 * Initialize LUT (32-entries) prior switching PB_OCN_BAR1 from BOOT
	 * mode.
	 *
	 * initialize pointer to LUT associated with PB_OCN_BAR1
	 */
	reg_ptr =
		(ulong *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PB_REG_OFFSET + 0x800);

	for (i = 0; i < 32; i++) {
		*reg_ptr++ = pb2ocn_lut1[i].lower;
		*reg_ptr++ = pb2ocn_lut1[i].upper;
	}

	__asm__ __volatile__ ("sync");

	/* Base addresses for CS0, CS1, CS2, CS3 */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B0_ADDR,
		0x00000000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B1_ADDR,
		0x00100000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B2_ADDR,
		0x00200000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B3_ADDR,
		0x00300000);
	__asm__ __volatile__ ("sync");

	/* Masks for HLP banks */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B0_MASK,
		0xFFF00000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B1_MASK,
		0xFFF00000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B2_MASK,
		0xFFF00000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B3_MASK,
		0xFFF00000);
	__asm__ __volatile__ ("sync");

	/* Set CTRL0 values for banks */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B0_CTRL0,
		0x7FFC44C2);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B1_CTRL0,
		0x7FFC44C0);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B2_CTRL0,
		0x7FFC44C0);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B3_CTRL0,
		0x7FFC44C2);
	__asm__ __volatile__ ("sync");

	/* Set banks to latched mode, enabled, and other default settings */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B0_CTRL1,
		0x7C0F2000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B1_CTRL1,
		0x7C0F2000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B2_CTRL1,
		0x7C0F2000);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_HLP_REG_OFFSET + HLP_B3_CTRL1,
		0x7C0F2000);
	__asm__ __volatile__ ("sync");

	/*
	 * Set new value for PB_OCN_BAR1: switch from BOOT to LUT mode.
	 * value for PB_OCN_BAR1: (BA-0xE000_0000 + size 512MB + ENable)
	 */
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PB_REG_OFFSET + PB_OCN_BAR1,
		0xE0000011);
	__asm__ __volatile__ ("sync");

	/* Make sure that OCN_BAR2 decoder is set (to allow following
	 * immediate read from SDRAM)
	 */

	temp = in32(CONFIG_SYS_TSI108_CSR_BASE + TSI108_PB_REG_OFFSET + PB_OCN_BAR1);
	__asm__ __volatile__ ("sync");

	/*
	 * SRI: At this point we have enabled the HLP banks. That means we can
	 * now read from the NVRAM and initialize the environment variables.
	 * We will over-ride the env_init called in board_init_f
	 * This is really a work-around because, the HLP bank 1
	 * where NVRAM resides is not visible during board_init_f
	 * (arch/powerpc/lib/board.c)
	 * Alternatively, we could use the I2C EEPROM at start-up to configure
	 * and enable all HLP banks and not just HLP 0 as is being done for
	 * Taiga Rev. 2.
	 */

	env_init ();

#ifndef DISABLE_PBM

	/*
	 * For IBM processors we have to set Address-Only commands generated
	 * by PBM that are different from ones set after reset.
	 */

	temp = get_cpu_type ();

	if ((CPU_750FX == temp) || (CPU_750GX == temp))
		out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PB_REG_OFFSET + PB_MCMD,
			0x00009955);
#endif	/* DISABLE_PBM */

#ifdef CONFIG_PCI
	/*
	 * Initialize PCI/X block
	 */

	/* Map PCI/X Configuration Space (16MB @ 0x0_FE000000) */
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET +
		PCI_PFAB_BAR0_UPPER, 0);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_PFAB_BAR0,
		0xFB000001);
	__asm__ __volatile__ ("sync");

	/* Set Bus Number for the attached PCI/X bus (we will use 0 for NB) */

	temp =	in32(CONFIG_SYS_TSI108_CSR_BASE +
		TSI108_PCI_REG_OFFSET + PCI_PCIX_STAT);

	temp &= ~0xFF00;	/* Clear the BUS_NUM field */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_PCIX_STAT,
		temp);

	/* Map PCI/X IO Space (64KB @ 0x0_FD000000) takes one 16MB LUT entry */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_PFAB_IO_UPPER,
		0);
	__asm__ __volatile__ ("sync");

	/* This register is on the PCI side to interpret the address it receives
	 * and maps it as a IO address.
	 */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_PFAB_IO,
		0x00000001);
	__asm__ __volatile__ ("sync");

	/*
	 * Map PCI/X Memory Space
	 *
	 * Transactions directed from OCM to PCI Memory Space are directed
	 * from PB to PCI
	 * unchanged (as defined by PB_OCN_BAR1,2 and LUT settings).
	 * If address remapping is required the corresponding PCI_PFAB_MEM32
	 * and PCI_PFAB_PFMx register groups have to be configured.
	 *
	 * Map the path from the PCI/X bus into the system memory
	 *
	 * The memory mapped window assotiated with PCI P2O_BAR2 provides
	 * access to the system memory without address remapping.
	 * All system memory is opened for accesses initiated by PCI/X bus
	 * masters.
	 *
	 * Initialize LUT associated with PCI P2O_BAR2
	 *
	 * set pointer to LUT associated with PCI P2O_BAR2
	 */

	reg_ptr =
		(ulong *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + 0x500);

#ifdef DISABLE_PBM

	/* In case when PBM is disabled (no HW supported cache snoopng on PB)
	 * P2O_BAR2 is directly mapped into the system memory without address
	 * translation.
	 */

	reg_val = 0x00000004;	/* SDRAM port + NO Addr_Translation */

	for (i = 0; i < 32; i++) {
		*reg_ptr++ = reg_val;	/* P2O_BAR2_LUTx */
		*reg_ptr++ = 0;		/* P2O_BAR2_LUT_UPPERx */
	}

	/* value for PCI BAR2 (size = 512MB, Enabled, No Addr. Translation) */
	reg_val = 0x00007500;
#else

	reg_val = 0x00000002;	/* Destination port = PBM */

	for (i = 0; i < 32; i++) {
		*reg_ptr++ = reg_val;	/* P2O_BAR2_LUTx */
/* P2O_BAR2_LUT_UPPERx : Set data swapping mode for PBM (byte swapping) */
		*reg_ptr++ = 0x40000000;
/* offset = 16MB, address translation is enabled to allow byte swapping */
		reg_val += 0x01000000;
	}

/* value for PCI BAR2 (size = 512MB, Enabled, Address Translation Enabled) */
	reg_val = 0x00007100;
#endif

	__asm__ __volatile__ ("eieio");
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_PAGE_SIZES,
		reg_val);
	__asm__ __volatile__ ("sync");

	/* Set 64-bit PCI bus address for system memory
	 * ( 0 is the best choice for easy mapping)
	 */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_BAR2,
		0x00000000);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_BAR2_UPPER,
		0x00000000);
	__asm__ __volatile__ ("sync");

#ifndef DISABLE_PBM
	/*
	 *  The memory mapped window assotiated with PCI P2O_BAR3 provides
	 *  access to the system memory using SDRAM OCN port and address
	 *  translation. This is alternative way to access SDRAM from PCI
	 *  required for Tsi108 emulation testing.
	 *  All system memory is opened for accesses initiated by
	 *  PCI/X bus masters.
	 *
	 *  Initialize LUT associated with PCI P2O_BAR3
	 *
	 *  set pointer to LUT associated with PCI P2O_BAR3
	 */
	reg_ptr =
		(ulong *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + 0x600);

	reg_val = 0x00000004;	/* Destination port = SDC */

	for (i = 0; i < 32; i++) {
		*reg_ptr++ = reg_val;	/* P2O_BAR3_LUTx */

/* P2O_BAR3_LUT_UPPERx : Set data swapping mode for PBM (byte swapping) */
		*reg_ptr++ = 0;

/* offset = 16MB, address translation is enabled to allow byte swapping */
		reg_val += 0x01000000;
	}

	__asm__ __volatile__ ("eieio");
	__asm__ __volatile__ ("sync");

	/* Configure PCI P2O_BAR3 (size = 512MB, Enabled) */

	reg_val =
		in32(CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET +
		 PCI_P2O_PAGE_SIZES);
	reg_val &= ~0x00FF;
	reg_val |= 0x0071;
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_PAGE_SIZES,
		reg_val);
	__asm__ __volatile__ ("sync");

	/* Set 64-bit base PCI bus address for window (0x20000000) */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_BAR3_UPPER,
		0x00000000);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_BAR3,
		0x20000000);
	__asm__ __volatile__ ("sync");

#endif	/* !DISABLE_PBM */

#ifdef ENABLE_PCI_CSR_BAR
	/* open if required access to Tsi108 CSRs from the PCI/X bus */
	/* enable BAR0 on the PCI/X bus */
	reg_val = in32(CONFIG_SYS_TSI108_CSR_BASE +
		TSI108_PCI_REG_OFFSET + PCI_MISC_CSR);
	reg_val |= 0x02;
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_MISC_CSR,
		reg_val);
	__asm__ __volatile__ ("sync");

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_BAR0_UPPER,
		0x00000000);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_P2O_BAR0,
		CONFIG_SYS_TSI108_CSR_BASE);
	__asm__ __volatile__ ("sync");

#endif

	/*
	 * Finally enable PCI/X Bus Master and Memory Space access
	 */

	reg_val = in32(CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_CSR);
	reg_val |= 0x06;
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_PCI_REG_OFFSET + PCI_CSR, reg_val);
	__asm__ __volatile__ ("sync");

#endif	/* CONFIG_PCI */

	/*
	 * Initialize MPIC outputs (interrupt pins):
	 * Interrupt routing on the Grendel Emul. Board:
	 * PB_INT[0] -> INT (CPU0)
	 * PB_INT[1] -> INT (CPU1)
	 * PB_INT[2] -> MCP (CPU0)
	 * PB_INT[3] -> MCP (CPU1)
	 * Set interrupt controller outputs as Level_Sensitive/Active_Low
	 */
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_MPIC_REG_OFFSET + MPIC_CSR(0), 0x02);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_MPIC_REG_OFFSET + MPIC_CSR(1), 0x02);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_MPIC_REG_OFFSET + MPIC_CSR(2), 0x02);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_MPIC_REG_OFFSET + MPIC_CSR(3), 0x02);
	__asm__ __volatile__ ("sync");

	/*
	 * Ensure that Machine Check exception is enabled
	 * We need it to support PCI Bus probing (configuration reads)
	 */

	reg_val = mfmsr ();
	mtmsr(reg_val | MSR_ME);

	return 0;
}

/*
 * Needed to print out L2 cache info
 * used in the misc_init_r function
 */

unsigned long get_l2cr (void)
{
	unsigned long l2controlreg;
	asm volatile ("mfspr %0, 1017":"=r" (l2controlreg):);
	return l2controlreg;
}

/*
 * misc_init_r()
 *
 * various things to do after relocation
 *
 */

int misc_init_r (void)
{
#ifdef CONFIG_SYS_CLK_SPREAD	/* Initialize Spread-Spectrum Clock generation */
	ulong i;

	/* Ensure that Spread-Spectrum is disabled */
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET + CG_PLL0_CTRL0, 0);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET + CG_PLL1_CTRL0, 0);

	/* Initialize PLL1: CG_PCI_CLK , internal OCN_CLK
	 * Uses pre-calculated value for Fout = 800 MHz, Fs = 30 kHz, D = 0.5%
	 */

	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET + CG_PLL1_CTRL0,
		0x002e0044);	/* D = 0.25% */
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET + CG_PLL1_CTRL1,
		0x00000039);	/* BWADJ */

	/* Initialize PLL0: CG_PB_CLKO  */
	/* Detect PB clock freq. */
	i = in32(CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET + CG_PWRUP_STATUS);
	i = (i >> 16) & 0x07;	/* Get PB PLL multiplier */

	out32 (CONFIG_SYS_TSI108_CSR_BASE +
		TSI108_CLK_REG_OFFSET + CG_PLL0_CTRL0, pll0_config[i].ctrl0);
	out32 (CONFIG_SYS_TSI108_CSR_BASE +
		TSI108_CLK_REG_OFFSET + CG_PLL0_CTRL1, pll0_config[i].ctrl1);

	/* Wait and set SSEN for both PLL0 and 1 */
	udelay (1000);
	out32 (CONFIG_SYS_TSI108_CSR_BASE + TSI108_CLK_REG_OFFSET + CG_PLL1_CTRL0,
		0x802e0044);	/* D=0.25% */
	out32 (CONFIG_SYS_TSI108_CSR_BASE +
		TSI108_CLK_REG_OFFSET + CG_PLL0_CTRL0,
		0x80000000 | pll0_config[i].ctrl0);
#endif	/* CONFIG_SYS_CLK_SPREAD */

#ifdef CONFIG_SYS_L2
	l2cache_enable ();
#endif
	printf ("BUS:   %lu MHz\n", gd->bus_clk / 1000000);
	printf ("MEM:   %lu MHz\n", gd->mem_clk / 1000000);

	/*
	 * All the information needed to print the cache details is avaiblable
	 * at this point i.e. above call to l2cache_enable is the very last
	 * thing done with regards to enabling diabling the cache.
	 * So this seems like a good place to print all this information
	 */

	printf ("CACHE: ");
	switch (get_cpu_type()) {
	case CPU_7447A:
		printf ("L1 Instruction cache - 32KB 8-way");
		(get_hid0 () & (1 << 15)) ? printf (" ENABLED\n") :
			printf (" DISABLED\n");
		printf ("L1 Data cache - 32KB 8-way");
		(get_hid0 () & (1 << 14)) ? printf (" ENABLED\n") :
			printf (" DISABLED\n");
		printf ("Unified L2 cache - 512KB 8-way");
		(get_l2cr () & (1 << 31)) ? printf (" ENABLED\n") :
			printf (" DISABLED\n");
		printf ("\n");
		break;

	case CPU_7448:
		printf ("L1 Instruction cache - 32KB 8-way");
		(get_hid0 () & (1 << 15)) ? printf (" ENABLED\n") :
			printf (" DISABLED\n");
		printf ("L1 Data cache - 32KB 8-way");
		(get_hid0 () & (1 << 14)) ? printf (" ENABLED\n") :
			printf (" DISABLED\n");
		printf ("Unified L2 cache - 1MB 8-way");
		(get_l2cr () & (1 << 31)) ? printf (" ENABLED\n") :
			printf (" DISABLED\n");
		break;
	default:
		break;
	}
	return 0;
}
