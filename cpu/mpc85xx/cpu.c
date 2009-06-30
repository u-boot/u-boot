/*
 * Copyright 2004,2007,2008 Freescale Semiconductor, Inc.
 * (C) Copyright 2002, 2003 Motorola Inc.
 * Xianghua Xiao (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
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

#include <config.h>
#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <tsec.h>
#include <netdev.h>
#include <fsl_esdhc.h>
#include <asm/cache.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct cpu_type cpu_type_list [] = {
	CPU_TYPE_ENTRY(8533, 8533),
	CPU_TYPE_ENTRY(8533, 8533_E),
	CPU_TYPE_ENTRY(8535, 8535),
	CPU_TYPE_ENTRY(8535, 8535_E),
	CPU_TYPE_ENTRY(8536, 8536),
	CPU_TYPE_ENTRY(8536, 8536_E),
	CPU_TYPE_ENTRY(8540, 8540),
	CPU_TYPE_ENTRY(8541, 8541),
	CPU_TYPE_ENTRY(8541, 8541_E),
	CPU_TYPE_ENTRY(8543, 8543),
	CPU_TYPE_ENTRY(8543, 8543_E),
	CPU_TYPE_ENTRY(8544, 8544),
	CPU_TYPE_ENTRY(8544, 8544_E),
	CPU_TYPE_ENTRY(8545, 8545),
	CPU_TYPE_ENTRY(8545, 8545_E),
	CPU_TYPE_ENTRY(8547, 8547_E),
	CPU_TYPE_ENTRY(8548, 8548),
	CPU_TYPE_ENTRY(8548, 8548_E),
	CPU_TYPE_ENTRY(8555, 8555),
	CPU_TYPE_ENTRY(8555, 8555_E),
	CPU_TYPE_ENTRY(8560, 8560),
	CPU_TYPE_ENTRY(8567, 8567),
	CPU_TYPE_ENTRY(8567, 8567_E),
	CPU_TYPE_ENTRY(8568, 8568),
	CPU_TYPE_ENTRY(8568, 8568_E),
	CPU_TYPE_ENTRY(8569, 8569),
	CPU_TYPE_ENTRY(8569, 8569_E),
	CPU_TYPE_ENTRY(8572, 8572),
	CPU_TYPE_ENTRY(8572, 8572_E),
	CPU_TYPE_ENTRY(P2020, P2020),
	CPU_TYPE_ENTRY(P2020, P2020_E),
};

struct cpu_type *identify_cpu(u32 ver)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++)
		if (cpu_type_list[i].soc_ver == ver)
			return &cpu_type_list[i];

	return NULL;
}

int checkcpu (void)
{
	sys_info_t sysinfo;
	uint pvr, svr;
	uint fam;
	uint ver;
	uint major, minor;
	struct cpu_type *cpu;
	char buf1[32], buf2[32];
#ifdef CONFIG_DDR_CLK_FREQ
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 ddr_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO)
		>> MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
#else
	u32 ddr_ratio = 0;
#endif
	int i;

	svr = get_svr();
	ver = SVR_SOC_VER(svr);
	major = SVR_MAJ(svr);
#ifdef CONFIG_MPC8536
	major &= 0x7; /* the msb of this nibble is a mfg code */
#endif
	minor = SVR_MIN(svr);

#if (CONFIG_NUM_CPUS > 1)
	volatile ccsr_pic_t *pic = (void *)(CONFIG_SYS_MPC85xx_PIC_ADDR);
	printf("CPU%d:  ", pic->whoami);
#else
	puts("CPU:   ");
