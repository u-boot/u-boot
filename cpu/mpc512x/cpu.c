/*
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
 * (C) Copyright 2007 DENX Software Engineering
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
 * CPU specific code for the MPC512x family.
 *
 * Derived from the MPC83xx code.
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <asm/processor.h>

#if defined(CONFIG_OF_LIBFDT)
#include <fdt_support.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	ulong clock = gd->cpu_clk;
	u32 pvr = get_pvr ();
	u32 spridr = immr->sysconf.spridr;
	char buf1[32], buf2[32];

	puts ("CPU:   ");

	switch (spridr & 0xffff0000) {
	case SPR_5121E:
		puts ("MPC5121e ");
		break;
	default:
		printf ("Unknown part ID %08x ", spridr & 0xffff0000);
	}
	printf ("rev. %d.%d, Core ", SVR_MJREV (spridr), SVR_MNREV (spridr));

	switch (pvr & 0xffff0000) {
	case PVR_E300C4:
		puts ("e300c4 ");
		break;
	default:
		puts ("unknown ");
	}
	printf ("at %s MHz, CSB at %s MHz\n",
		strmhz(buf1, clock),
		strmhz(buf2, gd->csb_clk) );
	return 0;
}


int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~( MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Enable Reset Control Reg - "RSTE" is the magic word that let us go
	 */
	immap->reset.rpr = 0x52535445;

	/* Verify Reset Control Reg is enabled */
	while (!((immap->reset.rcer) & RCER_CRE))
		;

	printf ("Resetting the board.\n");
	udelay(200);

	/* Perform reset */
	immap->reset.rcr = RCR_SWHR;

	/* Unreached... */
	return 1;
}


/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */
unsigned long get_tbclk (void)
{
	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return tbclk;
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	int re_enable = disable_interrupts ();

	/* Reset watchdog */
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	immr->wdt.swsrr = 0x556c;
	immr->wdt.swsrr = 0xaa39;

	if (re_enable)
		enable_interrupts ();
}
#endif

#ifdef CONFIG_OF_LIBFDT

#ifdef CONFIG_OF_SUPPORT_OLD_DEVICE_TREES
/*
 * fdt setup for old device trees
 * fix up
 * 	cpu clocks
 * 	soc clocks
 * 	ethernet addresses
 */
static void old_ft_cpu_setup(void *blob, bd_t *bd)
{
	/*
	 * avoid fixing up by path because that
	 * produces scary error messages
	 */
	uchar enetaddr[6];

	/*
	 * old device trees have ethernet nodes with
	 * device_type = "network"
	 */
	eth_getenv_enetaddr("ethaddr", enetaddr);
	do_fixup_by_prop(blob, "device_type", "network", 8,
		"local-mac-address", enetaddr, 6, 0);
	do_fixup_by_prop(blob, "device_type", "network", 8,
		"address", enetaddr, 6, 0);
	/*
	 * old device trees have soc nodes with
	 * device_type = "soc"
	 */
	do_fixup_by_prop_u32(blob, "device_type", "soc", 4,
		"bus-frequency", bd->bi_ipsfreq, 0);
}
#endif

static void ft_clock_setup(void *blob, bd_t *bd)
{
	char *cpu_path = "/cpus/" OF_CPU;

	/*
	 * fixup cpu clocks using path
	 */
	do_fixup_by_path_u32(blob, cpu_path,
		"timebase-frequency", OF_TBCLK, 1);
	do_fixup_by_path_u32(blob, cpu_path,
		"bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_path_u32(blob, cpu_path,
		"clock-frequency", bd->bi_intfreq, 1);
	/*
	 * fixup soc clocks using compatible
	 */
	do_fixup_by_compat_u32(blob, OF_SOC_COMPAT,
		"bus-frequency", bd->bi_ipsfreq, 1);
}

void ft_cpu_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_OF_SUPPORT_OLD_DEVICE_TREES
	old_ft_cpu_setup(blob, bd);
#endif
	ft_clock_setup(blob, bd);
#ifdef CONFIG_HAS_ETH0
	fdt_fixup_ethernet(blob);
#endif
}
#endif

#ifdef CONFIG_MPC512x_FEC
/* Default initializations for FEC controllers.  To override,
 * create a board-specific function called:
 * 	int board_eth_init(bd_t *bis)
 */

int cpu_eth_init(bd_t *bis)
{
	return mpc512x_fec_initialize(bis);
}
#endif
