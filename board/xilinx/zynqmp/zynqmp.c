/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dwc3-uboot.h>
#include <netdev.h>
#include <ahci.h>
#include <scsi.h>
#include <usb.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
	writel(val, &crlapb_base->timestamp_ref_ctrl);

	/* Program freq register in System counter and enable system counter */
	writel(gd->cpu_clk, &iou_scntr->base_frequency_id_register);
	writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_HDBG |
	       ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
	       &iou_scntr->counter_control_register);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int timer_init(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
}

#ifdef CONFIG_SCSI_AHCI_PLAT
void scsi_init(void)
{
	ahci_init((void __iomem *)ZYNQMP_SATA_BASEADDR);
	scsi_scan(1);
}
#endif

int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM0)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR0,
						CONFIG_ZYNQ_GEM_PHY_ADDR0, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM1)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR1,
						CONFIG_ZYNQ_GEM_PHY_ADDR1, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM2)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR2,
						CONFIG_ZYNQ_GEM_PHY_ADDR2, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM3)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR3,
						CONFIG_ZYNQ_GEM_PHY_ADDR3, 0);
# endif
#endif
	return ret;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	int ret = 0;
	u32 ver = zynqmp_get_silicon_version();

	if (ver != ZYNQMP_CSU_VERSION_VELOCE) {
#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
		ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
		ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif
	}

	return ret;
}
#endif

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;
	u32 ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		setenv("setup", "setenv baudrate 4800 && setenv bootcmd run veloce");
	case ZYNQMP_CSU_VERSION_EP108:
	case ZYNQMP_CSU_VERSION_SILICON:
		setenv("setup", "setenv serverip 10.10.70.101 && setenv ipaddr 10.10.71.100 && setenv partid auto");
		break;
	case ZYNQMP_CSU_VERSION_QEMU:
	default:
		setenv("setup", "setenv serverip 10.0.2.2 && setenv ipaddr 10.0.2.15 && setenv partid 0");
	}

	reg = readl(&crlapb_base->boot_mode);
	bootmode = reg & BOOT_MODES_MASK;

	puts("Bootmode: ");
	switch (bootmode) {
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		setenv("modeboot", "jtagboot");
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		setenv("modeboot", "qspiboot");
		puts("QSPI_MODE\n");
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		setenv("modeboot", "sdboot");
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		setenv("modeboot", "sdboot");
		break;
	case SD_MODE1:
		puts("SD_MODE1\n");
#if defined(CONFIG_ZYNQ_SDHCI0) && defined(CONFIG_ZYNQ_SDHCI1)
		setenv("sdbootdev", "1");
#endif
		setenv("modeboot", "sdboot");
		break;
	case NAND_MODE:
		puts("NAND_MODE\n");
		setenv("modeboot", "nandboot");
		break;
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	return 0;
}

int checkboard(void)
{
	puts("Board:\tXilinx ZynqMP\n");
	return 0;
}

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = ZYNQMP_USB0_XHCI_BASEADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return dwc3_uboot_init(&dwc3_device_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	dwc3_uboot_exit(index);
	return 0;
}
#endif
