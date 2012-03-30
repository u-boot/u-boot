/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <hwconfig.h>
#include <i2c.h>
#include <malloc.h>
#include <miiphy.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/da850_lowlevel.h>
#include <asm/arch/davinci_misc.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sdmmc_defs.h>
#include <asm/arch/timer_defs.h>

DECLARE_GLOBAL_DATA_PTR;

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },
	{ DAVINCI_LPSC_SPI1 },
	{ DAVINCI_LPSC_ARM_RAM_ROM },
	{ DAVINCI_LPSC_UART0 },
	{ DAVINCI_LPSC_EMAC },
	{ DAVINCI_LPSC_UART0 },
	{ DAVINCI_LPSC_GPIO },
	{ DAVINCI_LPSC_DDR_EMIF },
	{ DAVINCI_LPSC_UART1 },
	{ DAVINCI_LPSC_UART2 },
	{ DAVINCI_LPSC_MMC_SD1 },
	{ DAVINCI_LPSC_USB20 },
	{ DAVINCI_LPSC_USB11 },
};

const int lpsc_size = ARRAY_SIZE(lpsc);

static const struct pinmux_config enbw_pins[] = {
	{ pinmux(0), 8, 0 },
	{ pinmux(0), 8, 1 },
	{ pinmux(0), 8, 2 },
	{ pinmux(0), 8, 3 },
	{ pinmux(0), 8, 4 },
	{ pinmux(0), 8, 5 },
	{ pinmux(1), 4, 0 },
	{ pinmux(1), 8, 1 },
	{ pinmux(1), 8, 2 },
	{ pinmux(1), 8, 3 },
	{ pinmux(1), 8, 4 },
	{ pinmux(1), 8, 5 },
	{ pinmux(1), 8, 6 },
	{ pinmux(1), 4, 7 },
	{ pinmux(2), 8, 0 },
	{ pinmux(5), 1, 0 },
	{ pinmux(5), 1, 3 },
	{ pinmux(5), 1, 7 },
	{ pinmux(6), 1, 0 },
	{ pinmux(6), 1, 1 },
	{ pinmux(6), 8, 2 },
	{ pinmux(6), 8, 3 },
	{ pinmux(6), 1, 4 },
	{ pinmux(6), 8, 5 },
	{ pinmux(6), 1, 7 },
	{ pinmux(7), 8, 2 },
	{ pinmux(7), 1, 3 },
	{ pinmux(7), 1, 6 },
	{ pinmux(7), 1, 7 },
	{ pinmux(13), 8, 2 },
	{ pinmux(13), 8, 3 },
	{ pinmux(13), 8, 4 },
	{ pinmux(13), 8, 5 },
	{ pinmux(13), 8, 6 },
	{ pinmux(13), 8, 7 },
	{ pinmux(14), 8, 0 },
	{ pinmux(14), 8, 1 },
	{ pinmux(16), 8, 1 },
	{ pinmux(16), 8, 2 },
	{ pinmux(16), 8, 3 },
	{ pinmux(16), 8, 4 },
	{ pinmux(16), 8, 5 },
	{ pinmux(16), 8, 6 },
	{ pinmux(16), 8, 7 },
	{ pinmux(17), 1, 0 },
	{ pinmux(17), 1, 1 },
	{ pinmux(17), 1, 2 },
	{ pinmux(17), 8, 3 },
	{ pinmux(17), 8, 4 },
	{ pinmux(17), 8, 5 },
	{ pinmux(17), 8, 6 },
	{ pinmux(17), 8, 7 },
	{ pinmux(18), 8, 0 },
	{ pinmux(18), 8, 1 },
	{ pinmux(18), 2, 2 },
	{ pinmux(18), 2, 3 },
	{ pinmux(18), 2, 4 },
	{ pinmux(18), 8, 6 },
	{ pinmux(18), 8, 7 },
	{ pinmux(19), 8, 0 },
	{ pinmux(19), 2, 1 },
	{ pinmux(19), 2, 2 },
	{ pinmux(19), 2, 3 },
	{ pinmux(19), 2, 4 },
	{ pinmux(19), 8, 5 },
	{ pinmux(19), 8, 6 },
};

