/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/irq.h>
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

	qrk_pci_read_config_dword(QUARK_LEGACY_BRIDGE, 0xd8, &bc);
	bc |= 0x1;	/* unprotect the flash */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, 0xd8, bc);
}

static void quark_setup_bars(void)
{
	/* GPIO - D31:F0:R44h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_GBA,
				   CONFIG_GPIO_BASE | IO_BAR_EN);

	/* ACPI PM1 Block - D31:F0:R48h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_PM1BLK,
				   CONFIG_ACPI_PM1_BASE | IO_BAR_EN);

	/* GPE0 - D31:F0:R4Ch */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_GPE0BLK,
				   CONFIG_ACPI_GPE0_BASE | IO_BAR_EN);

	/* WDT - D31:F0:R84h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_WDTBA,
				   CONFIG_WDT_BASE | IO_BAR_EN);

	/* RCBA - D31:F0:RF0h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_RCBA,
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

static void quark_pcie_early_init(void)
{
	u32 pcie_cfg;

	/*
	 * Step1: Assert PCIe signal PERST#
	 *
	 * The CPU interface to the PERST# signal is platform dependent.
	 * Call the board-specific codes to perform this task.
	 */
	board_assert_perst();

	/* Step2: PHY common lane reset */
	pcie_cfg = msg_port_alt_read(MSG_PORT_SOC_UNIT, PCIE_CFG);
	pcie_cfg |= PCIE_PHY_LANE_RST;
	msg_port_alt_write(MSG_PORT_SOC_UNIT, PCIE_CFG, pcie_cfg);
	/* wait 1 ms for PHY common lane reset */
	mdelay(1);

	/* Step3: PHY sideband interface reset and controller main reset */
	pcie_cfg = msg_port_alt_read(MSG_PORT_SOC_UNIT, PCIE_CFG);
	pcie_cfg |= (PCIE_PHY_SB_RST | PCIE_CTLR_MAIN_RST);
	msg_port_alt_write(MSG_PORT_SOC_UNIT, PCIE_CFG, pcie_cfg);
	/* wait 80ms for PLL to lock */
	mdelay(80);

	/* Step4: Controller sideband interface reset */
	pcie_cfg = msg_port_alt_read(MSG_PORT_SOC_UNIT, PCIE_CFG);
	pcie_cfg |= PCIE_CTLR_SB_RST;
	msg_port_alt_write(MSG_PORT_SOC_UNIT, PCIE_CFG, pcie_cfg);
	/* wait 20ms for controller sideband interface reset */
	mdelay(20);

	/* Step5: De-assert PERST# */
	board_deassert_perst();

	/* Step6: Controller primary interface reset */
	pcie_cfg = msg_port_alt_read(MSG_PORT_SOC_UNIT, PCIE_CFG);
	pcie_cfg |= PCIE_CTLR_PRI_RST;
	msg_port_alt_write(MSG_PORT_SOC_UNIT, PCIE_CFG, pcie_cfg);

	/* Mixer Load Lane 0 */
	pcie_cfg = msg_port_io_read(MSG_PORT_PCIE_AFE, PCIE_RXPICTRL0_L0);
	pcie_cfg &= ~((1 << 6) | (1 << 7));
	msg_port_io_write(MSG_PORT_PCIE_AFE, PCIE_RXPICTRL0_L0, pcie_cfg);

	/* Mixer Load Lane 1 */
	pcie_cfg = msg_port_io_read(MSG_PORT_PCIE_AFE, PCIE_RXPICTRL0_L1);
	pcie_cfg &= ~((1 << 6) | (1 << 7));
	msg_port_io_write(MSG_PORT_PCIE_AFE, PCIE_RXPICTRL0_L1, pcie_cfg);
}

