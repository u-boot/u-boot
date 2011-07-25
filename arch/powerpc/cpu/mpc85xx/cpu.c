/*
 * Copyright 2004,2007-2011 Freescale Semiconductor, Inc.
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
#include <fsl_esdhc.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/fsl_ifc.h>
#include <asm/fsl_law.h>
#include <asm/fsl_lbc.h>
#include <post.h>
#include <asm/processor.h>
#include <asm/fsl_ddr_sdram.h>

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	sys_info_t sysinfo;
	uint pvr, svr;
	uint ver;
	uint major, minor;
	struct cpu_type *cpu;
	char buf1[32], buf2[32];
#if defined(CONFIG_DDR_CLK_FREQ) || defined(CONFIG_FSL_CORENET)
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif /* CONFIG_FSL_CORENET */
#ifdef CONFIG_DDR_CLK_FREQ
	u32 ddr_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO)
		>> MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
#else
#ifdef CONFIG_FSL_CORENET
	u32 ddr_sync = ((gur->rcwsr[5]) & FSL_CORENET_RCWSR5_DDR_SYNC)
		>> FSL_CORENET_RCWSR5_DDR_SYNC_SHIFT;
#else
	u32 ddr_ratio = 0;
#endif /* CONFIG_FSL_CORENET */
#endif /* CONFIG_DDR_CLK_FREQ */
	int i;

	svr = get_svr();
	major = SVR_MAJ(svr);
#ifdef CONFIG_MPC8536
	major &= 0x7; /* the msb of this nibble is a mfg code */
#endif
	minor = SVR_MIN(svr);

	if (cpu_numcores() > 1) {
#ifndef CONFIG_MP
		puts("Unicore software on multiprocessor system!!\n"
		     "To enable mutlticore build define CONFIG_MP\n");
#endif
		volatile ccsr_pic_t *pic = (void *)(CONFIG_SYS_MPC8xxx_PIC_ADDR);
		printf("CPU%d:  ", pic->whoami);
	} else {
		puts("CPU:   ");
	}

	cpu = gd->cpu;

	puts(cpu->name);
	if (IS_E_PROCESSOR(svr))
		puts("E");

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, svr);

	pvr = get_pvr();
	ver = PVR_VER(pvr);
	major = PVR_MAJ(pvr);
	minor = PVR_MIN(pvr);

	printf("Core:  ");
	switch(ver) {
	case PVR_VER_E500_V1:
	case PVR_VER_E500_V2:
		puts("E500");
		break;
	case PVR_VER_E500MC:
		puts("E500MC");
		break;
	case PVR_VER_E5500:
		puts("E5500");
		break;
	default:
		puts("Unknown");
		break;
	}

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, pvr);

	get_sys_info(&sysinfo);

	puts("Clock Configuration:");
	for (i = 0; i < cpu_numcores(); i++) {
		if (!(i & 3))
			printf ("\n       ");
		printf("CPU%d:%-4s MHz, ",
				i,strmhz(buf1, sysinfo.freqProcessor[i]));
	}
	printf("\n       CCB:%-4s MHz,\n", strmhz(buf1, sysinfo.freqSystemBus));