const struct pinmux_resource pinmuxes[] = {
	PINMUX_ITEM(emac_pins_mii),
	PINMUX_ITEM(emac_pins_mdio),
	PINMUX_ITEM(i2c0_pins),
	PINMUX_ITEM(emifa_pins_cs2),
	PINMUX_ITEM(emifa_pins_cs3),
	PINMUX_ITEM(emifa_pins_cs4),
	PINMUX_ITEM(emifa_pins_nand),
	PINMUX_ITEM(emifa_pins_nor),
	PINMUX_ITEM(spi1_pins_base),
	PINMUX_ITEM(spi1_pins_scs0),
	PINMUX_ITEM(uart1_pins_txrx),
	PINMUX_ITEM(uart2_pins_txrx),
	PINMUX_ITEM(uart2_pins_rtscts),
	PINMUX_ITEM(enbw_pins),
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

struct gpio_config {
	char name[GPIO_NAME_SIZE];
	unsigned char bank;
	unsigned char gpio;
	unsigned char out;
	unsigned char value;
};

static const struct gpio_config enbw_gpio_config[] = {
	{ "RS485 enable",	8, 11, 1, 0 },
	{ "RS485 iso",		8, 10, 1, 0 },
	{ "W2HUT RS485 Rx ena",	8,  9, 1, 0 },
	{ "W2HUT RS485 iso",	8,  8, 1, 0 },
	{ "LAN reset",		7, 15, 1, 1 },
	{ "ena 11V PLC",	7, 14, 1, 0 },
	{ "ena 1.5V PLC",	7, 13, 1, 0 },
	{ "disable VBUS",	7, 12, 1, 1 },
	{ "PLC reset",		6, 13, 1, 1 },
	{ "LCM RS",		6, 12, 1, 0 },
	{ "LCM R/W",		6, 11, 1, 0 },
	{ "PLC pairing",	6, 10, 1, 1 },
	{ "PLC MDIO CLK",	6,  9, 1, 0 },
	{ "HK218",		6,  8, 1, 0 },
	{ "HK218 Rx",		6,  1, 1, 1 },
	{ "TPM reset",		6,  0, 1, 1 },
	{ "LCM E",		2,  2, 1, 1 },
	{ "PV-IF RxD ena",	0, 15, 1, 1 },
	{ "LED1",		1, 15, 1, 1 },
	{ "LED2",		0,  1, 1, 1 },
	{ "LED3",		0,  2, 1, 1 },
	{ "LED4",		0,  3, 1, 1 },
	{ "LED5",		0,  4, 1, 1 },
	{ "LED6",		0,  5, 1, 0 },
	{ "LED7",		0,  6, 1, 0 },
	{ "LED8",		0, 14, 1, 0 },
	{ "USER1",		0, 12, 0, 0 },
	{ "USER2",		0, 13, 0, 0 },
};

#define PHY_POWER	0x0800

static void enbw_cmc_switch(int port, int on)
{
	const char	*devname;
	unsigned char phyaddr = 3;
	unsigned char	reg = 0;
	unsigned short	data;

	if (port == 1)
		phyaddr = 2;

	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("Error: no mii device\n");
		return;
	}
	if (miiphy_read(devname, phyaddr, reg, &data) != 0) {
		printf("Error reading from the PHY addr=%02x reg=%02x\n",
			phyaddr, reg);
		return;
	}

	if (on)
		data &= ~PHY_POWER;
	else
		data |= PHY_POWER;

	if (miiphy_write(devname, phyaddr, reg, data) != 0) {
		printf("Error writing to the PHY addr=%02x reg=%02x\n",
			phyaddr, reg);
		return;
	}
}