#endif

	cpu = identify_cpu(ver);
	if (cpu) {
		puts(cpu->name);

		if (IS_E_PROCESSOR(svr))
			puts("E");
	} else {
		puts("Unknown");
	}

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, svr);

	pvr = get_pvr();
	fam = PVR_FAM(pvr);
	ver = PVR_VER(pvr);
	major = PVR_MAJ(pvr);
	minor = PVR_MIN(pvr);

	printf("Core:  ");
	switch (fam) {
	case PVR_FAM(PVR_85xx):
	    puts("E500");
	    break;
	default:
	    puts("Unknown");
	    break;
	}

	if (PVR_MEM(pvr) == 0x03)
		puts("MC");

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, pvr);

	get_sys_info(&sysinfo);

	puts("Clock Configuration:");
	for (i = 0; i < CONFIG_NUM_CPUS; i++) {
		if (!(i & 3))
			printf ("\n       ");
		printf("CPU%d:%-4s MHz, ",
				i,strmhz(buf1, sysinfo.freqProcessor[i]));
	}
	printf("\n       CCB:%-4s MHz,\n", strmhz(buf1, sysinfo.freqSystemBus));

	switch (ddr_ratio) {
	case 0x0:
		printf("       DDR:%-4s MHz (%s MT/s data rate), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
		break;
	case 0x7:
		printf("       DDR:%-4s MHz (%s MT/s data rate) (Synchronous), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
		break;
	default:
		printf("       DDR:%-4s MHz (%s MT/s data rate) (Asynchronous), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
		break;
	}

	if (sysinfo.freqLocalBus > LCRR_CLKDIV)
		printf("LBC:%-4s MHz\n", strmhz(buf1, sysinfo.freqLocalBus));
	else
		printf("LBC: unknown (LCRR[CLKDIV] = 0x%02lx)\n",
		       sysinfo.freqLocalBus);

#ifdef CONFIG_CPM2
	printf("CPM:   %s MHz\n", strmhz(buf1, sysinfo.freqSystemBus));
#endif

#ifdef CONFIG_QE
	printf("       QE:%-4s MHz\n", strmhz(buf1, sysinfo.freqQE));
#endif

	puts("L1:    D-cache 32 kB enabled\n       I-cache 32 kB enabled\n");

	return 0;
}


/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	uint pvr;
	uint ver;
	unsigned long val, msr;

	pvr = get_pvr();
	ver = PVR_VER(pvr);

	if (ver & 1){
	/* e500 v2 core has reset control register */
		volatile unsigned int * rstcr;
		rstcr = (volatile unsigned int *)(CONFIG_SYS_IMMR + 0xE00B0);
		*rstcr = 0x2;		/* HRESET_REQ */
		udelay(100);
	}

	/*
	 * Fallthrough if the code above failed
	 * Initiate hard reset in debug control register DBCR0
	 * Make sure MSR[DE] = 1
	 */

	msr = mfmsr ();
	msr |= MSR_DE;
	mtmsr (msr);

	val = mfspr(DBCR0);
	val |= 0x70000000;
	mtspr(DBCR0,val);

	return 1;
}


/*
 * Get timebase clock frequency
 */
unsigned long get_tbclk (void)
{
	return (gd->bus_clk + 4UL)/8UL;
}


#if defined(CONFIG_WATCHDOG)
void
watchdog_reset(void)
{
	int re_enable = disable_interrupts();
	reset_85xx_watchdog();
	if (re_enable) enable_interrupts();
}

void
reset_85xx_watchdog(void)
{
	/*
	 * Clear TSR(WIS) bit by writing 1
	 */
	unsigned long val;
	val = mfspr(SPRN_TSR);
	val |= TSR_WIS;
	mtspr(SPRN_TSR, val);
}
#endif	/* CONFIG_WATCHDOG */

/*
 * Configures a UPM. The function requires the respective MxMR to be set
 * before calling this function. "size" is the number or entries, not a sizeof.
 */
void upmconfig (uint upm, uint * table, uint size)
{
	int i, mdr, mad, old_mad = 0;
	volatile u32 *mxmr;
	volatile ccsr_lbc_t *lbc = (void *)(CONFIG_SYS_MPC85xx_LBC_ADDR);
	volatile u32 *brp,*orp;
	volatile u8* dummy = NULL;
	int upmmask;

	switch (upm) {
	case UPMA:
		mxmr = &lbc->mamr;
		upmmask = BR_MS_UPMA;
		break;
	case UPMB:
		mxmr = &lbc->mbmr;
		upmmask = BR_MS_UPMB;
		break;
	case UPMC:
		mxmr = &lbc->mcmr;
		upmmask = BR_MS_UPMC;
		break;
	default:
		printf("%s: Bad UPM index %d to configure\n", __FUNCTION__, upm);
		hang();
	}

	/* Find the address for the dummy write transaction */
	for (brp = &lbc->br0, orp = &lbc->or0, i = 0; i < 8;
		 i++, brp += 2, orp += 2) {

		/* Look for a valid BR with selected UPM */
		if ((in_be32(brp) & (BR_V | BR_MSEL)) == (BR_V | upmmask)) {
			dummy = (volatile u8*)(in_be32(brp) & BR_BA);
			break;
		}
	}

	if (i == 8) {
		printf("Error: %s() could not find matching BR\n", __FUNCTION__);
		hang();
	}

	for (i = 0; i < size; i++) {
		/* 1 */
		out_be32(mxmr,  (in_be32(mxmr) & 0x4fffffc0) | MxMR_OP_WARR | i);
		/* 2 */
		out_be32(&lbc->mdr, table[i]);
		/* 3 */
		mdr = in_be32(&lbc->mdr);
		/* 4 */
		*(volatile u8 *)dummy = 0;
		/* 5 */
		do {
			mad = in_be32(mxmr) & MxMR_MAD_MSK;
		} while (mad <= old_mad && !(!mad && i == (size-1)));
		old_mad = mad;
	}
	out_be32(mxmr, (in_be32(mxmr) & 0x4fffffc0) | MxMR_OP_NORM);
}


/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_ETHER_ON_FCC)
	fec_initialize(bis);
#endif

#if defined(CONFIG_UEC_ETH)
	uec_standard_init(bis);
#endif

#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_MPC85XX_FEC)
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
