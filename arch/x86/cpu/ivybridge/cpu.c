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
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <pch.h>
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
	outb(SYS_RST | RST_CPU, PORT_RESET);
	cpu_hlt();

	/* Not reached */
	return -EINVAL;
}

int arch_cpu_init(void)
{
	post_code(POST_CPU_INIT);

	return x86_cpu_init_f();
}

int arch_cpu_init_dm(void)
{
	struct pci_controller *hose;
	struct udevice *bus, *dev;
	int ret;

	post_code(0x70);
	ret = uclass_get_device(UCLASS_PCI, 0, &bus);
	post_code(0x71);
	if (ret)
		return ret;
	post_code(0x72);
	hose = dev_get_uclass_priv(bus);

	/* TODO(sjg@chromium.org): Get rid of gd->hose */
	gd->hose = hose;

	ret = uclass_first_device(UCLASS_LPC, &dev);
	if (!dev)
		return -ENODEV;

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
static void enable_usb_bar(struct udevice *bus)
{
	pci_dev_t usb0 = PCH_EHCI1_DEV;
	pci_dev_t usb1 = PCH_EHCI2_DEV;
	pci_dev_t usb3 = PCH_XHCI_DEV;
	ulong cmd;

	/* USB Controller 1 */
	pci_bus_write_config(bus, usb0, PCI_BASE_ADDRESS_0,
			     PCH_EHCI0_TEMP_BAR0, PCI_SIZE_32);
	pci_bus_read_config(bus, usb0, PCI_COMMAND, &cmd, PCI_SIZE_32);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_bus_write_config(bus, usb0, PCI_COMMAND, cmd, PCI_SIZE_32);

	/* USB Controller 2 */
	pci_bus_write_config(bus, usb1, PCI_BASE_ADDRESS_0,
			     PCH_EHCI1_TEMP_BAR0, PCI_SIZE_32);
	pci_bus_read_config(bus, usb1, PCI_COMMAND, &cmd, PCI_SIZE_32);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_bus_write_config(bus, usb1, PCI_COMMAND, cmd, PCI_SIZE_32);

	/* USB3 Controller 1 */
	pci_bus_write_config(bus, usb3, PCI_BASE_ADDRESS_0,
			     PCH_XHCI_TEMP_BAR0, PCI_SIZE_32);
	pci_bus_read_config(bus, usb3, PCI_COMMAND, &cmd, PCI_SIZE_32);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_bus_write_config(bus, usb3, PCI_COMMAND, cmd, PCI_SIZE_32);
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
	struct udevice *dev, *lpc;
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
	if (ret)
		return ret;

	/* Enable upper 128bytes of CMOS */
	writel(1 << 2, RCB_REG(RC));

	/* TODO: cmos_post_init() */
	if (readl(MCHBAR_REG(SSKPD)) == 0xCAFE) {
		debug("soft reset detected\n");
		boot_mode = PEI_BOOT_SOFT_RESET;

		/* System is not happy after keyboard reset... */
		debug("Issuing CF9 warm reset\n");
		reset_cpu(0);
	}

	/* Early chipset init required before RAM init can work */
	uclass_first_device(UCLASS_NORTHBRIDGE, &dev);

	ret = uclass_first_device(UCLASS_LPC, &lpc);
	if (ret)
		return ret;
	if (!dev)
		return -ENODEV;

	/* Cause the SATA device to do its early init */
	uclass_first_device(UCLASS_DISK, &dev);

	/* Check PM1_STS[15] to see if we are waking from Sx */
	pm1_sts = inw(DEFAULT_PMBASE + PM1_STS);

	/* Read PM1_CNT[12:10] to determine which Sx state */
	pm1_cnt = inl(DEFAULT_PMBASE + PM1_CNT);

	if ((pm1_sts & WAK_STS) && ((pm1_cnt >> 10) & 7) == 5) {
		debug("Resume from S3 detected, but disabled.\n");
	} else {
		/*
		 * TODO: An indication of life might be possible here (e.g.
		 * keyboard light)
		 */
	}
	post_code(POST_EARLY_INIT);

	/* Enable SPD ROMs and DDR-III DRAM */
	ret = uclass_first_device(UCLASS_I2C, &dev);
	if (ret)
		return ret;
	if (!dev)
		return -ENODEV;

	/* Prepare USB controller early in S3 resume */
	if (boot_mode == PEI_BOOT_RESUME)
		enable_usb_bar(pci_get_controller(lpc->parent));

	gd->arch.pei_boot_mode = boot_mode;

	/* Print processor name */
	name = cpu_get_name(processor_name);
	printf("CPU:   %s\n", name);

	post_code(POST_CPU_INFO);

	return 0;
}

void board_debug_uart_init(void)
{
	/* This enables the debug UART */
	pci_x86_write_config(NULL, PCH_LPC_DEV, LPC_EN, COMA_LPC_EN,
			     PCI_SIZE_16);
}
