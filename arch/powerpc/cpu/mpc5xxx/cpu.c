/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#include <fdt_support.h>
#endif

#if defined(CONFIG_OF_IDE_FIXUP)
#include <ide.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	ulong clock = gd->cpu_clk;
	char buf[32];
	uint svr, pvr;

	puts ("CPU:   ");

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
	printf (" at %s MHz\n", strmhz (buf, clock));
	return 0;
}

/* ------------------------------------------------------------------------- */

int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
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

#ifdef CONFIG_OF_BOARD_SETUP
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
#ifdef CONFIG_OF_IDE_FIXUP
	if (!ide_device_present(0)) {
		/* NO CF card detected -> delete ata node in DTS */
		int nodeoffset = 0;
		char nodename[] = "/soc5200@f0000000/ata@3a00";

		nodeoffset = fdt_path_offset(blob, nodename);
		if (nodeoffset >= 0) {
			fdt_del_node(blob, nodeoffset);
		} else {
			printf("%s: cannot find %s node err:%s\n",
				__func__, nodename, fdt_strerror(nodeoffset));
		}
	}

#endif /* CONFIG_OF_IDE_FIXUP */
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}
#endif /* CONFIG_OF_BOARD_SETUP */

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

#if defined(CONFIG_WATCHDOG)
void watchdog_reset(void)
{
	int re_enable = disable_interrupts();
	reset_5xxx_watchdog();
	if (re_enable) enable_interrupts();
}

void reset_5xxx_watchdog(void)
{
	volatile struct mpc5xxx_gpt *gpt0 =
		(struct mpc5xxx_gpt *) MPC5XXX_GPT;

	/* Trigger TIMER_0 by writing A5 to OCPW */
	clrsetbits_be32(&gpt0->emsr, 0xff000000, 0xa5000000);
}
#endif	/* CONFIG_WATCHDOG */
