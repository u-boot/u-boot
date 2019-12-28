// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <fdtdec.h>
#include <init.h>
#include <malloc.h>
#include <time.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <versalpl.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA_VERSALPL)
static xilinx_desc versalpl = XILINX_VERSAL_DESC;
#endif

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

#if defined(CONFIG_FPGA_VERSALPL)
	fpga_init();
	fpga_add(fpga_xilinx, &versalpl);
#endif

	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	if (current_el() != 3)
		return 0;

	debug("iou_switch ctrl div0 %x\n",
	      readl(&crlapb_base->iou_switch_ctrl));

	writel(IOU_SWITCH_CTRL_CLKACT_BIT |
	       (CONFIG_IOU_SWITCH_DIVISOR0 << IOU_SWITCH_CTRL_DIVISOR0_SHIFT),
	       &crlapb_base->iou_switch_ctrl);

	/* Global timer init - Program time stamp reference clk */
	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val |= CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT;
	writel(val, &crlapb_base->timestamp_ref_ctrl);

	debug("ref ctrl 0x%x\n",
	      readl(&crlapb_base->timestamp_ref_ctrl));

	/* Clear reset of timestamp reg */
	writel(0, &crlapb_base->rst_timestamp);

	/*
	 * Program freq register in System counter and
	 * enable system counter.
	 */
	writel(COUNTER_FREQUENCY,
	       &iou_scntr_secure->base_frequency_id_register);

	debug("counter val 0x%x\n",
	      readl(&iou_scntr_secure->base_frequency_id_register));

	writel(IOU_SCNTRS_CONTROL_EN,
	       &iou_scntr_secure->counter_control_register);

	debug("scntrs control 0x%x\n",
	      readl(&iou_scntr_secure->counter_control_register));
	debug("timer 0x%llx\n", get_ticks());
	debug("timer 0x%llx\n", get_ticks());

	return 0;
}

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;
	struct udevice *dev;
	int bootseq = -1;
	int bootseq_len = 0;
	int env_targets_len = 0;
	const char *mode;
	char *new_targets;
	char *env_targets;
	ulong initrd_hi;

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	reg = readl(&crp_base->boot_mode_usr);

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "dfu_usb";
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "jtag pxe dhcp";
		break;
	case QSPI_MODE_24BIT:
		puts("QSPI_MODE_24\n");
		mode = "xspi0";
		break;
	case QSPI_MODE_32BIT:
		puts("QSPI_MODE_32\n");
		mode = "xspi0";
		break;
	case OSPI_MODE:
		puts("OSPI_MODE\n");
		mode = "xspi0";
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@f1050000", &dev)) {
			puts("Boot from EMMC but without SD1 enabled!\n");
			return -1;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev->seq);
		mode = "mmc";
		bootseq = dev->seq;
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@f1040000", &dev)) {
			puts("Boot from SD0 but without SD0 enabled!\n");
			return -1;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev->seq);

		mode = "mmc";
		bootseq = dev->seq;
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		/* fall through */
	case SD_MODE1:
		puts("SD_MODE1\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@f1050000", &dev)) {
			puts("Boot from SD1 but without SD1 enabled!\n");
			return -1;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev->seq);

		mode = "mmc";
		bootseq = dev->seq;
		break;
	default:
		mode = "";
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	if (bootseq >= 0) {
		bootseq_len = snprintf(NULL, 0, "%i", bootseq);
		debug("Bootseq len: %x\n", bootseq_len);
	}

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	env_targets = env_get("boot_targets");
	if (env_targets)
		env_targets_len = strlen(env_targets);

	new_targets = calloc(1, strlen(mode) + env_targets_len + 2 +
			     bootseq_len);
	if (!new_targets)
		return -ENOMEM;

	if (bootseq >= 0)
		sprintf(new_targets, "%s%x %s", mode, bootseq,
			env_targets ? env_targets : "");
	else
		sprintf(new_targets, "%s %s", mode,
			env_targets ? env_targets : "");

	env_set("boot_targets", new_targets);

	initrd_hi = gd->start_addr_sp - CONFIG_STACK_SIZE;
	initrd_hi = round_down(initrd_hi, SZ_16M);
	env_set_addr("initrd_high", (void *)initrd_hi);

	env_set_hex("script_offset_f", CONFIG_BOOT_SCRIPT_OFFSET);

	return 0;
}

int dram_init_banksize(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	mem_map_fill();

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

void reset_cpu(ulong addr)
{
}
