/*
 * common.c
 *
 * common board functions for B&R boards
 *
 * Copyright (C) 2013 Hannes Petermaier <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include "bur_common.h"

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
/* --------------------------------------------------------------------------*/
void blink(u32 blinks, u32 intervall, u32 pin)
{
	gpio_direction_output(pin, 0);
	int val = 0;

	do {
		val ^= 0x01;
		gpio_set_value(pin, val);
		mdelay(intervall);
	} while (blinks--);

	gpio_set_value(pin, 0);
}
#ifdef CONFIG_SPL_BUILD
void pmicsetup(u32 mpupll)
{
	int mpu_vdd;
	int usb_cur_lim;

	/* setup I2C */
	enable_i2c0_pin_mux();
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);

	if (i2c_probe(TPS65217_CHIP_PM)) {
		puts("PMIC (0x24) not found! skip further initalization.\n");
		return;
	}

	/* Get the frequency which is defined by device fuses */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);
	printf("detected max. frequency: %d - ", dpll_mpu_opp100.m);

	if (0 != mpupll) {
		dpll_mpu_opp100.m = MPUPLL_M_1000;
		printf("retuning MPU-PLL to: %d MHz.\n", dpll_mpu_opp100.m);
	} else {
		puts("ok.\n");
	}
	/*
	 * Increase USB current limit to 1300mA or 1800mA and set
	 * the MPU voltage controller as needed.
	 */
	if (dpll_mpu_opp100.m == MPUPLL_M_1000) {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1800MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;
	} else {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
	}

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_POWER_PATH,
			       usb_cur_lim, TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set DCDC3 (CORE) voltage to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
				    TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set LDO3 to 1.8V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS1,
			       TPS65217_LDO_VOLTAGE_OUT_1_8,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");
	/* Set LDO4 to 3.3V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS2,
			       TPS65217_LDO_VOLTAGE_OUT_3_3,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

#endif /* CONFIG_SPL_BUILD */

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */
	return;
}

/* describing port offsets of TI's CPSW block */
static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_id		= 0,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_id		= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
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
#endif /* CONFIG_DRIVER_TI_CPSW, ... */

#if defined(CONFIG_DRIVER_TI_CPSW)

int board_eth_init(bd_t *bis)
{
	int rv = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
	if (!getenv("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC ... ");

		if (is_valid_ether_addr(mac_addr)) {
			printf("using: %02X:%02X:%02X:%02X:%02X:%02X.\n",
			       mac_addr[0], mac_addr[1], mac_addr[2],
			       mac_addr[3], mac_addr[4], mac_addr[5]
				);
			eth_setenv_enetaddr("ethaddr", mac_addr);
		}
	}
	writel(MII_MODE_ENABLE, &cdev->miisel);
	cpsw_slaves[0].phy_if = PHY_INTERFACE_MODE_MII;
	cpsw_slaves[1].phy_if =	PHY_INTERFACE_MODE_MII;

	rv = cpsw_register(&cpsw_data);
	if (rv < 0) {
		printf("Error %d registering CPSW switch\n", rv);
		return 0;
	}
#endif /* CONFIG_DRIVER_TI_CPSW, ... */
	return rv;
}
#endif /* CONFIG_DRIVER_TI_CPSW */
