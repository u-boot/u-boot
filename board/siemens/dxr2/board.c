/*
 * Board functions for TI AM335X based dxr2 board
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 *
 * Board functions for TI AM335X based boards
 * u-boot:/board/ti/am335x/board.c
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <watchdog.h>
#include "board.h"
#include "../common/factoryset.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
static struct dxr2_baseboard_id __attribute__((section(".data"))) settings;
/* @303MHz-i0 */
const struct ddr3_data ddr3_default = {
	0x33524444, 0x56312e34, 0x0080, 0x0000, 0x0038, 0x003E, 0x00A4,
	0x0075, 0x0888A39B, 0x26247FDA, 0x501F821F, 0x00100206, 0x61A44A32,
	0x00000618, 0x0000014A,
};

static void set_default_ddr3_timings(void)
{
	printf("Set default DDR3 settings\n");
	settings.ddr3 = ddr3_default;
}

static void print_ddr3_timings(void)
{
	printf("\n\nDDR3 Timing parameters:\n");
	printf("Diff     Eeprom  Default\n");
	PRINTARGS(magic);
	PRINTARGS(version);
	PRINTARGS(ddr3_sratio);
	PRINTARGS(iclkout);

	PRINTARGS(dt0rdsratio0);
	PRINTARGS(dt0wdsratio0);
	PRINTARGS(dt0fwsratio0);
	PRINTARGS(dt0wrsratio0);

	PRINTARGS(sdram_tim1);
	PRINTARGS(sdram_tim2);
	PRINTARGS(sdram_tim3);

	PRINTARGS(emif_ddr_phy_ctlr_1);

	PRINTARGS(sdram_config);
	PRINTARGS(ref_ctrl);
	PRINTARGS(ioctr_val);
}

static void print_chip_data(void)
{
	printf("\n");
	printf("Device: '%s'\n", settings.chip.sdevname);
	printf("HW version: '%s'\n", settings.chip.shwver);
}
#endif /* CONFIG_SPL_BUILD */

/*
 * Read header information from EEPROM into global structure.
 */
static int read_eeprom(void)
{
	/* Check if baseboard eeprom is available */
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR)) {
		printf("Could not probe the EEPROM; something fundamentally wrong on the I2C bus.\n");
		return 1;
	}

#ifdef CONFIG_SPL_BUILD
	/* Read Siemens eeprom data (DDR3) */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, EEPROM_ADDR_DDR3, 2,
		     (uchar *)&settings.ddr3, sizeof(struct ddr3_data))) {
		printf("Could not read the EEPROM; something fundamentally wrong on the I2C bus.\nUse default DDR3 timings\n");
		set_default_ddr3_timings();
	}
	/* Read Siemens eeprom data (CHIP) */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, EEPROM_ADDR_CHIP, 2,
		     (uchar *)&settings.chip, sizeof(settings.chip)))
		printf("Could not read chip settings\n");

	if (ddr3_default.magic == settings.ddr3.magic &&
	    ddr3_default.version == settings.ddr3.version) {
		printf("Using DDR3 settings from EEPROM\n");
	} else {
		if (ddr3_default.magic != settings.ddr3.magic)
			printf("Error: No valid DDR3 data in eeprom.\n");
		if (ddr3_default.version != settings.ddr3.version)
			printf("Error: DDR3 data version does not match.\n");

		printf("Using default settings\n");
		set_default_ddr3_timings();
	}

	if (MAGIC_CHIP == settings.chip.magic) {
		printf("Valid chip data in eeprom\n");
		print_chip_data();
	} else {
		printf("Error: No chip data in eeprom\n");
	}

	print_ddr3_timings();
#endif
	return 0;
}

#ifdef CONFIG_SPL_BUILD
static void board_init_ddr(void)
{
struct emif_regs dxr2_ddr3_emif_reg_data = {
	.zq_config = 0x50074BE4,
};

struct ddr_data dxr2_ddr3_data = {
};

struct cmd_control dxr2_ddr3_cmd_ctrl_data = {
};

struct ctrl_ioregs dxr2_ddr3_ioregs = {
};

	/* pass values from eeprom */
	dxr2_ddr3_emif_reg_data.sdram_tim1 = settings.ddr3.sdram_tim1;
	dxr2_ddr3_emif_reg_data.sdram_tim2 = settings.ddr3.sdram_tim2;
	dxr2_ddr3_emif_reg_data.sdram_tim3 = settings.ddr3.sdram_tim3;
	dxr2_ddr3_emif_reg_data.emif_ddr_phy_ctlr_1 =
		settings.ddr3.emif_ddr_phy_ctlr_1;
	dxr2_ddr3_emif_reg_data.sdram_config = settings.ddr3.sdram_config;
	dxr2_ddr3_emif_reg_data.ref_ctrl = settings.ddr3.ref_ctrl;

	dxr2_ddr3_data.datardsratio0 = settings.ddr3.dt0rdsratio0;
	dxr2_ddr3_data.datawdsratio0 = settings.ddr3.dt0wdsratio0;
	dxr2_ddr3_data.datafwsratio0 = settings.ddr3.dt0fwsratio0;
	dxr2_ddr3_data.datawrsratio0 = settings.ddr3.dt0wrsratio0;

	dxr2_ddr3_cmd_ctrl_data.cmd0csratio = settings.ddr3.ddr3_sratio;
	dxr2_ddr3_cmd_ctrl_data.cmd0iclkout = settings.ddr3.iclkout;
	dxr2_ddr3_cmd_ctrl_data.cmd1csratio = settings.ddr3.ddr3_sratio;
	dxr2_ddr3_cmd_ctrl_data.cmd1iclkout = settings.ddr3.iclkout;
	dxr2_ddr3_cmd_ctrl_data.cmd2csratio = settings.ddr3.ddr3_sratio;
	dxr2_ddr3_cmd_ctrl_data.cmd2iclkout = settings.ddr3.iclkout;

	dxr2_ddr3_ioregs.cm0ioctl = settings.ddr3.ioctr_val,
	dxr2_ddr3_ioregs.cm1ioctl = settings.ddr3.ioctr_val,
	dxr2_ddr3_ioregs.cm2ioctl = settings.ddr3.ioctr_val,
	dxr2_ddr3_ioregs.dt0ioctl = settings.ddr3.ioctr_val,
	dxr2_ddr3_ioregs.dt1ioctl = settings.ddr3.ioctr_val,

	config_ddr(DDR_PLL_FREQ, &dxr2_ddr3_ioregs, &dxr2_ddr3_data,
		   &dxr2_ddr3_cmd_ctrl_data, &dxr2_ddr3_emif_reg_data, 0);
}

static void spl_siemens_board_init(void)
{
	return;
}
#endif /* if def CONFIG_SPL_BUILD */

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_id		= 0,
		.phy_if		= PHY_INTERFACE_MODE_MII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 4,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

#if defined(CONFIG_DRIVER_TI_CPSW) || \
	(defined(CONFIG_USB_ETHER) && defined(CONFIG_MUSB_GADGET))
int board_eth_init(bd_t *bis)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
	int n = 0;
	int rv;

	factoryset_setenv();

	/* Set rgmii mode and enable rmii clock to be sourced from chip */
	writel((RMII_MODE_ENABLE | RMII_CHIPCKL_ENABLE), &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
	return n;
}
#endif /* #if defined(CONFIG_DRIVER_TI_CPSW) */
#endif /* #if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) */

#include "../common/board.c"
