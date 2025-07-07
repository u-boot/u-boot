// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022 - 2025, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <cpu_func.h>
#include <fdtdec.h>
#include <init.h>
#include <env_internal.h>
#include <log.h>
#include <malloc.h>
#include <time.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <versalpl.h>
#include "../../xilinx/common/board.h"

#include <linux/bitfield.h>
#include <debug_uart.h>
#include <generated/dt.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA_VERSALPL)
static xilinx_desc versalpl = {
	xilinx_versal2, csu_dma, 1, &versal_op, 0, &versal_op, NULL,
	FPGA_LEGACY
};
#endif

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

#if defined(CONFIG_FPGA_VERSALPL)
	fpga_init();
	fpga_add(fpga_xilinx, &versalpl);
#endif

	if (CONFIG_IS_ENABLED(DM_I2C) && CONFIG_IS_ENABLED(I2C_EEPROM))
		xilinx_read_eeprom();

	return 0;
}

static u32 platform_id, platform_version;

char *soc_name_decode(void)
{
	char *name, *platform_name;

	switch (platform_id) {
	case VERSAL2_SPP:
		platform_name = "spp";
		break;
	case VERSAL2_EMU:
		platform_name = "emu";
		break;
	case VERSAL2_SPP_MMD:
		platform_name = "spp-mmd";
		break;
	case VERSAL2_EMU_MMD:
		platform_name = "emu-mmd";
		break;
	case VERSAL2_QEMU:
		platform_name = "qemu";
		break;
	default:
		return NULL;
	}

	/*
	 * --rev. are 6 chars
	 * max platform name is qemu which is 4 chars
	 * platform version number are 1+1
	 * Plus 1 char for \n
	 */
	name = calloc(1, strlen(CONFIG_SYS_BOARD) + 13);
	if (!name)
		return NULL;

	sprintf(name, "%s-%s-rev%d.%d-el%d", CONFIG_SYS_BOARD,
		platform_name, platform_version / 10,
		platform_version % 10, current_el());

	return name;
}

bool soc_detection(void)
{
	u32 version, ps_version;

	version = readl(PMC_TAP_VERSION);
	platform_id = FIELD_GET(PLATFORM_MASK, version);
	ps_version = FIELD_GET(PS_VERSION_MASK, version);

	debug("idcode %x, version %x, usercode %x\n",
	      readl(PMC_TAP_IDCODE), version,
	      readl(PMC_TAP_USERCODE));

	debug("pmc_ver %lx, ps version %x, rtl version %lx\n",
	      FIELD_GET(PMC_VERSION_MASK, version),
	      ps_version,
	      FIELD_GET(RTL_VERSION_MASK, version));

	platform_version = FIELD_GET(PLATFORM_VERSION_MASK, version);

	debug("Platform id: %d version: %d.%d\n", platform_id,
	      platform_version / 10, platform_version % 10);

	return true;
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
	writel(CONFIG_COUNTER_FREQUENCY,
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

static u8 versal2_get_bootmode(void)
{
	u8 bootmode;
	u32 reg = 0;

	reg = readl(&crp_base->boot_mode_usr);

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	return bootmode;
}

static int boot_targets_setup(void)
{
	u8 bootmode;
	struct udevice *dev;
	int bootseq = -1;
	int bootseq_len = 0;
	int env_targets_len = 0;
	const char *mode = NULL;
	char *new_targets;
	char *env_targets;

	bootmode = versal2_get_bootmode();

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "usb_dfu0 usb_dfu1";
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "jtag pxe dhcp";
		break;
	case QSPI_MODE_24BIT:
		puts("QSPI_MODE_24\n");
		if (uclass_get_device_by_name(UCLASS_SPI,
					      "spi@f1030000", &dev)) {
			debug("QSPI driver for QSPI device is not present\n");
			break;
		}
		mode = "xspi";
		bootseq = dev_seq(dev);
		break;
	case QSPI_MODE_32BIT:
		puts("QSPI_MODE_32\n");
		if (uclass_get_device_by_name(UCLASS_SPI,
					      "spi@f1030000", &dev)) {
			debug("QSPI driver for QSPI device is not present\n");
			break;
		}
		mode = "xspi";
		bootseq = dev_seq(dev);
		break;
	case OSPI_MODE:
		puts("OSPI_MODE\n");
		if (uclass_get_device_by_name(UCLASS_SPI,
					      "spi@f1010000", &dev)) {
			debug("OSPI driver for OSPI device is not present\n");
			break;
		}
		mode = "xspi";
		bootseq = dev_seq(dev);
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		mode = "mmc";
		bootseq = dev_seq(dev);
		break;
	case SELECTMAP_MODE:
		puts("SELECTMAP_MODE\n");
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@f1040000", &dev)) {
			debug("SD0 driver for SD0 device is not present\n");
			break;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		fallthrough;
	case SD_MODE1:
		puts("SD_MODE1\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@f1050000", &dev)) {
			debug("SD1 driver for SD1 device is not present\n");
			break;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		break;
	case UFS_MODE:
		puts("UFS_MODE\n");
		if (uclass_get_device(UCLASS_UFS, 0, &dev)) {
			debug("UFS driver for UFS device is not present\n");
			break;
		}
		debug("ufs device found at %p\n", dev);

		mode = "ufs";
		break;
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	if (mode) {
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
		free(new_targets);
	}

	return 0;
}

int board_late_init(void)
{
	int ret;

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	if (!IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG))
		return 0;

	if (IS_ENABLED(CONFIG_DISTRO_DEFAULTS)) {
		ret = boot_targets_setup();
		if (ret)
			return ret;
	}

	return board_late_init_xilinx();
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
	int ret;

	if (IS_ENABLED(CONFIG_SYS_MEM_RSVD_FOR_MMU))
		ret = fdtdec_setup_mem_size_base();
	else
		ret = fdtdec_setup_mem_size_base_lowest();

	if (ret)
		return -EINVAL;

	return 0;
}

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
}
#endif

#if defined(CONFIG_ENV_IS_NOWHERE)
enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = versal2_get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode) {
	case EMMC_MODE:
	case SD_MODE:
	case SD1_LSHFT_MODE:
	case SD_MODE1:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_FAT))
			return ENVL_FAT;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		return ENVL_NOWHERE;
	case OSPI_MODE:
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		return ENVL_NOWHERE;
	case JTAG_MODE:
	case SELECTMAP_MODE:
	default:
		return ENVL_NOWHERE;
	}
}
#endif
