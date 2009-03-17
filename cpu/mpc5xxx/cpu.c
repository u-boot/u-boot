/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * CPU specific code for the MPC5xxx CPUs
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <net.h>
#include <mpc5xxx.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/processor.h>

#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt_support.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	ulong clock = gd->cpu_clk;
	char buf[32];
#ifndef CONFIG_MGT5100
	uint svr, pvr;
#endif

	puts ("CPU:   ");

#ifdef CONFIG_MGT5100
	puts   (CPU_ID_STR);
	printf (" (JTAG ID %08lx)", *(vu_long *)MPC5XXX_CDM_JTAGID);
#else
	svr = get_svr();
	pvr = get_pvr();

	switch (pvr) {
	case PVR_5200:
		printf("MPC5200");
		break;
	case PVR_5200B:
		printf("MPC5200B");
		break;
	default:
		printf("Unknown MPC5xxx");
		break;
	}

	printf (" v%d.%d, Core v%d.%d", SVR_MJREV (svr), SVR_MNREV (svr),
		PVR_MAJ(pvr), PVR_MIN(pvr));
#endif
	printf (" at %s MHz\n", strmhz (buf, clock));
	return 0;
}

/* ------------------------------------------------------------------------- */

int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/* Charge the watchdog timer */
	*(vu_long *)(MPC5XXX_GPT0_COUNTER) = 0x0001000f;
	*(vu_long *)(MPC5XXX_GPT0_ENABLE) = 0x9004; /* wden|ce|timer_ms */
	while(1);

	return 1;

}

/* ------------------------------------------------------------------------- */

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 *
 */
unsigned long get_tbclk (void)
{
	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return (tbclk);
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_OF_LIBFDT) && defined (CONFIG_OF_BOARD_SETUP)
void ft_cpu_setup(void *blob, bd_t *bd)
{
	int div = in_8((void*)CONFIG_SYS_MBAR + 0x204) & 0x0020 ? 8 : 4;
	char * cpu_path = "/cpus/" OF_CPU;
#ifdef CONFIG_MPC5xxx_FEC
	uchar enetaddr[6];
	char * eth_path = "/" OF_SOC "/ethernet@3000";
#endif

	do_fixup_by_path_u32(blob, cpu_path, "timebase-frequency", OF_TBCLK, 1);
	do_fixup_by_path_u32(blob, cpu_path, "bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_path_u32(blob, cpu_path, "clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_path_u32(blob, "/" OF_SOC, "bus-frequency", bd->bi_ipbfreq, 1);
	do_fixup_by_path_u32(blob, "/" OF_SOC, "system-frequency",
				bd->bi_busfreq*div, 1);
#ifdef CONFIG_MPC5xxx_FEC
	eth_getenv_enetaddr("ethaddr", enetaddr);
	do_fixup_by_path(blob, eth_path, "mac-address", enetaddr, 6, 0);
	do_fixup_by_path(blob, eth_path, "local-mac-address", enetaddr, 6, 0);
#endif
}
#endif

#ifdef CONFIG_BOOTCOUNT_LIMIT

void bootcount_store (ulong a)
{
	volatile ulong *save_addr = (volatile ulong *) (MPC5XXX_CDM_BRDCRMB);

	*save_addr = (BOOTCOUNT_MAGIC & 0xffff0000) | a;
}

ulong bootcount_load (void)
{
	volatile ulong *save_addr = (volatile ulong *) (MPC5XXX_CDM_BRDCRMB);

	if ((*save_addr & 0xffff0000) != (BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return (*save_addr & 0x0000ffff);
}
#endif /* CONFIG_BOOTCOUNT_LIMIT */

#ifdef CONFIG_MPC5xxx_FEC
/* Default initializations for FEC controllers.  To override,
 * create a board-specific function called:
 * 	int board_eth_init(bd_t *bis)
 */

int cpu_eth_init(bd_t *bis)
{
	return mpc5xxx_fec_initialize(bis);
}
#endif
