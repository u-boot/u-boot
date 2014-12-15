/*
 * Copyright (c) 2014 Google, Inc
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
 * Some portions from coreboot src/mainboard/google/link/romstage.c
 * and src/cpu/intel/model_206ax/bootblock.c
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
#include <asm/lapic.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/arch/model_206ax.h>
#include <asm/arch/microcode.h>
#include <asm/arch/pch.h>
#include <asm/arch/sandybridge.h>

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

static int enable_smbus(void)
{
	pci_dev_t dev;
	uint16_t value;

	/* Set the SMBus device statically. */
	dev = PCI_BDF(0x0, 0x1f, 0x3);

	/* Check to make sure we've got the right device. */
	value = pci_read_config16(dev, 0x0);
	if (value != 0x8086) {
		printf("SMBus controller not found\n");
		return -ENOSYS;
	}

	/* Set SMBus I/O base. */
	pci_write_config32(dev, SMB_BASE,
			   SMBUS_IO_BASE | PCI_BASE_ADDRESS_SPACE_IO);

	/* Set SMBus enable. */
	pci_write_config8(dev, HOSTC, HST_EN);

	/* Set SMBus I/O space enable. */
	pci_write_config16(dev, PCI_COMMAND, PCI_COMMAND_IO);

	/* Disable interrupt generation. */
	outb(0, SMBUS_IO_BASE + SMBHSTCTL);

	/* Clear any lingering errors, so transactions can run. */
	outb(inb(SMBUS_IO_BASE + SMBHSTSTAT), SMBUS_IO_BASE + SMBHSTSTAT);
	debug("SMBus controller enabled\n");

	return 0;
}

#define PCH_EHCI0_TEMP_BAR0 0xe8000000
#define PCH_EHCI1_TEMP_BAR0 0xe8000400
#define PCH_XHCI_TEMP_BAR0  0xe8001000

/*
 * Setup USB controller MMIO BAR to prevent the reference code from
 * resetting the controller.
 *
 * The BAR will be re-assigned during device enumeration so these are only
 * temporary.
 *
 * This is used to speed up the resume path.
 */
static void enable_usb_bar(void)
{
	pci_dev_t usb0 = PCH_EHCI1_DEV;
	pci_dev_t usb1 = PCH_EHCI2_DEV;
	pci_dev_t usb3 = PCH_XHCI_DEV;
	u32 cmd;

	/* USB Controller 1 */
	pci_write_config32(usb0, PCI_BASE_ADDRESS_0,
			   PCH_EHCI0_TEMP_BAR0);
	cmd = pci_read_config32(usb0, PCI_COMMAND);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config32(usb0, PCI_COMMAND, cmd);

	/* USB Controller 1 */
	pci_write_config32(usb1, PCI_BASE_ADDRESS_0,
			   PCH_EHCI1_TEMP_BAR0);
	cmd = pci_read_config32(usb1, PCI_COMMAND);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config32(usb1, PCI_COMMAND, cmd);

	/* USB3 Controller */
	pci_write_config32(usb3, PCI_BASE_ADDRESS_0,
			   PCH_XHCI_TEMP_BAR0);
	cmd = pci_read_config32(usb3, PCI_COMMAND);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config32(usb3, PCI_COMMAND, cmd);
}

static int report_bist_failure(void)
{
	if (gd->arch.bist != 0) {
		post_code(POST_BIST_FAILURE);
		printf("BIST failed: %08x\n", gd->arch.bist);
		return -EFAULT;
	}

	return 0;
}

int print_cpuinfo(void)
{
	enum pei_boot_mode_t boot_mode = PEI_BOOT_NONE;
	char processor_name[CPU_MAX_NAME_LEN];
	const char *name;
	uint32_t pm1_cnt;
	uint16_t pm1_sts;
	int ret;

	/* Halt if there was a built in self test failure */
	ret = report_bist_failure();
	if (ret)
		return ret;

	enable_lapic();

	ret = microcode_update_intel();
	if (ret && ret != -ENOENT && ret != -EEXIST)
		return ret;

	/* Enable upper 128bytes of CMOS */
	writel(1 << 2, RCB_REG(RC));

	/* TODO: cmos_post_init() */
	if (readl(MCHBAR_REG(SSKPD)) == 0xCAFE) {
		debug("soft reset detected\n");
		boot_mode = PEI_BOOT_SOFT_RESET;

		/* System is not happy after keyboard reset... */
		debug("Issuing CF9 warm reset\n");
		outb(0x6, 0xcf9);
		cpu_hlt();
	}

	/* Early chipset init required before RAM init can work */
	sandybridge_early_init(SANDYBRIDGE_MOBILE);

	/* Check PM1_STS[15] to see if we are waking from Sx */
	pm1_sts = inw(DEFAULT_PMBASE + PM1_STS);

	/* Read PM1_CNT[12:10] to determine which Sx state */
	pm1_cnt = inl(DEFAULT_PMBASE + PM1_CNT);

	if ((pm1_sts & WAK_STS) && ((pm1_cnt >> 10) & 7) == 5) {
#if CONFIG_HAVE_ACPI_RESUME
		debug("Resume from S3 detected.\n");
		boot_mode = PEI_BOOT_RESUME;
		/* Clear SLP_TYPE. This will break stage2 but
		 * we care for that when we get there.
		 */
		outl(pm1_cnt & ~(7 << 10), DEFAULT_PMBASE + PM1_CNT);
#else
		debug("Resume from S3 detected, but disabled.\n");
#endif
	} else {
		/*
		 * TODO: An indication of life might be possible here (e.g.
		 * keyboard light)
		 */
	}
	post_code(POST_EARLY_INIT);

	/* Enable SPD ROMs and DDR-III DRAM */
	ret = enable_smbus();
	if (ret)
		return ret;

	/* Prepare USB controller early in S3 resume */
	if (boot_mode == PEI_BOOT_RESUME)
		enable_usb_bar();

	gd->arch.pei_boot_mode = boot_mode;

	/* TODO: Move this to the board or driver */
	pci_write_config32(PCH_LPC_DEV, GPIO_BASE, DEFAULT_GPIOBASE | 1);
	pci_write_config32(PCH_LPC_DEV, GPIO_CNTL, 0x10);

	/* Print processor name */
	name = cpu_get_name(processor_name);
	printf("CPU:   %s\n", name);

	post_code(POST_CPU_INFO);

	return 0;
}
