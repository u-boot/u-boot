// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Kontron Electronics GmbH
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <dm/uclass.h>
#include <efi.h>
#include <efi_loader.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <mmc.h>
#include <net.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/arch/ccm_regs.h>

#include "../common/hw-uid.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return 0;
}

#if IS_ENABLED(CONFIG_KONTRON_HW_UID)
struct uid_otp_loc uid_otp_locations[] = {
	{
		.addr = (u32 *)(FSB_BASE_ADDR + 0x8000 + 0x5e8),
		.len = 2,
		.format = UID_OTP_FORMAT_DEC,
		.desc = "BOARD"
	},
	{
		.addr = (u32 *)(FSB_BASE_ADDR + 0x8000 + 0x5e0),
		.len = 2,
		.format = UID_OTP_FORMAT_DEC,
		.desc = "SOM"
	}
};
#endif /* CONFIG_KONTRON_HW_UID */

int ft_board_setup(void *blob, struct bd_info *bd)
{
	enum boot_device boot_dev;
	char env_str_sd[] = "sd-card";
	char env_str_emmc[] = "emmc";
	char *env_config_str;

	if (env_get_location(0, 0) != ENVL_MMC)
		return 0;

	boot_dev = get_boot_device();
	if (boot_dev == SD2_BOOT)
		env_config_str = env_str_sd;
	else if (boot_dev == MMC1_BOOT)
		env_config_str = env_str_emmc;
	else
		return 0;

	/*
	 * Export a string to the devicetree that tells userspace tools like
	 * libubootenv where the environment is currently coming from.
	 */
	return fdt_find_and_setprop(blob, "/chosen", "u-boot,env-config",
				    env_config_str, strlen(env_config_str) + 1, 1);
}

static int setup_eqos(void)
{
	struct blk_ctrl_wakeupmix_regs *bctrl =
		(struct blk_ctrl_wakeupmix_regs *)BLK_CTRL_WAKEUPMIX_BASE_ADDR;

	/* set INTF as RGMII, enable RGMII TXC clock */
	clrsetbits_le32(&bctrl->eqos_gpr,
			BCTRL_GPR_ENET_QOS_INTF_MODE_MASK,
			BCTRL_GPR_ENET_QOS_INTF_SEL_RGMII | BCTRL_GPR_ENET_QOS_CLK_GEN_EN);

	return set_clk_eqos(ENET_125MHZ);
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_DWC_ETH_QOS))
		setup_eqos();

	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_KONTRON_HW_UID))
		get_serial_number(uid_otp_locations, ARRAY_SIZE(uid_otp_locations));

	if (get_boot_device() == USB_BOOT) {
		env_set("bootcmd", "fastboot 0");
		env_set("bootdelay", "0");
	}

	return 0;
}

#if IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
int board_mmc_get_env_dev(int devno)
{
	return devno;
}

uint mmc_get_env_part(struct mmc *mmc)
{
	if (IS_SD(mmc))
		return EMMC_HWPART_DEFAULT;

	switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
	case EMMC_BOOT_PART_BOOT1:
		return EMMC_HWPART_BOOT1;
	case EMMC_BOOT_PART_BOOT2:
		return EMMC_HWPART_BOOT2;
	default:
		return EMMC_HWPART_DEFAULT;
	}
}

int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	/* use normal offset for SD card */
	if (IS_SD(mmc)) {
		*env_addr = CONFIG_ENV_OFFSET;
		if (copy)
			*env_addr = CONFIG_ENV_OFFSET_REDUND;

		return 0;
	}

	switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
	case EMMC_BOOT_PART_BOOT1:
	case EMMC_BOOT_PART_BOOT2:
		*env_addr = mmc->capacity - CONFIG_ENV_SIZE - CONFIG_ENV_SIZE;
		if (copy)
			*env_addr = mmc->capacity - CONFIG_ENV_SIZE;
	break;
	default:
		*env_addr = CONFIG_ENV_OFFSET;
		if (copy)
			*env_addr = CONFIG_ENV_OFFSET_REDUND;
	}

	return 0;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio)
		return ENVL_UNKNOWN;

	if (CONFIG_IS_ENABLED(ENV_IS_NOWHERE) && get_boot_device() == USB_BOOT)
		return ENVL_NOWHERE;

	return arch_env_get_location(op, prio);
}