#ifdef CONFIG_FSL_CORENET
	if (ddr_sync == 1) {
		printf("       DDR:%-4s MHz (%s MT/s data rate) "
			"(Synchronous), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
	} else {
		printf("       DDR:%-4s MHz (%s MT/s data rate) "
			"(Asynchronous), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
	}
#else
	switch (ddr_ratio) {
	case 0x0:
		printf("       DDR:%-4s MHz (%s MT/s data rate), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
		break;
	case 0x7:
		printf("       DDR:%-4s MHz (%s MT/s data rate) "
			"(Synchronous), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
		break;
	default:
		printf("       DDR:%-4s MHz (%s MT/s data rate) "
			"(Asynchronous), ",
			strmhz(buf1, sysinfo.freqDDRBus/2),
			strmhz(buf2, sysinfo.freqDDRBus));
		break;
	}
#endif

#if defined(CONFIG_FSL_LBC)
	if (sysinfo.freqLocalBus > LCRR_CLKDIV) {
		printf("LBC:%-4s MHz\n", strmhz(buf1, sysinfo.freqLocalBus));
	} else {
		printf("LBC: unknown (LCRR[CLKDIV] = 0x%02lx)\n",
		       sysinfo.freqLocalBus);
	}
#endif

#ifdef CONFIG_CPM2
	printf("CPM:   %s MHz\n", strmhz(buf1, sysinfo.freqSystemBus));
#endif

#ifdef CONFIG_QE
	printf("       QE:%-4s MHz\n", strmhz(buf1, sysinfo.freqQE));
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	for (i = 0; i < CONFIG_SYS_NUM_FMAN; i++) {
		printf("       FMAN%d: %s MHz\n", i + 1,
			strmhz(buf1, sysinfo.freqFMan[i]));
	}
#endif

#ifdef CONFIG_SYS_DPAA_PME
	printf("       PME:   %s MHz\n", strmhz(buf1, sysinfo.freqPME));
#endif

	puts("L1:    D-cache 32 kB enabled\n       I-cache 32 kB enabled\n");

	return 0;
}


/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
/* Everything after the first generation of PQ3 parts has RSTCR */
#if defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
    defined(CONFIG_MPC8555) || defined(CONFIG_MPC8560)
	unsigned long val, msr;

	/*
	 * Initiate hard reset in debug control register DBCR0
	 * Make sure MSR[DE] = 1.  This only resets the core.
	 */
	msr = mfmsr ();
	msr |= MSR_DE;
	mtmsr (msr);

	val = mfspr(DBCR0);
	val |= 0x70000000;
	mtspr(DBCR0,val);
#else
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	out_be32(&gur->rstcr, 0x2);	/* HRESET_REQ */
	udelay(100);
#endif

	return 1;
}


/*
 * Get timebase clock frequency
 */
#ifndef CONFIG_SYS_FSL_TBCLK_DIV
#define CONFIG_SYS_FSL_TBCLK_DIV 8
#endif
unsigned long get_tbclk (void)
{
	unsigned long tbclk_div = CONFIG_SYS_FSL_TBCLK_DIV;

	return (gd->bus_clk + (tbclk_div >> 1)) / tbclk_div;
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

/*
 * Print out the state of various machine registers.
 * Currently prints out LAWs, BR0/OR0 for LBC, CSPR/CSOR/Timing
 * parameters for IFC and TLBs
 */
void mpc85xx_reginfo(void)
{
	print_tlbcam();
	print_laws();
#if defined(CONFIG_FSL_LBC)
	print_lbc_regs();
#endif
#ifdef CONFIG_FSL_IFC
	print_ifc_regs();
#endif

}

/* Common ddr init for non-corenet fsl 85xx platforms */
#ifndef CONFIG_FSL_CORENET
#if defined(CONFIG_SYS_RAMBOOT) && !defined(CONFIG_SYS_INIT_L2_ADDR)
phys_size_t initdram(int board_type)
{
#if defined(CONFIG_SPD_EEPROM) || defined(CONFIG_DDR_SPD)
	return fsl_ddr_sdram_size();
#else
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
#endif
}
#else /* CONFIG_SYS_RAMBOOT */
phys_size_t initdram(int board_type)
{
	phys_size_t dram_size = 0;

#if defined(CONFIG_SYS_FSL_ERRATUM_DDR_MSYNC_IN)
	{
		ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
		unsigned int x = 10;
		unsigned int i;

		/*
		 * Work around to stabilize DDR DLL
		 */
		out_be32(&gur->ddrdllcr, 0x81000000);
		asm("sync;isync;msync");
		udelay(200);
		while (in_be32(&gur->ddrdllcr) != 0x81000100) {
			setbits_be32(&gur->devdisr, 0x00010000);
			for (i = 0; i < x; i++)
				;
			clrbits_be32(&gur->devdisr, 0x00010000);
			x++;
		}
	}
#endif

#if	defined(CONFIG_SPD_EEPROM)	|| \
	defined(CONFIG_DDR_SPD)		|| \
	defined(CONFIG_SYS_DDR_RAW_TIMING)
	dram_size = fsl_ddr_sdram();
#else
	dram_size = fixed_sdram();
#endif
	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(dram_size);
#endif

#if defined(CONFIG_FSL_LBC)
	/* Some boards also have sdram on the lbc */
	lbc_sdram_init();
#endif

	puts("DDR: ");
	return dram_size;
}
#endif /* CONFIG_SYS_RAMBOOT */
#endif

#if CONFIG_POST & CONFIG_SYS_POST_MEMORY

/* Board-specific functions defined in each board's ddr.c */
void fsl_ddr_get_spd(generic_spd_eeprom_t *ctrl_dimms_spd,
	unsigned int ctrl_num);
void read_tlbcam_entry(int idx, u32 *valid, u32 *tsize, unsigned long *epn,
		       phys_addr_t *rpn);
unsigned int
	setup_ddr_tlbs_phys(phys_addr_t p_addr, unsigned int memsize_in_meg);

void clear_ddr_tlbs_phys(phys_addr_t p_addr, unsigned int memsize_in_meg);

static void dump_spd_ddr_reg(void)
{
	int i, j, k, m;
	u8 *p_8;
	u32 *p_32;
	ccsr_ddr_t *ddr[CONFIG_NUM_DDR_CONTROLLERS];
	generic_spd_eeprom_t
		spd[CONFIG_NUM_DDR_CONTROLLERS][CONFIG_DIMM_SLOTS_PER_CTLR];

	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
		fsl_ddr_get_spd(spd[i], i);

	puts("SPD data of all dimms (zero vaule is omitted)...\n");
	puts("Byte (hex)  ");
	k = 1;
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
		for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++)
			printf("Dimm%d ", k++);
	}
	puts("\n");
	for (k = 0; k < sizeof(generic_spd_eeprom_t); k++) {
		m = 0;
		printf("%3d (0x%02x)  ", k, k);
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				p_8 = (u8 *) &spd[i][j];
				if (p_8[k]) {
					printf("0x%02x  ", p_8[k]);
					m++;
				} else
					puts("      ");
			}
		}
		if (m)
			puts("\n");
		else
			puts("\r");
	}

	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
		switch (i) {
		case 0:
			ddr[i] = (void *)CONFIG_SYS_MPC85xx_DDR_ADDR;
			break;
#ifdef CONFIG_SYS_MPC85xx_DDR2_ADDR
		case 1:
			ddr[i] = (void *)CONFIG_SYS_MPC85xx_DDR2_ADDR;
			break;
#endif
		default:
			printf("%s unexpected controller number = %u\n",
				__func__, i);
			return;
		}
	}
	printf("DDR registers dump for all controllers "
		"(zero vaule is omitted)...\n");
	puts("Offset (hex)   ");
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
		printf("     Base + 0x%04x", (u32)ddr[i] & 0xFFFF);
	puts("\n");
	for (k = 0; k < sizeof(ccsr_ddr_t)/4; k++) {
		m = 0;
		printf("%6d (0x%04x)", k * 4, k * 4);
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			p_32 = (u32 *) ddr[i];
			if (p_32[k]) {
				printf("        0x%08x", p_32[k]);
				m++;
			} else
				puts("                  ");
		}
		if (m)
			puts("\n");
		else
			puts("\r");
	}
	puts("\n");
}

