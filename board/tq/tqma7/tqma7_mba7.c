// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Steffen Doster
 */

#include <env.h>
#include <errno.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-mx7/imx-regs.h>
#include <asm/mach-imx/boot_mode.h>

#include "../common/tq_bb.h"

const char *tq_bb_get_boardname(void)
{
	return "MBa7x";
}

#if !IS_ENABLED(CONFIG_SPL_BUILD)

static int mba7_setup_fec(int fec_id)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;
	int ret;

	switch (fec_id) {
	case 0:
		/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
				IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
				IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK, 0);
		break;
	case 1:
		/* Use 125M anatop REF_CLK2 for ENET2, clear gpr1[14], gpr1[18]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
				IOMUXC_GPR_GPR1_GPR_ENET2_TX_CLK_SEL_MASK |
				IOMUXC_GPR_GPR1_GPR_ENET2_CLK_DIR_MASK, 0);
		break;
	default:
		printf("FEC%d: unsupported\n", fec_id);
		return -EINVAL;
	}

	ret = set_clk_enet(ENET_125MHZ);
	if (ret)
		return ret;

	return 0;
}

int tq_bb_board_init(void)
{
	mba7_setup_fec(0);

	if (!is_cpu_type(MXC_CPU_MX7S))
		mba7_setup_fec(1);

	return 0;
}

int tq_bb_board_late_init(void)
{
	puts("Boot:  ");

	if (is_boot_from_usb()) {
		puts("USB\n");
		env_set_runtime("boot_dev", "mmc");
		env_set_runtime("mmcdev", "0");
		env_set_runtime("mmcblkdev", "0");
	} else {
		/*
		 * try to get sd card slots in order:
		 * eMMC: on Module
		 * -> therefore index 0 for bootloader
		 * index n in kernel (controller instance 3) -> patches needed for
		 * alias indexing
		 * SD1: on Mainboard
		 * index n in kernel (controller instance 1) -> patches needed for
		 * alias indexing
		 * we assume to have a kernel patch that will present mmcblk dev
		 * indexed like controller devs
		 */
		enum boot_device bd = get_boot_device();

		switch (bd) {
		case MMC3_BOOT:
			puts("USDHC3(eMMC)\n");
			env_set_runtime("boot_dev", "mmc");
			env_set_runtime("mmcdev", "0");
			env_set_runtime("mmcblkdev", "0");
			break;
		case SD1_BOOT:
			puts("USDHC1(SD)\n");
			env_set_runtime("boot_dev", "mmc");
			env_set_runtime("mmcdev", "1");
			env_set_runtime("mmcblkdev", "1");
			break;
		case QSPI_BOOT:
			puts("QSPI\n");
			env_set_runtime("boot_dev", "qspi");
			env_set_runtime("mmcdev", "0");
			env_set_runtime("mmcblkdev", "0");
			break;
		default:
			printf("unhandled boot device %d\n", (int)bd);
			env_set_runtime("mmcdev", "0");
			env_set_runtime("mmcblkdev", "0");
		}
	}

	if (!env_get("fdtfile")) {
		/* provide default setting for fdtfile if nothing in env is set */

		switch (get_cpu_type()) {
		case MXC_CPU_MX7S:
			env_set_runtime("fdtfile", "imx7s-mba7.dtb");
			break;
		case MXC_CPU_MX7D:
			env_set_runtime("fdtfile", "imx7d-mba7.dtb");
			break;
		default:
			debug("unknown CPU");
		}
	}

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	switch (devno) {
	case 2:
		/* eMMC */
		return 0;
	case 0:
		/* SD card */
		return 1;
	default:
		/* Unknown */
		return 0;
	}
}

#endif /* !IS_ENABLED(CONFIG_SPL_BUILD) */
