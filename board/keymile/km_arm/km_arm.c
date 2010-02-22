/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <i2c.h>
#include <nand.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/mpp.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

static int	io_dev;
extern I2C_MUX_DEVICE *i2c_mux_ident_muxstring (uchar *buf);

/* Multi-Purpose Pins Functionality configuration */
u32 kwmpp_config[] = {
	MPP0_NF_IO2,
	MPP1_NF_IO3,
	MPP2_NF_IO4,
	MPP3_NF_IO5,
	MPP4_NF_IO6,
	MPP5_NF_IO7,
	MPP6_SYSRST_OUTn,
	MPP7_PEX_RST_OUTn,
#if defined(CONFIG_SOFT_I2C)
	MPP8_GPIO,		/* SDA */
	MPP9_GPIO,		/* SCL */
#endif
#if defined(CONFIG_HARD_I2C)
	MPP8_TW_SDA,
	MPP9_TW_SCK,
#endif
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP12_GPO,		/* Reserved */
	MPP13_UART1_TXD,
	MPP14_UART1_RXD,
	MPP15_GPIO,		/* Not used */
	MPP16_GPIO,		/* Not used */
	MPP17_GPIO,		/* Reserved */
	MPP18_NF_IO0,
	MPP19_NF_IO1,
	MPP20_GPIO,
	MPP21_GPIO,
	MPP22_GPIO,
	MPP23_GPIO,
	MPP24_GPIO,
	MPP25_GPIO,
	MPP26_GPIO,
	MPP27_GPIO,
	MPP28_GPIO,
	MPP29_GPIO,
	MPP30_GPIO,
	MPP31_GPIO,
	MPP32_GPIO,
	MPP33_GPIO,
	MPP34_GPIO,		/* CDL1 (input) */
	MPP35_GPIO,		/* CDL2 (input) */
	MPP36_GPIO,		/* MAIN_IRQ (input) */
	MPP37_GPIO,		/* BOARD_LED */
	MPP38_GPIO,		/* Piggy3 LED[1] */
	MPP39_GPIO,		/* Piggy3 LED[2] */
	MPP40_GPIO,		/* Piggy3 LED[3] */
	MPP41_GPIO,		/* Piggy3 LED[4] */
	MPP42_GPIO,		/* Piggy3 LED[5] */
	MPP43_GPIO,		/* Piggy3 LED[6] */
	MPP44_GPIO,		/* Piggy3 LED[7] */
	MPP45_GPIO,		/* Piggy3 LED[8] */
	MPP46_GPIO,		/* Reserved */
	MPP47_GPIO,		/* Reserved */
	MPP48_GPIO,		/* Reserved */
	MPP49_GPIO,		/* SW_INTOUTn */
	0
};

int ethernet_present(void)
{
	uchar	buf;
	int	ret = 0;

	if (i2c_read(0x10, 2, 1, &buf, 1) != 0) {
		printf ("%s: Error reading Boco\n", __FUNCTION__);
		return -1;
	}
	if ((buf & 0x40) == 0x40) {
		ret = 1;
	}
	return ret;
}

int misc_init_r(void)
{
	I2C_MUX_DEVICE	*i2cdev;
	char *str;
	int mach_type;

	/* add I2C Bus for I/O Expander */
	i2cdev = i2c_mux_ident_muxstring((uchar *)"pca9554a:70:a");
	io_dev = i2cdev->busid;
	puts("Piggy:");
	if (ethernet_present() == 0)
		puts (" not");
	puts(" present\n");

	str = getenv("mach_type");
	if (str != NULL) {
		mach_type = simple_strtoul(str, NULL, 10);
		printf("Overwriting MACH_TYPE with %d!!!\n", mach_type);
		gd->bd->bi_arch_number = mach_type;
	}
	return 0;
}