/* invalid the TLBs for DDR and setup new ones to cover p_addr */
static int reset_tlb(phys_addr_t p_addr, u32 size, phys_addr_t *phys_offset)
{
	u32 vstart = CONFIG_SYS_DDR_SDRAM_BASE;
	unsigned long epn;
	u32 tsize, valid, ptr;
	int ddr_esel;

	clear_ddr_tlbs_phys(p_addr, size>>20);

	/* Setup new tlb to cover the physical address */
	setup_ddr_tlbs_phys(p_addr, size>>20);

	ptr = vstart;
	ddr_esel = find_tlb_idx((void *)ptr, 1);
	if (ddr_esel != -1) {
		read_tlbcam_entry(ddr_esel, &valid, &tsize, &epn, phys_offset);
	} else {
		printf("TLB error in function %s\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * slide the testing window up to test another area
 * for 32_bit system, the maximum testable memory is limited to
 * CONFIG_MAX_MEM_MAPPED
 */
int arch_memory_test_advance(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	phys_addr_t test_cap, p_addr;
	phys_size_t p_size = min(gd->ram_size, CONFIG_MAX_MEM_MAPPED);

#if !defined(CONFIG_PHYS_64BIT) || \
    !defined(CONFIG_SYS_INIT_RAM_ADDR_PHYS) || \
	(CONFIG_SYS_INIT_RAM_ADDR_PHYS < 0x100000000ull)
		test_cap = p_size;
#else
		test_cap = gd->ram_size;
#endif
	p_addr = (*vstart) + (*size) + (*phys_offset);
	if (p_addr < test_cap - 1) {
		p_size = min(test_cap - p_addr, CONFIG_MAX_MEM_MAPPED);
		if (reset_tlb(p_addr, p_size, phys_offset) == -1)
			return -1;
		*vstart = CONFIG_SYS_DDR_SDRAM_BASE;
		*size = (u32) p_size;
		printf("Testing 0x%08llx - 0x%08llx\n",
			(u64)(*vstart) + (*phys_offset),
			(u64)(*vstart) + (*phys_offset) + (*size) - 1);
	} else
		return 1;

	return 0;
}

/* initialization for testing area */
int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	phys_size_t p_size = min(gd->ram_size, CONFIG_MAX_MEM_MAPPED);

	*vstart = CONFIG_SYS_DDR_SDRAM_BASE;
	*size = (u32) p_size;	/* CONFIG_MAX_MEM_MAPPED < 4G */
	*phys_offset = 0;

#if !defined(CONFIG_PHYS_64BIT) || \
    !defined(CONFIG_SYS_INIT_RAM_ADDR_PHYS) || \
	(CONFIG_SYS_INIT_RAM_ADDR_PHYS < 0x100000000ull)
		if (gd->ram_size > CONFIG_MAX_MEM_MAPPED) {
			puts("Cannot test more than ");
			print_size(CONFIG_MAX_MEM_MAPPED,
				" without proper 36BIT support.\n");
		}
#endif
	printf("Testing 0x%08llx - 0x%08llx\n",
		(u64)(*vstart) + (*phys_offset),
		(u64)(*vstart) + (*phys_offset) + (*size) - 1);

	return 0;
}

/* invalid TLBs for DDR and remap as normal after testing */
int arch_memory_test_cleanup(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	unsigned long epn;
	u32 tsize, valid, ptr;
	phys_addr_t rpn = 0;
	int ddr_esel;

	/* disable the TLBs for this testing */
	ptr = *vstart;

	while (ptr < (*vstart) + (*size)) {
		ddr_esel = find_tlb_idx((void *)ptr, 1);
		if (ddr_esel != -1) {
			read_tlbcam_entry(ddr_esel, &valid, &tsize, &epn, &rpn);
			disable_tlb(ddr_esel);
		}
		ptr += TSIZE_TO_BYTES(tsize);
	}

	puts("Remap DDR ");
	setup_ddr_tlbs(gd->ram_size>>20);
	puts("\n");

	return 0;
}

void arch_memory_failure_handle(void)
{
	dump_spd_ddr_reg();
}
#endif
