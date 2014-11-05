/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <status_led.h>
#include <mmc.h>
#include <linux/compiler.h>

#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/am35x_def.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"CM-T3517 board",
	"NAND 128/512M",
};

int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set(STATUS_LED_BOOT, STATUS_LED_ON);
#endif

	return 0;
}

int misc_init_r(void)
{
	cl_print_pcb_info();
	dieid_num_r();

	return 0;
}

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
#define SB_T35_CD_GPIO 144
#define SB_T35_WP_GPIO 59

int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, SB_T35_CD_GPIO, SB_T35_WP_GPIO);
}
#endif