int board_init(void)
{
	int i, ret;

#ifndef CONFIG_USE_IRQ
	irq_init();
#endif
	/* address of boot parameters, not used as booting with DTT */
	gd->bd->bi_boot_params = 0;

	for (i = 0; i < ARRAY_SIZE(enbw_gpio_config); i++) {
		int gpio = enbw_gpio_config[i].bank * 16 +
			enbw_gpio_config[i].gpio;

		ret = gpio_request(gpio, enbw_gpio_config[i].name);
		if (ret) {
			printf("%s: Could not get %s gpio\n", __func__,
				enbw_gpio_config[i].name);
			return -1;
		}

		if (enbw_gpio_config[i].out)
			gpio_direction_output(gpio,
				enbw_gpio_config[i].value);
		else
			gpio_direction_input(gpio);
	}

	/* setup the SUSPSRC for ARM to control emulation suspend */
	clrbits_le32(&davinci_syscfg_regs->suspsrc,
		(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		DAVINCI_SYSCFG_SUSPSRC_UART2));

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC
/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_DRIVER_TI_EMAC
	davinci_emac_mii_mode_sel(0);
#endif /* CONFIG_DRIVER_TI_EMAC */

	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	if (hwconfig_subarg_cmp("switch", "lan", "on"))
		/* Switch port lan on */
		enbw_cmc_switch(1, 1);
	else
		enbw_cmc_switch(1, 0);

	if (hwconfig_subarg_cmp("switch", "pwl", "on"))
		/* Switch port pwl on */
		enbw_cmc_switch(2, 1);
	else
		enbw_cmc_switch(2, 0);

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */

#ifdef CONFIG_PREBOOT
static uchar kbd_magic_prefix[]		= "key_magic_";
static uchar kbd_command_prefix[]	= "key_cmd_";

struct kbd_data_t {
	char s1;
};

struct kbd_data_t *get_keys(struct kbd_data_t *kbd_data)
{
	/* read SW1 + SW2 */
	kbd_data->s1 = gpio_get_value(12) +
		(gpio_get_value(13) << 1);
	return kbd_data;
}

static int compare_magic(const struct kbd_data_t *kbd_data, char *str)
{
	char s1 = str[0];

	if (s1 >= '0' && s1 <= '9')
		s1 -= '0';
	else if (s1 >= 'a' && s1 <= 'f')
		s1 = s1 - 'a' + 10;
	else if (s1 >= 'A' && s1 <= 'F')
		s1 = s1 - 'A' + 10;
	else
		return -1;

	if (s1 != kbd_data->s1)
		return -1;

	return 0;
}

static char *key_match(const struct kbd_data_t *kbd_data)
{
	char magic[sizeof(kbd_magic_prefix) + 1];
	char *suffix;
	char *kbd_magic_keys;

	/*
	 * The following string defines the characters that can be appended
	 * to "key_magic" to form the names of environment variables that
	 * hold "magic" key codes, i. e. such key codes that can cause
	 * pre-boot actions. If the string is empty (""), then only
	 * "key_magic" is checked (old behaviour); the string "125" causes
	 * checks for "key_magic1", "key_magic2" and "key_magic5", etc.
	 */
	kbd_magic_keys = getenv("magic_keys");
	if (kbd_magic_keys == NULL)
		kbd_magic_keys = "";

	/*
	 * loop over all magic keys;
	 * use '\0' suffix in case of empty string
	 */
	for (suffix = kbd_magic_keys; *suffix ||
		suffix == kbd_magic_keys; ++suffix) {
		sprintf(magic, "%s%c", kbd_magic_prefix, *suffix);

		if (compare_magic(kbd_data, getenv(magic)) == 0) {
			char cmd_name[sizeof(kbd_command_prefix) + 1];
			char *cmd;

			sprintf(cmd_name, "%s%c", kbd_command_prefix, *suffix);
			cmd = getenv(cmd_name);

			return cmd;
		}
	}

	return NULL;
}
#endif /* CONFIG_PREBOOT */

int misc_init_r(void)
{
	char *s, buf[32];
#ifdef CONFIG_PREBOOT
	struct kbd_data_t kbd_data;
	/* Decode keys */
	char *str = strdup(key_match(get_keys(&kbd_data)));
	/* Set or delete definition */
	setenv("preboot", str);
	free(str);
#endif /* CONFIG_PREBOOT */

	/* count all restarts, and save this in an environment var */
	s = getenv("restartcount");

	if (s)
		sprintf(buf, "%ld", simple_strtoul(s, NULL, 10) + 1);
	else
		strcpy(buf, "1");

	setenv("restartcount", buf);
	saveenv();

#ifdef CONFIG_HW_WATCHDOG
	davinci_hw_watchdog_enable();
#endif

	return 0;
}

struct cmc_led {
	char name[20];
	unsigned char bank;
	unsigned char gpio;
};

struct cmc_led led_table[] = {
	{"led1", 1, 15},
	{"led2", 0, 1},
	{"led3", 0, 2},
	{"led4", 0, 3},
	{"led5", 0, 4},
	{"led6", 0, 5},
	{"led7", 0, 6},
	{"led8", 0, 14},
};

static int cmc_get_led_state(struct cmc_led *led)
{
	int value;
	int gpio = led->bank * 16 + led->gpio;

	value = gpio_get_value(gpio);

	return value;
}

static int cmc_set_led_state(struct cmc_led *led, int state)
{
	int gpio = led->bank * 16 + led->gpio;

	gpio_set_value(gpio, state);
	return 0;
}

static int do_led(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmc_led *led;
	int found = 0;
	int i = 0;
	int only_print = 0;
	int len = ARRAY_SIZE(led_table);

	if (argc < 2)
		return cmd_usage(cmdtp);

	if (argc < 3)
		only_print = 1;

	led = led_table;
	while ((!found) && (i < len)) {
		if (strcmp(argv[1], led->name) == 0) {
			found = 1;
		} else {
			led++;
			i++;
		}
	}
	if (!found)
		return cmd_usage(cmdtp);

	if (only_print) {
		if (cmc_get_led_state(led))
			printf("on\n");
		else
			printf("off\n");

		return 0;
	}
	if (strcmp(argv[2], "on") == 0)
		cmc_set_led_state(led, 1);
	else
		cmc_set_led_state(led, 0);

	return 0;
}

U_BOOT_CMD(led, 3, 1, do_led,
	"switch on/off board led",
	"[name] [on/off]"
);

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	davinci_hw_watchdog_reset();
}
#endif

