// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <cpu_func.h>
#include <fdtdec.h>
#include <init.h>
#include <env_internal.h>
#include <log.h>
#include <malloc.h>
#include <mmc.h>
#include <spi.h>
#include <time.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <versalpl.h>
#include "../common/board.h"

#include <debug_uart.h>
#include <generated/dt.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA_VERSALPL)
static xilinx_desc versalpl = {
	xilinx_versal_net, csu_dma, 1, &versal_op, 0, &versal_op, NULL,
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
	return 0;
}

int board_early_init_f(void)
{
	if (IS_ENABLED(CONFIG_DEBUG_UART)) {
		/* Uart debug for sure */
		debug_uart_init();
		puts("Debug uart enabled\n"); /* or printch() */
	}

	return 0;
}

int board_early_init_r(void)
{
	if (current_el() != 3)
		return 0;

	versal_net_timer_setup();

	return 0;
}

static int spi_get_bootseq(u8 bootmode, const char **modename)
{
	struct udevice *dev;
	const char *name;
	int bootseq;

	switch (bootmode) {
	case QSPI_MODE_24BIT:
		if (modename)
			*modename = "QSPI_MODE_24\n";
		name = "spi@f1030000";
		break;
	case QSPI_MODE_32BIT:
		if (modename)
			*modename = "QSPI_MODE_32\n";
		name = "spi@f1030000";
		break;
	case OSPI_MODE:
		if (modename)
			*modename = "OSPI_MODE\n";
		name = "spi@f1010000";
		break;
	default:
		return -1;
	}

	if (uclass_get_device_by_name(UCLASS_SPI, name, &dev)) {
		debug("SPI driver for %s is not present\n", name);
		return -1;
	}

	bootseq = dev_seq(dev);
	debug("bootseq %d\n", bootseq);

	return bootseq;
}

int spi_get_env_dev(void)
{
	return spi_get_bootseq(versal_net_get_bootmode(), NULL);
}

static int mmc_get_bootseq(u8 bootmode, const char **modename)
{
	struct udevice *dev;
	const char *name;

	switch (bootmode) {
	case SD_MODE:
		if (modename)
			*modename = "SD_MODE\n";
		name = "mmc@f1040000";
		break;
	case EMMC_MODE:
		if (modename)
			*modename = "EMMC_MODE\n";
		name = "mmc@f1050000";
		break;
	case SD_MODE1:
	case SD1_LSHFT_MODE:
		if (modename)
			*modename = "SD_MODE1\n";
		name = "mmc@f1050000";
		break;
	default:
		return -1;
	}

	if (uclass_get_device_by_name(UCLASS_MMC, name, &dev)) {
		debug("MMC driver for %s is not present\n", name);
		return -1;
	}

	return dev_seq(dev);
}

int mmc_get_env_dev(void)
{
	return mmc_get_bootseq(versal_net_get_bootmode(), NULL);
}

static int boot_targets_setup(void)
{
	u8 bootmode;
	int bootseq = -1;
	int bootseq_len = 0;
	int env_targets_len = 0;
	const char *mode = NULL;
	const char *modename = NULL;
	char *new_targets;
	char *env_targets;

	bootmode = versal_net_get_bootmode();

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
	case QSPI_MODE_32BIT:
	case OSPI_MODE:
		bootseq = spi_get_bootseq(bootmode, &modename);
		if (modename)
			puts(modename);
		if (bootseq >= 0)
			mode = "xspi";
		break;
	case SELECTMAP_MODE:
		puts("SELECTMAP_MODE\n");
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		fallthrough;
	case SD_MODE:
	case EMMC_MODE:
	case SD_MODE1:
		bootseq = mmc_get_bootseq(bootmode, &modename);
		if (modename)
			puts(modename);
		if (bootseq >= 0)
			mode = "mmc";
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

void reset_cpu(void)
{
}

#if defined(CONFIG_ENV_IS_NOWHERE)
enum env_location env_get_location(enum env_operation op, int prio)
{
	u8 bootmode = versal_net_get_bootmode();

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
