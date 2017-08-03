/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <sata.h>
#include <ahci.h>
#include <scsi.h>
#include <malloc.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <zynqmppl.h>
#include <i2c.h>
#include <g_dnl.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
    !defined(CONFIG_SPL_BUILD)
static xilinx_desc zynqmppl = XILINX_ZYNQMP_DESC;

static const struct {
	uint32_t id;
	char *name;
} zynqmp_devices[] = {
	{
		.id = 0x10,
		.name = "3eg",
	},
	{
		.id = 0x11,
		.name = "2eg",
	},
	{
		.id = 0x20,
		.name = "5ev",
	},
	{
		.id = 0x21,
		.name = "4ev",
	},
	{
		.id = 0x30,
		.name = "7ev",
	},
	{
		.id = 0x38,
		.name = "9eg",
	},
	{
		.id = 0x39,
		.name = "6eg",
	},
	{
		.id = 0x40,
		.name = "11eg",
	},
	{
		.id = 0x50,
		.name = "15eg",
	},
	{
		.id = 0x58,
		.name = "19eg",
	},
	{
		.id = 0x59,
		.name = "17eg",
	},
};
#endif

int chip_id(unsigned char id)
{
	struct pt_regs regs;
	int val = -EINVAL;

	if (current_el() != 3) {
		regs.regs[0] = ZYNQMP_SIP_SVC_CSU_DMA_CHIPID;
		regs.regs[1] = 0;
		regs.regs[2] = 0;
		regs.regs[3] = 0;

		smc_call(&regs);

		/*
		 * SMC returns:
		 * regs[0][31:0]  = status of the operation
		 * regs[0][63:32] = CSU.IDCODE register
		 * regs[1][31:0]  = CSU.version register
		 */
		switch (id) {
		case IDCODE:
			regs.regs[0] = upper_32_bits(regs.regs[0]);
			regs.regs[0] &= ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK |
					ZYNQMP_CSU_IDCODE_SVD_MASK;
			regs.regs[0] >>= ZYNQMP_CSU_IDCODE_SVD_SHIFT;
			val = regs.regs[0];
			break;
		case VERSION:
			regs.regs[1] = lower_32_bits(regs.regs[1]);
			regs.regs[1] &= ZYNQMP_CSU_SILICON_VER_MASK;
			val = regs.regs[1];
			break;
		default:
			printf("%s, Invalid Req:0x%x\n", __func__, id);
		}
	} else {
		switch (id) {
		case IDCODE:
			val = readl(ZYNQMP_CSU_IDCODE_ADDR);
			val &= ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK |
			       ZYNQMP_CSU_IDCODE_SVD_MASK;
			val >>= ZYNQMP_CSU_IDCODE_SVD_SHIFT;
			break;
		case VERSION:
			val = readl(ZYNQMP_CSU_VER_ADDR);
			val &= ZYNQMP_CSU_SILICON_VER_MASK;
			break;
		default:
			printf("%s, Invalid Req:0x%x\n", __func__, id);
		}
	}

	return val;
}

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
	!defined(CONFIG_SPL_BUILD)
static char *zynqmp_get_silicon_idcode_name(void)
{
	uint32_t i, id;

	id = chip_id(IDCODE);
	for (i = 0; i < ARRAY_SIZE(zynqmp_devices); i++) {
		if (zynqmp_devices[i].id == id)
			return zynqmp_devices[i].name;
	}
	return "unknown";
}
#endif

int board_early_init_f(void)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_CLK_ZYNQMP)
	zynqmp_pmufw_version();
#endif

#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_ZYNQMP_PSU_INIT_ENABLED)
	psu_init();
#endif

	return 0;
}

#define ZYNQMP_VERSION_SIZE	9

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
    !defined(CONFIG_SPL_BUILD) || (defined(CONFIG_SPL_FPGA_SUPPORT) && \
    defined(CONFIG_SPL_BUILD))
	if (current_el() != 3) {
		static char version[ZYNQMP_VERSION_SIZE];

		strncat(version, "xczu", 4);
		zynqmppl.name = strncat(version,
					zynqmp_get_silicon_idcode_name(),
					ZYNQMP_VERSION_SIZE - 5);
		printf("Chip ID:\t%s\n", zynqmppl.name);
		fpga_init();
		fpga_add(fpga_xilinx, &zynqmppl);
	}
#endif

	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val &= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;

	if (current_el() == 3 && !val) {
		val = readl(&crlapb_base->timestamp_ref_ctrl);
		val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
		writel(val, &crlapb_base->timestamp_ref_ctrl);

		/* Program freq register in System counter */
		writel(zynqmp_get_system_timer_freq(),
		       &iou_scntr_secure->base_frequency_id_register);
		/* And enable system counter */
		writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
		       &iou_scntr_secure->counter_control_register);
	}
	return 0;
}

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
#if defined(CONFIG_ZYNQ_GEM_EEPROM_ADDR) && \
    defined(CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET) && \
    defined(CONFIG_ZYNQ_EEPROM_BUS)
	i2c_set_bus_num(CONFIG_ZYNQ_EEPROM_BUS);

	if (eeprom_read(CONFIG_ZYNQ_GEM_EEPROM_ADDR,
			CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET,
			ethaddr, 6))
		printf("I2C EEPROM MAC address read failed\n");
#endif

	return 0;
}

#if !defined(CONFIG_SYS_SDRAM_BASE) && !defined(CONFIG_SYS_SDRAM_SIZE)
int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_memory_size() != 0)
		return -EINVAL;

	return 0;
}
#else
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}
#endif

void reset_cpu(ulong addr)
{
}

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;
	const char *mode;
	char *new_targets;

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	reg = readl(&crlapb_base->boot_mode);
	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "usb";
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "pxe dhcp";
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		mode = "qspi0";
		puts("QSPI_MODE\n");
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		mode = "mmc0";
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		mode = "mmc0";
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		/* fall through */
	case SD_MODE1:
		puts("SD_MODE1\n");
#if defined(CONFIG_ZYNQ_SDHCI0) && defined(CONFIG_ZYNQ_SDHCI1)
		mode = "mmc1";
#else
		mode = "mmc0";
#endif
		break;
	case NAND_MODE:
		puts("NAND_MODE\n");
		mode = "nand0";
		break;
	default:
		mode = "";
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	new_targets = calloc(1, strlen(mode) +
				strlen(env_get("boot_targets")) + 2);

	sprintf(new_targets, "%s %s", mode, env_get("boot_targets"));
	env_set("boot_targets", new_targets);

	return 0;
}

int checkboard(void)
{
	puts("Board: Xilinx ZynqMP\n");
	return 0;
}

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data0 = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = ZYNQMP_USB0_XHCI_BASEADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
};

static struct dwc3_device dwc3_device_data1 = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = ZYNQMP_USB1_XHCI_BASEADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 1,
};

int usb_gadget_handle_interrupts(int index)
{
	dwc3_uboot_handle_interrupt(index);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	debug("%s: index %x\n", __func__, index);

#if defined(CONFIG_USB_GADGET_DOWNLOAD)
	g_dnl_set_serialnumber(CONFIG_SYS_CONFIG_NAME);
#endif

	switch (index) {
	case 0:
		return dwc3_uboot_init(&dwc3_device_data0);
	case 1:
		return dwc3_uboot_init(&dwc3_device_data1);
	};

	return -1;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	dwc3_uboot_exit(index);
	return 0;
}
#endif
