/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <tsec.h>
#include <netdev.h>
#include <fsl_esdhc.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf1[32], buf2[32];
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int svr, major, minor, ver, i;

	svr = in_be32(&gur->svr);
	major = SVR_MAJ(svr);
	minor = SVR_MIN(svr);

	puts("CPU:   Freescale LayerScape ");

	ver = SVR_SOC_VER(svr);
	switch (ver) {
	case SOC_VER_SLS1020:
		puts("SLS1020");
		break;
	case SOC_VER_LS1020:
		puts("LS1020");
		break;
	case SOC_VER_LS1021:
		puts("LS1021");
		break;
	case SOC_VER_LS1022:
		puts("LS1022");
		break;
	default:
		puts("Unknown");
		break;
	}

	if (IS_E_PROCESSOR(svr) && (ver != SOC_VER_SLS1020))
		puts("E");

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, svr);

	puts("Clock Configuration:");

	printf("\n       CPU0(ARMV7):%-4s MHz, ", strmhz(buf1, gd->cpu_clk));
	printf("\n       Bus:%-4s MHz, ", strmhz(buf1, gd->bus_clk));
	printf("DDR:%-4s MHz (%s MT/s data rate), ",
	       strmhz(buf1, gd->mem_clk/2), strmhz(buf2, gd->mem_clk));
	puts("\n");

	/* Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		u32 rcw = in_be32(&gur->rcwsr[i]);

		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

	return 0;
}
#endif

void enable_caches(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
	icache_enable();
#endif
#ifndef CONFIG_SYS_DCACHE_OFF
	dcache_enable();
#endif
}

#ifdef CONFIG_FSL_ESDHC
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int cpu_eth_init(bd_t *bis)
{
#ifdef CONFIG_TSEC_ENET
	tsec_standard_init(bis);
#endif

	return 0;
}
