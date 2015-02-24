/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/arch/device.h>
#include <asm/arch/msg_port.h>
#include <asm/arch/quark.h>

static struct pci_device_id mmc_supported[] = {
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_QRK_SDIO },
};

/*
 * TODO:
 *
 * This whole routine should be removed until we fully convert the ICH SPI
 * driver to DM and make use of DT to pass the bios control register offset
 */
static void unprotect_spi_flash(void)
{
	u32 bc;

	bc = pci_read_config32(QUARK_LEGACY_BRIDGE, 0xd8);
	bc |= 0x1;	/* unprotect the flash */
	pci_write_config32(QUARK_LEGACY_BRIDGE, 0xd8, bc);
}

static void quark_setup_bars(void)
{
	/* GPIO - D31:F0:R44h */
	pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_GBA,
			       CONFIG_GPIO_BASE | IO_BAR_EN);

	/* ACPI PM1 Block - D31:F0:R48h */
	pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_PM1BLK,
			       CONFIG_ACPI_PM1_BASE | IO_BAR_EN);

	/* GPE0 - D31:F0:R4Ch */
	pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_GPE0BLK,
			       CONFIG_ACPI_GPE0_BASE | IO_BAR_EN);

	/* WDT - D31:F0:R84h */
	pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_WDTBA,
			       CONFIG_WDT_BASE | IO_BAR_EN);

	/* RCBA - D31:F0:RF0h */
	pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_RCBA,
			       CONFIG_RCBA_BASE | MEM_BAR_EN);

	/* ACPI P Block - Msg Port 04:R70h */
	msg_port_write(MSG_PORT_RMU, PBLK_BA,
		       CONFIG_ACPI_PBLK_BASE | IO_BAR_EN);

	/* SPI DMA - Msg Port 04:R7Ah */
	msg_port_write(MSG_PORT_RMU, SPI_DMA_BA,
		       CONFIG_SPI_DMA_BASE | IO_BAR_EN);

	/* PCIe ECAM */
	msg_port_write(MSG_PORT_MEM_ARBITER, AEC_CTRL,
		       CONFIG_PCIE_ECAM_BASE | MEM_BAR_EN);
	msg_port_write(MSG_PORT_HOST_BRIDGE, HEC_REG,
		       CONFIG_PCIE_ECAM_BASE | MEM_BAR_EN);
}

int arch_cpu_init(void)
{
	struct pci_controller *hose;
	int ret;

	post_code(POST_CPU_INIT);
#ifdef CONFIG_SYS_X86_TSC_TIMER
	timer_set_base(rdtsc());
#endif

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	ret = pci_early_init_hose(&hose);
	if (ret)
		return ret;

	/*
	 * Quark SoC has some non-standard BARs (excluding PCI standard BARs)
	 * which need be initialized with suggested values
	 */
	quark_setup_bars();

	unprotect_spi_flash();

	return 0;
}

int print_cpuinfo(void)
{
	post_code(POST_CPU_INFO);
	return default_print_cpuinfo();
}

void reset_cpu(ulong addr)
{
	/* cold reset */
	outb(0x08, PORT_RESET);
}

int cpu_mmc_init(bd_t *bis)
{
	return pci_mmc_init("Quark SDHCI", mmc_supported,
			    ARRAY_SIZE(mmc_supported));
}
