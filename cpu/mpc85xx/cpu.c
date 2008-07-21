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

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct cpu_type cpu_type_list [] = {
	CPU_TYPE_ENTRY(8533, 8533),
	CPU_TYPE_ENTRY(8533, 8533_E),
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
	CPU_TYPE_ENTRY(8572, 8572),
	CPU_TYPE_ENTRY(8572, 8572_E),
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
	uint lcrr;		/* local bus clock ratio register */
	uint clkdiv;		/* clock divider portion of lcrr */
	uint pvr, svr;
	uint fam;
	uint ver;
	uint major, minor;
	struct cpu_type *cpu;
#ifdef CONFIG_DDR_CLK_FREQ
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	u32 ddr_ratio = ((gur->porpllsr) & 0x00003e00) >> 9;
#else
	u32 ddr_ratio = 0;
#endif

	svr = get_svr();
	ver = SVR_SOC_VER(svr);
	major = SVR_MAJ(svr);
	minor = SVR_MIN(svr);

	puts("CPU:   ");

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
	printf(", Version: %d.%d, (0x%08x)\n", major, minor, pvr);

	get_sys_info(&sysinfo);

	puts("Clock Configuration:\n");
	printf("       CPU:%4lu MHz, ", DIV_ROUND_UP(sysinfo.freqProcessor,1000000));
	printf("CCB:%4lu MHz,\n", DIV_ROUND_UP(sysinfo.freqSystemBus,1000000));

	switch (ddr_ratio) {
	case 0x0:
		printf("       DDR:%4lu MHz (%lu MT/s data rate), ",
		DIV_ROUND_UP(sysinfo.freqDDRBus,2000000), DIV_ROUND_UP(sysinfo.freqDDRBus,1000000));
		break;
	case 0x7:
		printf("       DDR:%4lu MHz (%lu MT/s data rate) (Synchronous), ",
		DIV_ROUND_UP(sysinfo.freqDDRBus, 2000000), DIV_ROUND_UP(sysinfo.freqDDRBus, 1000000));
		break;
	default:
		printf("       DDR:%4lu MHz (%lu MT/s data rate) (Asynchronous), ",
		DIV_ROUND_UP(sysinfo.freqDDRBus, 2000000), DIV_ROUND_UP(sysinfo.freqDDRBus,1000000));
		break;
	}

#if defined(CFG_LBC_LCRR)
	lcrr = CFG_LBC_LCRR;
#else
	{
	    volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);

	    lcrr = lbc->lcrr;
	}
#endif
	clkdiv = lcrr & 0x0f;
	if (clkdiv == 2 || clkdiv == 4 || clkdiv == 8) {
#if defined(CONFIG_MPC8548) || defined(CONFIG_MPC8544)
		/*
		 * Yes, the entire PQ38 family use the same
		 * bit-representation for twice the clock divider values.
		 */
		 clkdiv *= 2;
#endif
		printf("LBC:%4lu MHz\n",
		       DIV_ROUND_UP(sysinfo.freqSystemBus, 1000000) / clkdiv);
	} else {
		printf("LBC: unknown (lcrr: 0x%08x)\n", lcrr);
	}

#ifdef CONFIG_CPM2
	printf("CPM:   %lu Mhz\n", sysinfo.freqSystemBus / 1000000);
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
		rstcr = (volatile unsigned int *)(CFG_IMMR + 0xE00B0);
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

#if defined(CONFIG_DDR_ECC)
void dma_init(void) {
	volatile ccsr_dma_t *dma = (void *)(CFG_MPC85xx_DMA_ADDR);

	dma->satr0 = 0x02c40000;
	dma->datr0 = 0x02c40000;
	dma->sr0 = 0xfffffff; /* clear any errors */
	asm("sync; isync; msync");
	return;
}

uint dma_check(void) {
	volatile ccsr_dma_t *dma = (void *)(CFG_MPC85xx_DMA_ADDR);
	volatile uint status = dma->sr0;

	/* While the channel is busy, spin */
	while((status & 4) == 4) {
		status = dma->sr0;
	}

	/* clear MR0[CS] channel start bit */
	dma->mr0 &= 0x00000001;
	asm("sync;isync;msync");

	if (status != 0) {
		printf ("DMA Error: status = %x\n", status);
	}
	return status;
}

int dma_xfer(void *dest, uint count, void *src) {
	volatile ccsr_dma_t *dma = (void *)(CFG_MPC85xx_DMA_ADDR);

	dma->dar0 = (uint) dest;
	dma->sar0 = (uint) src;
	dma->bcr0 = count;
	dma->mr0 = 0xf000004;
	asm("sync;isync;msync");
	dma->mr0 = 0xf000005;
	asm("sync;isync;msync");
	return dma_check();
}
#endif
/*
 * Configures a UPM. Currently, the loop fields in MxMR (RLF, WLF and TLF)
 * are hardcoded as "1"."size" is the number or entries, not a sizeof.
 */
void upmconfig (uint upm, uint * table, uint size)
{
	int i, mdr, mad, old_mad = 0;
	volatile u32 *mxmr;
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);
	int loopval = 0x00004440;
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
		if ((in_be32(brp) & (BR_V | upmmask)) == (BR_V | upmmask)) {
			dummy = (volatile u8*)(in_be32(brp) >> BR_BA_SHIFT);
			break;
		}
	}

	if (i == 8) {
		printf("Error: %s() could not find matching BR\n", __FUNCTION__);
		hang();
	}

	for (i = 0; i < size; i++) {
		/* 1 */
		out_be32(mxmr, loopval | 0x10000000 | i); /* OP_WRITE */
		/* 2 */
		out_be32(&lbc->mdr, table[i]);
		/* 3 */
		mdr = in_be32(&lbc->mdr);
		/* 4 */
		*(volatile u8 *)dummy = 0;
		/* 5 */
		do {
			mad = in_be32(mxmr) & 0x3f;
		} while (mad <= old_mad && !(!mad && i == (size-1)));
		old_mad = mad;
	}
	out_be32(mxmr, loopval); /* OP_NORMAL */
}

#if defined(CONFIG_TSEC_ENET) || defined(CONFIGMPC85XX_FEC)
/* Default initializations for TSEC controllers.  To override,
 * create a board-specific function called:
 * 	int board_eth_init(bd_t *bis)
 */

extern int tsec_initialize(bd_t * bis, int index, char *devname);

int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_TSEC1)
	tsec_initialize(bis, 0, CONFIG_TSEC1_NAME);
#endif
#if defined(CONFIG_TSEC2)
	tsec_initialize(bis, 1, CONFIG_TSEC2_NAME);
#endif
#if defined(CONFIG_MPC85XX_FEC)
	tsec_initialize(bis, 2, CONFIG_MPC85XX_FEC_NAME);
#else
#if defined(CONFIG_TSEC3)
	tsec_initialize(bis, 2, CONFIG_TSEC3_NAME);
#endif
#if defined(CONFIG_TSEC4)
	tsec_initialize(bis, 3, CONFIG_TSEC4_NAME);
#endif
#endif
	return 0;
}
#endif
