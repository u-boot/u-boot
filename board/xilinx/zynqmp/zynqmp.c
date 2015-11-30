/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <ahci.h>
#include <scsi.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <usb.h>
#include <dwc3-uboot.h>

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

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;

	reg = readl(&crlapb_base->boot_mode);
	bootmode = reg & BOOT_MODES_MASK;

	switch (bootmode) {
	case SD_MODE:
	case EMMC_MODE:
		setenv("modeboot", "sdboot");
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
