/*
 * Copyright (C) 2015 Compulab, Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mux.h>
#include <asm/arch/ddr_defs.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/emif.h>
#include <power/pmic.h>
#include <power/tps65218.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#ifndef CONFIG_SKIP_LOWLEVEL_INIT

const struct dpll_params dpll_mpu  = { 800,  24, 1,  -1, -1, -1, -1 };
const struct dpll_params dpll_core = { 1000, 24, -1, -1, 10,  8,  4 };
const struct dpll_params dpll_per  = { 960,  24, 5,  -1, -1, -1, -1 };
const struct dpll_params dpll_ddr  = { 400,  23, 1,  -1,  1, -1, -1 };

const struct ctrl_ioregs ioregs_ddr3 = {
	.cm0ioctl		= DDR3_ADDRCTRL_IOCTRL_VALUE,
	.cm1ioctl		= DDR3_ADDRCTRL_WD0_IOCTRL_VALUE,
	.cm2ioctl		= DDR3_ADDRCTRL_WD1_IOCTRL_VALUE,
	.dt0ioctl		= DDR3_DATA0_IOCTRL_VALUE,
	.dt1ioctl		= DDR3_DATA0_IOCTRL_VALUE,
	.dt2ioctrl		= DDR3_DATA0_IOCTRL_VALUE,
	.dt3ioctrl		= DDR3_DATA0_IOCTRL_VALUE,
	.emif_sdram_config_ext	= 0x0143,
};

/* EMIF DDR3 Configurations are different for production AM43X GP EVMs */
struct emif_regs ddr3_emif_regs = {
	.sdram_config			= 0x638413B2,
	.ref_ctrl			= 0x00000C30,
	.sdram_tim1			= 0xEAAAD4DB,
	.sdram_tim2			= 0x266B7FDA,
	.sdram_tim3			= 0x107F8678,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x50074BE4,
	.temp_alert_config		= 0x0,
	.emif_ddr_phy_ctlr_1		= 0x0E004008,
	.emif_ddr_ext_phy_ctrl_1	= 0x08020080,
	.emif_ddr_ext_phy_ctrl_2	= 0x00000066,
	.emif_ddr_ext_phy_ctrl_3	= 0x00000091,
	.emif_ddr_ext_phy_ctrl_4	= 0x000000B9,
	.emif_ddr_ext_phy_ctrl_5	= 0x000000E6,
	.emif_rd_wr_exec_thresh		= 0x80000405,
	.emif_prio_class_serv_map	= 0x80000001,
	.emif_connect_id_serv_1_map	= 0x80000094,
	.emif_connect_id_serv_2_map	= 0x00000000,
	.emif_cos_config		= 0x000FFFFF
};

const u32 ext_phy_ctrl_const_base_ddr3[] = {
	0x00000000,
	0x00000044,
	0x00000044,
	0x00000046,
	0x00000046,
	0x00000000,
	0x00000059,
	0x00000077,
	0x00000093,
	0x000000A8,
	0x00000000,
	0x00000019,
	0x00000037,
	0x00000053,
	0x00000068,
	0x00000000,
	0x0,
	0x0,
	0x40000000,
	0x08102040
};

void emif_get_ext_phy_ctrl_const_regs(const u32 **regs, u32 *size)
{
	*regs = ext_phy_ctrl_const_base_ddr3;
	*size = ARRAY_SIZE(ext_phy_ctrl_const_base_ddr3);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

const struct dpll_params *get_dpll_mpu_params(void)
{
	return &dpll_mpu;
}

const struct dpll_params *get_dpll_core_params(void)
{
	return &dpll_core;
}

const struct dpll_params *get_dpll_per_params(void)
{
	return &dpll_per;
}

static void enable_vtt_regulator(void)
{
	u32 temp;

	writel(GPIO_CTRL_ENABLEMODULE, AM33XX_GPIO5_BASE + OMAP_GPIO_CTRL);
	writel(GPIO_SETDATAOUT(7), AM33XX_GPIO5_BASE + OMAP_GPIO_SETDATAOUT);
	temp = readl(AM33XX_GPIO5_BASE + OMAP_GPIO_OE);
	temp = temp & ~(GPIO_OE_ENABLE(7));
	writel(temp, AM33XX_GPIO5_BASE + OMAP_GPIO_OE);
}

void sdram_init(void)
{
	unsigned long ram_size;

	enable_vtt_regulator();
	config_ddr(0, &ioregs_ddr3, NULL, NULL, &ddr3_emif_regs, 0);
	ram_size = get_ram_size((long int *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (ram_size == 0x80000000 ||
	    ram_size == 0x40000000 ||
	    ram_size == 0x20000000)
		return;

	ddr3_emif_regs.sdram_config = 0x638453B2;
	config_ddr(0, &ioregs_ddr3, NULL, NULL, &ddr3_emif_regs, 0);
	ram_size = get_ram_size((long int *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (ram_size == 0x08000000)
		return;

	hang();
}
#endif

/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;

	power_tps65218_init(I2C_PMIC);
	p = pmic_get("TPS65218_PMIC");
	if (p && !pmic_probe(p))
		puts("PMIC:  TPS65218\n");

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gpmc_init();
	set_i2c_pin_mux();
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);
	i2c_probe(TPS65218_CHIP_PM);

	return 0;
}

#ifdef CONFIG_DRIVER_TI_CPSW

static void cpsw_control(int enabled)
{
	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
		.phy_if		= PHY_INTERFACE_MODE_RGMII,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 1,
		.phy_if		= PHY_INTERFACE_MODE_RGMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
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

#define GPIO_PHY1_RST		170
#define GPIO_PHY2_RST		168

int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	if (phydev->drv->config)
		return phydev->drv->config(phydev);

	return 0;
}

static void board_phy_init(void)
{
	set_mdio_pin_mux();
	writel(0x40003, 0x44e10a74); /* Mux pin as clkout2 */
	writel(0x10006, 0x44df4108); /* Select EXTDEV as clock source */
	writel(0x4, 0x44df2e60); /* Set EXTDEV as MNbypass */

	/* For revision A */
	writel(0x2000009, 0x44df2e6c);
	writel(0x38a, 0x44df2e70);

	mdelay(10);

	gpio_request(GPIO_PHY1_RST, "phy1_rst");
	gpio_request(GPIO_PHY2_RST, "phy2_rst");
	gpio_direction_output(GPIO_PHY1_RST, 0);
	gpio_direction_output(GPIO_PHY2_RST, 0);
	mdelay(2);

	gpio_set_value(GPIO_PHY1_RST, 1);
	gpio_set_value(GPIO_PHY2_RST, 1);
	mdelay(2);
}

int board_eth_init(bd_t *bis)
{
	int rv;

	set_rgmii_pin_mux();
	writel(RGMII_MODE_ENABLE | RGMII_INT_DELAY, &cdev->miisel);
	board_phy_init();

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);

	return rv;
}
#endif
