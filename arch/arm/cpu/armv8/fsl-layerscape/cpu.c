/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/arch/cpu.h>
#include <asm/arch/speed.h>
#ifdef CONFIG_MP
#include <asm/arch/mp.h>
#endif
#include <fm_eth.h>
#include <fsl_debug_server.h>
#include <fsl-mc/fsl_mc.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
#include <asm/armv8/sec_firmware.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

struct mm_region *mem_map = early_map;

void cpu_name(char *name)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int i, svr, ver;

	svr = gur_in32(&gur->svr);
	ver = SVR_SOC_VER(svr);

	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++)
		if ((cpu_type_list[i].soc_ver & SVR_WO_E) == ver) {
			strcpy(name, cpu_type_list[i].name);

			if (IS_E_PROCESSOR(svr))
				strcat(name, "E");
			break;
		}

	if (i == ARRAY_SIZE(cpu_type_list))
		strcpy(name, "unknown");
}

#ifndef CONFIG_SYS_DCACHE_OFF
/*
 * To start MMU before DDR is available, we create MMU table in SRAM.
 * The base address of SRAM is CONFIG_SYS_FSL_OCRAM_BASE. We use three
 * levels of translation tables here to cover 40-bit address space.
 * We use 4KB granule size, with 40 bits physical address, T0SZ=24
 * Address above EARLY_PGTABLE_SIZE (0x5000) is free for other purpose.
 * Note, the debug print in cache_v8.c is not usable for debugging
 * these early MMU tables because UART is not yet available.
 */
static inline void early_mmu_setup(void)
{
	unsigned int el = current_el();

	/* global data is already setup, no allocation yet */
	gd->arch.tlb_addr = CONFIG_SYS_FSL_OCRAM_BASE;
	gd->arch.tlb_fillptr = gd->arch.tlb_addr;
	gd->arch.tlb_size = EARLY_PGTABLE_SIZE;

	/* Create early page tables */
	setup_pgtables();

	/* point TTBR to the new table */
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr,
			  get_tcr(el, NULL, NULL) &
			  ~(TCR_ORGN_MASK | TCR_IRGN_MASK),
			  MEMORY_ATTRIBUTES);

	set_sctlr(get_sctlr() | CR_M);
}

/*
 * The final tables look similar to early tables, but different in detail.
 * These tables are in DRAM. Sub tables are added to enable cache for
 * QBMan and OCRAM.
 *
 * Put the MMU table in secure memory if gd->arch.secure_ram is valid.
 * OCRAM will be not used for this purpose so gd->arch.secure_ram can't be 0.
 */
static inline void final_mmu_setup(void)
{
	u64 tlb_addr_save = gd->arch.tlb_addr;
	unsigned int el = current_el();
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	int index;
#endif

	mem_map = final_map;

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	if (gd->arch.secure_ram & MEM_RESERVE_SECURE_MAINTAINED) {
		if (el == 3) {
			/*
			 * Only use gd->arch.secure_ram if the address is
			 * recalculated. Align to 4KB for MMU table.
			 */
			/* put page tables in secure ram */
			index = ARRAY_SIZE(final_map) - 2;
			gd->arch.tlb_addr = gd->arch.secure_ram & ~0xfff;
			final_map[index].virt = gd->arch.secure_ram & ~0x3;
			final_map[index].phys = final_map[index].virt;
			final_map[index].size = CONFIG_SYS_MEM_RESERVE_SECURE;
			final_map[index].attrs = PTE_BLOCK_OUTER_SHARE;
			gd->arch.secure_ram |= MEM_RESERVE_SECURE_SECURED;
			tlb_addr_save = gd->arch.tlb_addr;
		} else {
			/* Use allocated (board_f.c) memory for TLB */
			tlb_addr_save = gd->arch.tlb_allocated;
			gd->arch.tlb_addr = tlb_addr_save;
		}
	}
#endif

	/* Reset the fill ptr */
	gd->arch.tlb_fillptr = tlb_addr_save;

	/* Create normal system page tables */
	setup_pgtables();

	/* Create emergency page tables */
	gd->arch.tlb_addr = gd->arch.tlb_fillptr;
	gd->arch.tlb_emerg = gd->arch.tlb_addr;
	setup_pgtables();
	gd->arch.tlb_addr = tlb_addr_save;

	/* flush new MMU table */
	flush_dcache_range(gd->arch.tlb_addr,
			   gd->arch.tlb_addr + gd->arch.tlb_size);

	/* point TTBR to the new table */
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr, get_tcr(el, NULL, NULL),
			  MEMORY_ATTRIBUTES);
	/*
	 * EL3 MMU is already enabled, just need to invalidate TLB to load the
	 * new table. The new table is compatible with the current table, if
	 * MMU somehow walks through the new table before invalidation TLB,
	 * it still works. So we don't need to turn off MMU here.
	 * When EL2 MMU table is created by calling this function, MMU needs
	 * to be enabled.
	 */
	set_sctlr(get_sctlr() | CR_M);
}

