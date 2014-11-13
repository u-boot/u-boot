/*
 * Copyright (c) 2014 Google, Inc
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
 * Some portions from coreboot src/mainboard/google/link/romstage.c
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/arch/model_206ax.h>
#include <asm/arch/microcode.h>
#include <asm/arch/pch.h>

DECLARE_GLOBAL_DATA_PTR;

static void enable_port80_on_lpc(struct pci_controller *hose, pci_dev_t dev)
{
	/* Enable port 80 POST on LPC */
	pci_hose_write_config_dword(hose, dev, PCH_RCBA_BASE, DEFAULT_RCBA | 1);
	clrbits_le32(RCB_REG(GCS), 4);
}

/*
 * Enable Prefetching and Caching.
 */
static void enable_spi_prefetch(struct pci_controller *hose, pci_dev_t dev)
{
	u8 reg8;

	pci_hose_read_config_byte(hose, dev, 0xdc, &reg8);
	reg8 &= ~(3 << 2);
	reg8 |= (2 << 2); /* Prefetching and Caching Enabled */
	pci_hose_write_config_byte(hose, dev, 0xdc, reg8);
}

static void set_var_mtrr(
	unsigned reg, unsigned base, unsigned size, unsigned type)

{
	/* Bit Bit 32-35 of MTRRphysMask should be set to 1 */
	/* FIXME: It only support 4G less range */
	wrmsr(MTRRphysBase_MSR(reg), base | type, 0);
	wrmsr(MTRRphysMask_MSR(reg), ~(size - 1) | MTRRphysMaskValid,
	      (1 << (CONFIG_CPU_ADDR_BITS - 32)) - 1);
}

static void enable_rom_caching(void)
{
	disable_caches();
	set_var_mtrr(1, 0xffc00000, 4 << 20, MTRR_TYPE_WRPROT);
	enable_caches();

	/* Enable Variable MTRRs */
	wrmsr(MTRRdefType_MSR, 0x800, 0);
}

static int set_flex_ratio_to_tdp_nominal(void)
{
	msr_t flex_ratio, msr;
	u8 nominal_ratio;

	/* Minimum CPU revision for configurable TDP support */
	if (cpuid_eax(1) < IVB_CONFIG_TDP_MIN_CPUID)
		return -EINVAL;

	/* Check for Flex Ratio support */
	flex_ratio = msr_read(MSR_FLEX_RATIO);
	if (!(flex_ratio.lo & FLEX_RATIO_EN))
		return -EINVAL;

	/* Check for >0 configurable TDPs */
	msr = msr_read(MSR_PLATFORM_INFO);
	if (((msr.hi >> 1) & 3) == 0)
		return -EINVAL;

	/* Use nominal TDP ratio for flex ratio */
	msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
	nominal_ratio = msr.lo & 0xff;

	/* See if flex ratio is already set to nominal TDP ratio */
	if (((flex_ratio.lo >> 8) & 0xff) == nominal_ratio)
		return 0;

	/* Set flex ratio to nominal TDP ratio */
	flex_ratio.lo &= ~0xff00;
	flex_ratio.lo |= nominal_ratio << 8;
	flex_ratio.lo |= FLEX_RATIO_LOCK;
	msr_write(MSR_FLEX_RATIO, flex_ratio);

	/* Set flex ratio in soft reset data register bits 11:6 */
	clrsetbits_le32(RCB_REG(SOFT_RESET_DATA), 0x3f << 6,
			(nominal_ratio & 0x3f) << 6);

	/* Set soft reset control to use register value */
	setbits_le32(RCB_REG(SOFT_RESET_CTRL), 1);

	/* Issue warm reset, will be "CPU only" due to soft reset data */
	outb(0x0, PORT_RESET);
	outb(0x6, PORT_RESET);
	cpu_hlt();

	/* Not reached */
	return -EINVAL;
}

static void set_spi_speed(void)
{
	u32 fdod;

	/* Observe SPI Descriptor Component Section 0 */
	writel(0x1000, RCB_REG(SPI_DESC_COMP0));

	/* Extract the1 Write/Erase SPI Frequency from descriptor */
	fdod = readl(RCB_REG(SPI_FREQ_WR_ERA));
	fdod >>= 24;
	fdod &= 7;

	/* Set Software Sequence frequency to match */
	clrsetbits_8(RCB_REG(SPI_FREQ_SWSEQ), 7, fdod);
}

int arch_cpu_init(void)
{
	const void *blob = gd->fdt_blob;
	struct pci_controller *hose;
	int node;
	int ret;

	post_code(POST_CPU_INIT);
	timer_set_base(rdtsc());

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	ret = pci_early_init_hose(&hose);
	if (ret)
		return ret;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INTEL_LPC);
	if (node < 0)
		return -ENOENT;
	ret = lpc_early_init(gd->fdt_blob, node, PCH_LPC_DEV);
	if (ret)
		return ret;

	enable_spi_prefetch(hose, PCH_LPC_DEV);

	/* This is already done in start.S, but let's do it in C */
	enable_port80_on_lpc(hose, PCH_LPC_DEV);

	/* already done in car.S */
	if (false)
		enable_rom_caching();

	set_spi_speed();

	/*
	 * We should do as little as possible before the serial console is
	 * up. Perhaps this should move to later. Our next lot of init
	 * happens in print_cpuinfo() when we have a console
	 */
	ret = set_flex_ratio_to_tdp_nominal();
	if (ret)
		return ret;

	return 0;
}

static int report_bist_failure(void)
{
	if (gd->arch.bist != 0) {
		printf("BIST failed: %08x\n", gd->arch.bist);
		return -EFAULT;
	}

	return 0;
}

int print_cpuinfo(void)
{
	char processor_name[CPU_MAX_NAME_LEN];
	const char *name;
	int ret;

	/* Halt if there was a built in self test failure */
	ret = report_bist_failure();
	if (ret)
		return ret;

	ret = microcode_update_intel();
	if (ret && ret != -ENOENT && ret != -EEXIST)
		return ret;

	/* Print processor name */
	name = cpu_get_name(processor_name);
	printf("CPU:   %s\n", name);

	return 0;
}
