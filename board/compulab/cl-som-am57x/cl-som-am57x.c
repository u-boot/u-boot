/*
 * Board functions for CompuLab cl_som_am57x board
 *
 * (C) Copyright 2016 CompuLab, Ltd. http://compulab.co.il/
 *
 * Author: Dmitry Lifshitz <lifshitz@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <palmas.h>
#include <usb.h>
#include <asm/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include "../common/common.h"
#include "../common/eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	"Board: CL-SOM-AM57x\n"
};

int board_init(void)
{
	/* Disable PMIC Powerhold feature, DEV_CTRL.DEV_ON = 1 */
	palmas_i2c_write_u8(TPS65903X_CHIP_P1, 0xA0, 0x1);

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#ifdef CONFIG_MMC
#define SB_SOM_CD_GPIO 187
#define SB_SOM_WP_GPIO 188

int board_mmc_init(bd_t *bis)
{
	int ret0, ret1;

	ret0 = omap_mmc_init(0, 0, 0, SB_SOM_CD_GPIO, SB_SOM_WP_GPIO);
	if (ret0)
		printf("cl-som-am57x: failed to initialize mmc0\n");

	ret1 = omap_mmc_init(1, 0, 0, -1, -1);
	if (ret1)
		printf("cl-som-am57x: failed to initialize mmc1\n");

	return ret0 && ret1;
}
#endif /* CONFIG_MMC */

int misc_init_r(void)
{
	cl_print_pcb_info();

	return 0;
}

u32 get_board_rev(void)
{
	return cl_eeprom_get_board_rev(CONFIG_SYS_I2C_EEPROM_BUS);
}