u64 get_page_table_size(void)
{
	return 0x10000;
}

int arch_cpu_init(void)
{
	icache_enable();
	__asm_invalidate_dcache_all();
	__asm_invalidate_tlb_all();
	early_mmu_setup();
	set_sctlr(get_sctlr() | CR_C);
	return 0;
}

void mmu_setup(void)
{
	final_mmu_setup();
}

/*
 * This function is called from common/board_r.c.
 * It recreates MMU table in main memory.
 */
void enable_caches(void)
{
	mmu_setup();
	__asm_invalidate_tlb_all();
	icache_enable();
	dcache_enable();
}
#endif

static inline u32 initiator_type(u32 cluster, int init_id)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 idx = (cluster >> (init_id * 8)) & TP_CLUSTER_INIT_MASK;
	u32 type = 0;

	type = gur_in32(&gur->tp_ityp[idx]);
	if (type & TP_ITYP_AV)
		return type;

	return 0;
}

u32 cpu_mask(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster, type, mask = 0;

	do {
		int j;

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM)
					mask |= 1 << count;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return mask;
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	return hweight32(cpu_mask());
}

int fsl_qoriq_core_to_cluster(unsigned int core)
{
	struct ccsr_gur __iomem *gur =
		(void __iomem *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster;

	do {
		int j;

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			if (initiator_type(cluster, j)) {
				if (count == core)
					return i;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return -1;      /* cannot identify the cluster */
}

u32 fsl_qoriq_core_to_type(unsigned int core)
{
	struct ccsr_gur __iomem *gur =
		(void __iomem *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster, type;

	do {
		int j;

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (count == core)
					return type;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return -1;      /* cannot identify the cluster */
}

uint get_svr(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	return gur_in32(&gur->svr);
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct sys_info sysinfo;
	char buf[32];
	unsigned int i, core;
	u32 type, rcw, svr = gur_in32(&gur->svr);

	puts("SoC: ");

	cpu_name(buf);
	printf(" %s (0x%x)\n", buf, svr);
	memset((u8 *)buf, 0x00, ARRAY_SIZE(buf));
	get_sys_info(&sysinfo);
	puts("Clock Configuration:");
	for_each_cpu(i, core, cpu_numcores(), cpu_mask()) {
		if (!(i % 3))
			puts("\n       ");
		type = TP_ITYP_VER(fsl_qoriq_core_to_type(core));
		printf("CPU%d(%s):%-4s MHz  ", core,
		       type == TY_ITYP_VER_A7 ? "A7 " :
		       (type == TY_ITYP_VER_A53 ? "A53" :
		       (type == TY_ITYP_VER_A57 ? "A57" :
		       (type == TY_ITYP_VER_A72 ? "A72" : "   "))),
		       strmhz(buf, sysinfo.freq_processor[core]));
	}
	printf("\n       Bus:      %-4s MHz  ",
	       strmhz(buf, sysinfo.freq_systembus));
	printf("DDR:      %-4s MT/s", strmhz(buf, sysinfo.freq_ddrbus));
#ifdef CONFIG_SYS_DPAA_FMAN
	printf("  FMAN:     %-4s MHz", strmhz(buf, sysinfo.freq_fman[0]));
#endif
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	if (soc_has_dp_ddr()) {
		printf("     DP-DDR:   %-4s MT/s",
		       strmhz(buf, sysinfo.freq_ddrbus2));
	}
#endif
	puts("\n");

	/*
	 * Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		rcw = gur_in32(&gur->rcwsr[i]);
		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int error = 0;

#ifdef CONFIG_FSL_MC_ENET
	error = fsl_mc_ldpaa_init(bis);
#endif
#ifdef CONFIG_FMAN_ENET
	fm_standard_init(bis);
#endif
	return error;
}

int arch_early_init_r(void)
{
#ifdef CONFIG_MP
	int rv = 1;
	u32 psci_ver = 0xffffffff;
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A009635
	erratum_a009635();
#endif

#ifdef CONFIG_MP
#if defined(CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT) && defined(CONFIG_ARMV8_PSCI)
	/* Check the psci version to determine if the psci is supported */
	psci_ver = sec_firmware_support_psci_version();
#endif
	if (psci_ver == 0xffffffff) {
		rv = fsl_layerscape_wake_seconday_cores();
		if (rv)
			printf("Did not wake secondary cores\n");
	}
#endif

#ifdef CONFIG_SYS_HAS_SERDES
	fsl_serdes_init();
#endif
#ifdef CONFIG_FMAN_ENET
	fman_enet_init();
#endif
	return 0;
}

int timer_init(void)
{
	u32 __iomem *cntcr = (u32 *)CONFIG_SYS_FSL_TIMER_ADDR;
#ifdef CONFIG_FSL_LSCH3
	u32 __iomem *cltbenr = (u32 *)CONFIG_SYS_FSL_PMU_CLTBENR;
#endif
#ifdef CONFIG_LS2080A
	u32 __iomem *pctbenr = (u32 *)FSL_PMU_PCTBENR_OFFSET;
#endif
#ifdef COUNTER_FREQUENCY_REAL
	unsigned long cntfrq = COUNTER_FREQUENCY_REAL;

	/* Update with accurate clock frequency */
	asm volatile("msr cntfrq_el0, %0" : : "r" (cntfrq) : "memory");
#endif

#ifdef CONFIG_FSL_LSCH3
	/* Enable timebase for all clusters.
	 * It is safe to do so even some clusters are not enabled.
	 */
	out_le32(cltbenr, 0xf);
#endif

#ifdef CONFIG_LS2080A
	/*
	 * In certain Layerscape SoCs, the clock for each core's
	 * has an enable bit in the PMU Physical Core Time Base Enable
	 * Register (PCTBENR), which allows the watchdog to operate.
	 */
	setbits_le32(pctbenr, 0xff);
#endif

	/* Enable clock for timer
	 * This is a global setting.
	 */
	out_le32(cntcr, 0x1);

	return 0;
}

void reset_cpu(ulong addr)
{
	u32 __iomem *rstcr = (u32 *)CONFIG_SYS_FSL_RST_ADDR;
	u32 val;

	/* Raise RESET_REQ_B */
	val = scfg_in32(rstcr);
	val |= 0x02;
	scfg_out32(rstcr, val);
}

phys_size_t board_reserve_ram_top(phys_size_t ram_size)
{
	phys_size_t ram_top = ram_size;

#ifdef CONFIG_SYS_MEM_TOP_HIDE
#error CONFIG_SYS_MEM_TOP_HIDE not to be used together with this function
#endif
/* Carve the Debug Server private DRAM block from the end of DRAM */
#ifdef CONFIG_FSL_DEBUG_SERVER
	ram_top -= debug_server_get_dram_block_size();
#endif

/* Carve the MC private DRAM block from the end of DRAM */
#ifdef CONFIG_FSL_MC_ENET
	ram_top -= mc_get_dram_block_size();
	ram_top &= ~(CONFIG_SYS_MC_RSV_MEM_ALIGN - 1);
#endif

	return ram_top;
}
