/*
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
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

/*
 * CPU specific code for the MPC83xx family.
 *
 * Derived from the MPC8260 and MPC85xx.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc83xx.h>
#include <asm/processor.h>
#include <libfdt.h>
#include <tsec.h>
#include <netdev.h>
#include <fsl_esdhc.h>
#ifdef CONFIG_BOOTCOUNT_LIMIT
#include <asm/immap_qe.h>
#include <asm/io.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int checkcpu(void)
{
	volatile immap_t *immr;
	ulong clock = gd->cpu_clk;
	u32 pvr = get_pvr();
	u32 spridr;
	char buf[32];
	int i;

	const struct cpu_type {
		char name[15];
		u32 partid;
	} cpu_type_list [] = {
		CPU_TYPE_ENTRY(8311),
		CPU_TYPE_ENTRY(8313),
		CPU_TYPE_ENTRY(8314),
		CPU_TYPE_ENTRY(8315),
		CPU_TYPE_ENTRY(8321),
		CPU_TYPE_ENTRY(8323),
		CPU_TYPE_ENTRY(8343),
		CPU_TYPE_ENTRY(8347_TBGA_),
		CPU_TYPE_ENTRY(8347_PBGA_),
		CPU_TYPE_ENTRY(8349),
		CPU_TYPE_ENTRY(8358_TBGA_),
		CPU_TYPE_ENTRY(8358_PBGA_),
		CPU_TYPE_ENTRY(8360),
		CPU_TYPE_ENTRY(8377),
		CPU_TYPE_ENTRY(8378),
		CPU_TYPE_ENTRY(8379),
	};

	immr = (immap_t *)CONFIG_SYS_IMMR;

	puts("CPU:   ");

	switch (pvr & 0xffff0000) {
		case PVR_E300C1:
			printf("e300c1, ");
			break;

		case PVR_E300C2:
			printf("e300c2, ");
			break;

		case PVR_E300C3:
			printf("e300c3, ");
			break;

		case PVR_E300C4:
			printf("e300c4, ");
			break;

		default:
			printf("Unknown core, ");
	}

	spridr = immr->sysconf.spridr;

	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++)
		if (cpu_type_list[i].partid == PARTID_NO_E(spridr)) {
			puts("MPC");
			puts(cpu_type_list[i].name);
			if (IS_E_PROCESSOR(spridr))
				puts("E");
			if (REVID_MAJOR(spridr) >= 2)
				puts("A");
			printf(", Rev: %d.%d", REVID_MAJOR(spridr),
			       REVID_MINOR(spridr));
			break;
		}

	if (i == ARRAY_SIZE(cpu_type_list))
		printf("(SPRIDR %08x unknown), ", spridr);

	printf(" at %s MHz, ", strmhz(buf, clock));

	printf("CSB: %s MHz\n", strmhz(buf, gd->csb_clk));

	return 0;
}


/*
 * Program a UPM with the code supplied in the table.
 *
 * The 'dummy' variable is used to increment the MAD. 'dummy' is
 * supposed to be a pointer to the memory of the device being
 * programmed by the UPM.  The data in the MDR is written into
 * memory and the MAD is incremented every time there's a write
 * to 'dummy'. Unfortunately, the current prototype for this
 * function doesn't allow for passing the address of this
 * device, and changing the prototype will break a number lots
 * of other code, so we need to use a round-about way of finding
 * the value for 'dummy'.
 *
 * The value can be extracted from the base address bits of the
 * Base Register (BR) associated with the specific UPM.  To find
 * that BR, we need to scan all 8 BRs until we find the one that
 * has its MSEL bits matching the UPM we want.  Once we know the
 * right BR, we can extract the base address bits from it.
 *
 * The MxMR and the BR and OR of the chosen bank should all be
 * configured before calling this function.
 *
 * Parameters:
 * upm: 0=UPMA, 1=UPMB, 2=UPMC
 * table: Pointer to an array of values to program
 * size: Number of elements in the array.  Must be 64 or less.
 */