int board_init(void)
{
	u32 tmp;

	kirkwood_mpp_conf(kwmpp_config);

	/*
	 * The FLASH_GPIO_PIN switches between using a
	 * NAND or a SPI FLASH. Set this pin on start
	 * to NAND mode.
	 */
	tmp = readl(KW_GPIO0_BASE);
	writel(tmp | FLASH_GPIO_PIN , KW_GPIO0_BASE);
	tmp = readl(KW_GPIO0_BASE + 4);
	writel(tmp & (~FLASH_GPIO_PIN) , KW_GPIO0_BASE + 4);
	printf("KM: setting NAND mode\n");

	/*
	 * arch number of board
	 */
	gd->bd->bi_arch_number = MACH_TYPE_SUEN3;

	/* address of boot parameters */
	gd->bd->bi_boot_params = kw_sdram_bar(0) + 0x100;

#if defined(CONFIG_SOFT_I2C)
	/* init the GPIO for I2C Bitbang driver */
	kw_gpio_set_valid(SUEN3_SDA_PIN, 1);
	kw_gpio_set_valid(SUEN3_SCL_PIN, 1);
	kw_gpio_direction_output(SUEN3_SDA_PIN, 0);
	kw_gpio_direction_output(SUEN3_SCL_PIN, 0);
#endif
#if defined(CONFIG_SYS_EEPROM_WREN)
	kw_gpio_set_valid(SUEN3_ENV_WP, 38);
	kw_gpio_direction_output(SUEN3_ENV_WP, 1);
#endif
	return 0;
}

#if defined(CONFIG_CMD_SF)
int do_spi_toggle(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 tmp;
	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	if ((strcmp(argv[1], "off") == 0)) {
		printf("SPI FLASH disabled, NAND enabled\n");
		/* Multi-Purpose Pins Functionality configuration */
		kwmpp_config[0] = MPP0_NF_IO2;
		kwmpp_config[1] = MPP1_NF_IO3;
		kwmpp_config[2] = MPP2_NF_IO4;
		kwmpp_config[3] = MPP3_NF_IO5;

		kirkwood_mpp_conf(kwmpp_config);
		tmp = readl(KW_GPIO0_BASE);
		writel(tmp | FLASH_GPIO_PIN , KW_GPIO0_BASE);
	} else if ((strcmp(argv[1], "on") == 0)) {
		printf("SPI FLASH enabled, NAND disabled\n");
		/* Multi-Purpose Pins Functionality configuration */
		kwmpp_config[0] = MPP0_SPI_SCn;
		kwmpp_config[1] = MPP1_SPI_MOSI;
		kwmpp_config[2] = MPP2_SPI_SCK;
		kwmpp_config[3] = MPP3_SPI_MISO;

		kirkwood_mpp_conf(kwmpp_config);
		tmp = readl(KW_GPIO0_BASE);
		writel(tmp & (~FLASH_GPIO_PIN) , KW_GPIO0_BASE);
	} else {
		cmd_usage(cmdtp);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	spitoggle,	2,	0,	do_spi_toggle,
	"En-/disable SPI FLASH access",
	"<on|off> - Enable (on) or disable (off) SPI FLASH access\n"
	);
#endif

int dram_init(void)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = kw_sdram_bar(i);
		gd->bd->bi_dram[i].size = get_ram_size((long *)kw_sdram_bar(i),
						       kw_sdram_bs(i));
	}
	return 0;
}

/* Configure and enable MV88E1118 PHY */
void reset_phy(void)
{
	char *name = "egiga0";

	if (miiphy_set_current_dev(name))
		return;

	/* reset the phy */
	miiphy_reset(name, CONFIG_PHY_BASE_ADR);
}

#if defined(CONFIG_HUSH_INIT_VAR)
int hush_init_var (void)
{
	ivm_read_eeprom ();
	return 0;
}
#endif

#if defined(CONFIG_BOOTCOUNT_LIMIT)
void bootcount_store (ulong a)
{
	volatile ulong *save_addr;
	volatile ulong size = 0;
	int i;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	save_addr = (ulong*)(size - BOOTCOUNT_ADDR);
	writel(a, save_addr);
	writel(BOOTCOUNT_MAGIC, &save_addr[1]);
}

ulong bootcount_load (void)
{
	volatile ulong *save_addr;
	volatile ulong size = 0;
	int i;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	save_addr = (ulong*)(size - BOOTCOUNT_ADDR);
	if (readl(&save_addr[1]) != BOOTCOUNT_MAGIC)
		return 0;
	else
		return readl(save_addr);
}
#endif

#if defined(CONFIG_SOFT_I2C)
void set_sda (int state)
{
	I2C_ACTIVE;
	I2C_SDA(state);
}

void set_scl (int state)
{
	I2C_SCL(state);
}

int get_sda (void)
{
	I2C_TRISTATE;
	return I2C_READ;
}

int get_scl (void)
{
	return (kw_gpio_get_value(SUEN3_SCL_PIN) ? 1 : 0);
}
#endif

#if defined(CONFIG_SYS_EEPROM_WREN)
int eeprom_write_enable (unsigned dev_addr, int state)
{
	kw_gpio_set_value(SUEN3_ENV_WP, !state);

	return !kw_gpio_get_value(SUEN3_ENV_WP);
}
#endif