#if defined(CONFIG_POST)
void arch_memory_failure_handle(void)
{
	struct davinci_gpio *gpio = davinci_gpio_bank01;
	int state = 1;

	/*
	 * if memor< failure blink with the LED 1,2 and 3
	 * as we running from flash, we cannot use the gpio
	 * api here, so access the gpio pin direct through
	 * the gpio register.
	 */
	while (1) {
		if (state) {
			clrbits_le32(&gpio->out_data, 0x80000006);
			state = 0;
		} else {
			setbits_le32(&gpio->out_data, 0x80000006);
			state = 1;
		}
		udelay(500);
	}
}
#endif

#if defined(CONFIG_BOOTCOUNT_LIMIT)
void bootcount_store(ulong a)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;

	/*
	 * write RTC kick register to enable write
	 * for RTC Scratch registers. Scratch0 and 1 are
	 * used for bootcount values.
	 */
	writel(RTC_KICK0R_WE, &reg->kick0r);
	writel(RTC_KICK1R_WE, &reg->kick1r);
	out_be32(&reg->scratch0, a);
	out_be32(&reg->scratch1, BOOTCOUNT_MAGIC);
}

ulong bootcount_load(void)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;

	if (in_be32(&reg->scratch1) != BOOTCOUNT_MAGIC)
		return 0;
	else
		return in_be32(&reg->scratch0);
}
#endif

void board_gpio_init(void)
{
	struct davinci_gpio *gpio = davinci_gpio_bank01;

	/*
	 * set LED (gpio Interface not usable here)
	 * set LED pins to output and state 0
	 */
	clrbits_le32(&gpio->dir, 0x8000407e);
	clrbits_le32(&gpio->out_data, 0x8000407e);
	/* set LED 1 - 5 to state on */
	setbits_le32(&gpio->out_data, 0x8000001e);
}

int board_late_init(void)
{
	cmc_set_led_state(&led_table[4], 0);

	return 0;
}

void show_boot_progress(int val)
{
	switch (val) {
	case 1:
		cmc_set_led_state(&led_table[4], 1);
		break;
	case 4:
		cmc_set_led_state(&led_table[4], 0);
		break;
	case 15:
		cmc_set_led_state(&led_table[4], 1);
		break;
	}
}

#ifdef CONFIG_DAVINCI_MMC
static struct davinci_mmc mmc_sd1 = {
	.reg_base	= (struct davinci_mmc_regs *)DAVINCI_MMC_SD1_BASE,
	.input_clk	= 228000000,
	.host_caps	= MMC_MODE_4BIT,
	.voltages	= MMC_VDD_32_33 | MMC_VDD_33_34,
	.version	= MMC_CTLR_VERSION_2,
};

int board_mmc_init(bd_t *bis)
{
	mmc_sd1.input_clk = clk_get(DAVINCI_MMC_CLKID);
	/* Add slot-0 to mmc subsystem */
	return davinci_mmc_init(bis, &mmc_sd1);
}
#endif