void upmconfig (uint upm, uint *table, uint size)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile fsl_lbus_t *lbus = &immap->lbus;
	volatile uchar *dummy = NULL;
	const u32 msel = (upm + 4) << BR_MSEL_SHIFT;	/* What the MSEL field in BRn should be */
	volatile u32 *mxmr = &lbus->mamr + upm;	/* Pointer to mamr, mbmr, or mcmr */
	uint i;

	/* Scan all the banks to determine the base address of the device */
	for (i = 0; i < 8; i++) {
		if ((lbus->bank[i].br & BR_MSEL) == msel) {
			dummy = (uchar *) (lbus->bank[i].br & BR_BA);
			break;
		}
	}

	if (!dummy) {
		printf("Error: %s() could not find matching BR\n", __FUNCTION__);
		hang();
	}

	/* Set the OP field in the MxMR to "write" and the MAD field to 000000 */
	*mxmr = (*mxmr & 0xCFFFFFC0) | 0x10000000;

	for (i = 0; i < size; i++) {
		lbus->mdr = table[i];
		__asm__ __volatile__ ("sync");
		*dummy = 0;	/* Write the value to memory and increment MAD */
		__asm__ __volatile__ ("sync");
		while(((*mxmr & 0x3f) != ((i + 1) & 0x3f)));
	}

	/* Set the OP field in the MxMR to "normal" and the MAD field to 000000 */
	*mxmr &= 0xCFFFFFC0;
}


int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
#ifndef MPC83xx_RESET
	ulong addr;
#endif

	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	puts("Resetting the board.\n");

#ifdef MPC83xx_RESET

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~( MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/* enable Reset Control Reg */
	immap->reset.rpr = 0x52535445;
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* confirm Reset Control Reg is enabled */
	while(!((immap->reset.rcer) & RCER_CRE));

	udelay(200);

	/* perform reset, only one bit */
	immap->reset.rcr = RCR_SWHR;

#else	/* ! MPC83xx_RESET */

	immap->reset.rmr = RMR_CSRE;    /* Checkstop Reset enable */

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
	addr = CONFIG_SYS_RESET_ADDRESS;

	((void (*)(void)) addr) ();
#endif	/* MPC83xx_RESET */

	return 1;
}


/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */

unsigned long get_tbclk(void)
{
	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return tbclk;
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	int re_enable = disable_interrupts();

	/* Reset the 83xx watchdog */
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	immr->wdt.swsrr = 0x556c;
	immr->wdt.swsrr = 0xaa39;

	if (re_enable)
		enable_interrupts ();
}
#endif

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_UEC_ETH)
	uec_standard_init(bis);
#endif

#if defined(CONFIG_TSEC_ENET)
	tsec_standard_init(bis);
#endif
	return 0;
}

/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
#ifdef CONFIG_FSL_ESDHC
	return fsl_esdhc_mmc_init(bis);
#else
	return 0;
#endif
}

#ifdef CONFIG_BOOTCOUNT_LIMIT

#if !defined(CONFIG_MPC8360)
#error "CONFIG_BOOTCOUNT_LIMIT only for MPC8360 implemented"
#endif

#if !defined(CONFIG_BOOTCOUNT_ADDR)
#define CONFIG_BOOTCOUNT_ADDR	(0x110000 + QE_MURAM_SIZE - 2 * sizeof(unsigned long))
#endif

#include <asm/io.h>

void bootcount_store (ulong a)
{
	void *reg = (void *)(CONFIG_SYS_IMMR + CONFIG_BOOTCOUNT_ADDR);
	out_be32 (reg, a);
	out_be32 (reg + 4, BOOTCOUNT_MAGIC);
}

ulong bootcount_load (void)
{
	void *reg = (void *)(CONFIG_SYS_IMMR + CONFIG_BOOTCOUNT_ADDR);

	if (in_be32 (reg + 4) != BOOTCOUNT_MAGIC)
		return 0;
	else
		return in_be32 (reg);
}
#endif /* CONFIG_BOOTCOUNT_LIMIT */