static void quark_usb_early_init(void)
{
	u32 usb;

	/* The sequence below comes from Quark firmware writer guide */

	usb = msg_port_alt_read(MSG_PORT_USB_AFE, USB2_GLOBAL_PORT);
	usb &= ~(1 << 1);
	usb |= ((1 << 6) | (1 << 7));
	msg_port_alt_write(MSG_PORT_USB_AFE, USB2_GLOBAL_PORT, usb);

	usb = msg_port_alt_read(MSG_PORT_USB_AFE, USB2_COMPBG);
	usb &= ~((1 << 8) | (1 << 9));
	usb |= ((1 << 7) | (1 << 10));
	msg_port_alt_write(MSG_PORT_USB_AFE, USB2_COMPBG, usb);

	usb = msg_port_alt_read(MSG_PORT_USB_AFE, USB2_PLL2);
	usb |= (1 << 29);
	msg_port_alt_write(MSG_PORT_USB_AFE, USB2_PLL2, usb);

	usb = msg_port_alt_read(MSG_PORT_USB_AFE, USB2_PLL1);
	usb |= (1 << 1);
	msg_port_alt_write(MSG_PORT_USB_AFE, USB2_PLL1, usb);

	usb = msg_port_alt_read(MSG_PORT_USB_AFE, USB2_PLL1);
	usb &= ~((1 << 3) | (1 << 4) | (1 << 5));
	usb |= (1 << 6);
	msg_port_alt_write(MSG_PORT_USB_AFE, USB2_PLL1, usb);

	usb = msg_port_alt_read(MSG_PORT_USB_AFE, USB2_PLL2);
	usb &= ~(1 << 29);
	msg_port_alt_write(MSG_PORT_USB_AFE, USB2_PLL2, usb);

	usb = msg_port_alt_read(MSG_PORT_USB_AFE, USB2_PLL2);
	usb |= (1 << 24);
	msg_port_alt_write(MSG_PORT_USB_AFE, USB2_PLL2, usb);
}

static void quark_enable_legacy_seg(void)
{
	u32 hmisc2;

	hmisc2 = msg_port_read(MSG_PORT_HOST_BRIDGE, HMISC2);
	hmisc2 |= (HMISC2_SEGE | HMISC2_SEGF | HMISC2_SEGAB);
	msg_port_write(MSG_PORT_HOST_BRIDGE, HMISC2, hmisc2);
}

int arch_cpu_init(void)
{
	int ret;

	post_code(POST_CPU_INIT);
#ifdef CONFIG_SYS_X86_TSC_TIMER
	timer_set_base(rdtsc());
#endif

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	/*
	 * Quark SoC has some non-standard BARs (excluding PCI standard BARs)
	 * which need be initialized with suggested values
	 */
	quark_setup_bars();

	/*
	 * Initialize PCIe controller
	 *
	 * Quark SoC holds the PCIe controller in reset following a power on.
	 * U-Boot needs to release the PCIe controller from reset. The PCIe
	 * controller (D23:F0/F1) will not be visible in PCI configuration
	 * space and any access to its PCI configuration registers will cause
	 * system hang while it is held in reset.
	 */
	quark_pcie_early_init();

	/* Initialize USB2 PHY */
	quark_usb_early_init();

	/* Turn on legacy segments (A/B/E/F) decode to system RAM */
	quark_enable_legacy_seg();

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
	x86_full_reset();
}

int cpu_mmc_init(bd_t *bis)
{
	return pci_mmc_init("Quark SDHCI", mmc_supported,
			    ARRAY_SIZE(mmc_supported));
}

void cpu_irq_init(void)
{
	struct quark_rcba *rcba;
	u32 base;

	qrk_pci_read_config_dword(QUARK_LEGACY_BRIDGE, LB_RCBA, &base);
	base &= ~MEM_BAR_EN;
	rcba = (struct quark_rcba *)base;

	/*
	 * Route Quark PCI device interrupt pin to PIRQ
	 *
	 * Route device#23's INTA/B/C/D to PIRQA/B/C/D
	 * Route device#20,21's INTA/B/C/D to PIRQE/F/G/H
	 */
	writew(PIRQC, &rcba->rmu_ir);
	writew(PIRQA | (PIRQB << 4) | (PIRQC << 8) | (PIRQD << 12),
	       &rcba->d23_ir);
	writew(PIRQD, &rcba->core_ir);
	writew(PIRQE | (PIRQF << 4) | (PIRQG << 8) | (PIRQH << 12),
	       &rcba->d20d21_ir);
}

int arch_misc_init(void)
{
	return pirq_init();
}
