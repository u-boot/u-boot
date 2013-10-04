/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/system.h>
#include <asm/openrisc_exc.h>

static volatile int illegal_instruction;

static void illegal_instruction_handler(void)
{
	ulong *epcr = (ulong *)mfspr(SPR_EPCR_BASE);

	/* skip over the illegal instruction */
	mtspr(SPR_EPCR_BASE, (ulong)(++epcr));
	illegal_instruction = 1;
}

static void checkinstructions(void)
{
	ulong ra = 1, rb = 1, rc;

	exception_install_handler(EXC_ILLEGAL_INSTR,
				illegal_instruction_handler);

	illegal_instruction = 0;
	asm volatile("l.mul %0,%1,%2" : "=r" (rc) : "r" (ra), "r" (rb));
	printf("           Hardware multiplier: %s\n",
		illegal_instruction ? "no" : "yes");

	illegal_instruction = 0;
	asm volatile("l.div %0,%1,%2" : "=r" (rc) : "r" (ra), "r" (rb));
	printf("           Hardware divider: %s\n",
		illegal_instruction ? "no" : "yes");

	exception_free_handler(EXC_ILLEGAL_INSTR);
}

int checkcpu(void)
{
	ulong upr = mfspr(SPR_UPR);
	ulong vr = mfspr(SPR_VR);
	ulong iccfgr = mfspr(SPR_ICCFGR);
	ulong dccfgr = mfspr(SPR_DCCFGR);
	ulong immucfgr = mfspr(SPR_IMMUCFGR);
	ulong dmmucfgr = mfspr(SPR_DMMUCFGR);
	ulong cpucfgr = mfspr(SPR_CPUCFGR);
	uint ver = (vr & SPR_VR_VER) >> 24;
	uint rev = vr & SPR_VR_REV;
	uint block_size;
	uint ways;
	uint sets;

	printf("CPU:   OpenRISC-%x00 (rev %d) @ %d MHz\n",
		ver, rev, (CONFIG_SYS_CLK_FREQ / 1000000));

	if (upr & SPR_UPR_DCP) {
		block_size = (dccfgr & SPR_DCCFGR_CBS) ? 32 : 16;
		ways = 1 << (dccfgr & SPR_DCCFGR_NCW);
		printf("       D-Cache: %d bytes, %d bytes/line, %d way(s)\n",
		       checkdcache(), block_size, ways);
	} else {
		printf("       D-Cache: no\n");
	}

	if (upr & SPR_UPR_ICP) {
		block_size = (iccfgr & SPR_ICCFGR_CBS) ? 32 : 16;
		ways = 1 << (iccfgr & SPR_ICCFGR_NCW);
		printf("       I-Cache: %d bytes, %d bytes/line, %d way(s)\n",
		       checkicache(), block_size, ways);
	} else {
		printf("       I-Cache: no\n");
	}

	if (upr & SPR_UPR_DMP) {
		sets = 1 << ((dmmucfgr & SPR_DMMUCFGR_NTS) >> 2);
		ways = (dmmucfgr & SPR_DMMUCFGR_NTW) + 1;
		printf("       DMMU: %d sets, %d way(s)\n",
		       sets, ways);
	} else {
		printf("       DMMU: no\n");
	}

	if (upr & SPR_UPR_IMP) {
		sets = 1 << ((immucfgr & SPR_IMMUCFGR_NTS) >> 2);
		ways = (immucfgr & SPR_IMMUCFGR_NTW) + 1;
		printf("       IMMU: %d sets, %d way(s)\n",
		       sets, ways);
	} else {
		printf("       IMMU: no\n");
	}

	printf("       MAC unit: %s\n",
		(upr & SPR_UPR_MP) ? "yes" : "no");
	printf("       Debug unit: %s\n",
		(upr & SPR_UPR_DUP) ? "yes" : "no");
	printf("       Performance counters: %s\n",
		(upr & SPR_UPR_PCUP) ? "yes" : "no");
	printf("       Power management: %s\n",
		(upr & SPR_UPR_PMP) ? "yes" : "no");
	printf("       Interrupt controller: %s\n",
		(upr & SPR_UPR_PICP) ? "yes" : "no");
	printf("       Timer: %s\n",
		(upr & SPR_UPR_TTP) ? "yes" : "no");
	printf("       Custom unit(s): %s\n",
		(upr & SPR_UPR_CUP) ? "yes" : "no");

	printf("       Supported instructions:\n");
	printf("           ORBIS32: %s\n",
		(cpucfgr & SPR_CPUCFGR_OB32S) ? "yes" : "no");
	printf("           ORBIS64: %s\n",
		(cpucfgr & SPR_CPUCFGR_OB64S) ? "yes" : "no");
	printf("           ORFPX32: %s\n",
		(cpucfgr & SPR_CPUCFGR_OF32S) ? "yes" : "no");
	printf("           ORFPX64: %s\n",
		(cpucfgr & SPR_CPUCFGR_OF64S) ? "yes" : "no");

	checkinstructions();

	return 0;
}

int cleanup_before_linux(void)
{
	disable_interrupts();
	return 0;
}

extern void __reset(void);

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	/* Code the jump to __reset here as the compiler is prone to
	   emitting a bad jump instruction if the function is in flash */
	__asm__("l.movhi r1,hi(__reset);  \
		 l.ori r1,r1,lo(__reset); \
		 l.jr r1");
	/* not reached, __reset does not return */
	return 0;
}
