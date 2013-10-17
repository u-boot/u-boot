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
 * SPDX-License-Identifier:	GPL-2.0+
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
#include <spi.h>
#include <linux/ctype.h>
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
	{ pinmux(5), 1, 5 },
	{ pinmux(5), 1, 4 },
	{ pinmux(5), 1, 3 },
	{ pinmux(5), 1, 2 },
	{ pinmux(5), 1, 1 },
	{ pinmux(5), 1, 0 },
	{ pinmux(6), 8, 0 },
	{ pinmux(6), 8, 1 },
	{ pinmux(6), 8, 2 },
	{ pinmux(6), 8, 3 },
	{ pinmux(6), 8, 4 },
	{ pinmux(6), 8, 5 },
	{ pinmux(6), 1, 7 },
	{ pinmux(7), 8, 2 },
	{ pinmux(7), 1, 3 },
	{ pinmux(7), 8, 6 },
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

static const struct gpio_config enbw_gpio_config_hut[] = {
	{ "RS485 enable",	8, 11, 1, 0 },
	{ "RS485 iso",		8, 10, 1, 1 },
	{ "W2HUT RS485 Rx ena",	8,  9, 1, 0 },
	{ "W2HUT RS485 iso",	8,  8, 1, 1 },
};

static const struct gpio_config enbw_gpio_config_w[] = {
	{ "RS485 enable",	8, 11, 1, 0 },
	{ "RS485 iso",		8, 10, 1, 0 },
	{ "W2HUT RS485 Rx ena",	8,  9, 1, 0 },
	{ "W2HUT RS485 iso",	8,  8, 1, 0 },
};

static const struct gpio_config enbw_gpio_config[] = {
	{ "LAN reset",		7, 15, 1, 1 },
	{ "ena 11V PLC",	7, 14, 1, 0 },
	{ "ena 1.5V PLC",	7, 13, 1, 0 },
	{ "disable VBUS",	7, 12, 1, 1 },
	{ "PLC reset",		6, 13, 1, 0 },
	{ "LCM RS",		6, 12, 1, 0 },
	{ "LCM R/W",		6, 11, 1, 0 },
	{ "PLC pairing",	6, 10, 1, 1 },
	{ "PLC MDIO CLK",	6,  9, 1, 0 },
	{ "HK218",		6,  8, 1, 0 },
	{ "HK218 Rx",		6,  1, 1, 1 },
	{ "TPM reset",		6,  0, 1, 0 },
	{ "Board-Type",		3,  9, 0, 0 },
	{ "HW-ID0",		2,  7, 0, 0 },
	{ "HW-ID1",		2,  6, 0, 0 },
	{ "HW-ID2",		2,  3, 0, 0 },
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

static int enbw_cmc_init_gpio(const struct gpio_config *conf, int sz)
{
	int i, ret;

	for (i = 0; i < sz; i++) {
		int gpio = conf[i].bank * 16 +
			conf[i].gpio;

		ret = gpio_request(gpio, conf[i].name);
		if (ret) {
			printf("%s: Could not get %s gpio\n", __func__,
				conf[i].name);
			return ret;
		}

		if (conf[i].out)
			gpio_direction_output(gpio,
				conf[i].value);
		else
			gpio_direction_input(gpio);
	}

	return 0;
}

int board_init(void)
{
	int board_type, hw_id;

#ifndef CONFIG_USE_IRQ
	irq_init();
#endif
	/* address of boot parameters, not used as booting with DTT */
	gd->bd->bi_boot_params = 0;

	enbw_cmc_init_gpio(enbw_gpio_config, ARRAY_SIZE(enbw_gpio_config));

	/* detect HW version */
	board_type = gpio_get_value(CONFIG_ENBW_CMC_BOARD_TYPE);
	hw_id = gpio_get_value(CONFIG_ENBW_CMC_HW_ID_BIT0) +
		(gpio_get_value(CONFIG_ENBW_CMC_HW_ID_BIT1) << 1) +
		(gpio_get_value(CONFIG_ENBW_CMC_HW_ID_BIT2) << 2);
	printf("BOARD: CMC-%s hw id: %d\n", (board_type ? "w2" : "hut"),
		hw_id);
	if (board_type)
		enbw_cmc_init_gpio(enbw_gpio_config_w,
			ARRAY_SIZE(enbw_gpio_config_w));
	else
		enbw_cmc_init_gpio(enbw_gpio_config_hut,
			ARRAY_SIZE(enbw_gpio_config_hut));

	/* setup the SUSPSRC for ARM to control emulation suspend */
	clrbits_le32(&davinci_syscfg_regs->suspsrc,
		(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		DAVINCI_SYSCFG_SUSPSRC_UART2));

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC

#define KSZ_CMD_READ	0x03
#define KSZ_CMD_WRITE	0x02
#define KSZ_ID		0x95

static int enbw_cmc_switch_read(struct spi_slave *spi, u8 reg, u8 *val)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;
	int cmd_len;
	u8 cmd[2];

	cmd[0] = KSZ_CMD_READ;
	cmd[1] = reg;
	cmd_len = 2;

	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
	if (ret) {
		debug("Failed to send command (%zu bytes): %d\n",
				cmd_len, ret);
		return -EINVAL;
	}
	flags |= SPI_XFER_END;
	*val = 0;
	cmd_len = 1;
	ret = spi_xfer(spi, cmd_len * 8, NULL, val, flags);
	if (ret) {
		debug("Failed to read (%zu bytes): %d\n",
				cmd_len, ret);
		return -EINVAL;
	}

	return 0;
}

static int enbw_cmc_switch_read_ident(struct spi_slave *spi)
{
	int ret;
	u8 val;

	ret = enbw_cmc_switch_read(spi, 0, &val);
	if (ret) {
		debug("Failed to read\n");
		return -EINVAL;
	}

	if (val != KSZ_ID)
		return -EINVAL;

	return 0;
}

static int enbw_cmc_switch_write(struct spi_slave *spi, unsigned long reg,
		unsigned long val)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;
	int cmd_len;
	u8 cmd[3];

	cmd[0] = KSZ_CMD_WRITE;
	cmd[1] = reg;
	cmd[2] = val;
	cmd_len = 3;
	flags |= SPI_XFER_END;

	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
	if (ret) {
		debug("Failed to send command (%zu bytes): %d\n",
				cmd_len, ret);
		return -EINVAL;
	}

	udelay(1000);
	ret = enbw_cmc_switch_read(spi, reg, &cmd[0]);
	if (ret) {
		debug("Failed to read\n");
		return -EINVAL;
	}
	if (val != cmd[0])
		debug("warning: reg: %lx va: %x soll: %lx\n",
			reg, cmd[0], val);

	return 0;
}

static int enbw_cmc_eof(unsigned char *ptr)
{
	if (*ptr == 0xff)
		return 1;

	return 0;
}

static char *enbw_cmc_getnewline(char *ptr)
{
	while (*ptr != 0x0a) {
		ptr++;
		if (enbw_cmc_eof((unsigned char *)ptr))
			return NULL;
	}

	ptr++;
	return ptr;
}

static char *enbw_cmc_getvalue(char *ptr, int *value)
{
	int	end = 0;

	*value = -EINVAL;

	if (!isxdigit(*ptr))
		end = 1;

	while (end) {
		if ((*ptr == '#') || (*ptr == ';')) {
			ptr = enbw_cmc_getnewline(ptr);
			return ptr;
		}
		if (ptr != NULL) {
			if (isxdigit(*ptr)) {
				end = 0;
			} else if (*ptr == 0x0a) {
				ptr++;
				return ptr;
			} else {
				ptr++;
				if (enbw_cmc_eof((unsigned char *)ptr))
					return NULL;
			}
		} else {
			return NULL;
		}
	}
	*value = (int)simple_strtoul((const char *)ptr, &ptr, 16);
	ptr++;
	return ptr;
}

static struct spi_slave *enbw_cmc_init_spi(void)
{
	struct spi_slave *spi;
	int ret;

	spi = spi_setup_slave(0, 0, 1000000, 0);
	if (!spi) {
		printf("Failed to set up slave\n");
		return NULL;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		debug("Failed to claim SPI bus: %d\n", ret);
		goto err_claim_bus;
	}

	ret = enbw_cmc_switch_read_ident(spi);
	if (ret)
		goto err_read;

	return spi;
err_read:
	spi_release_bus(spi);
err_claim_bus:
	spi_free_slave(spi);
	return NULL;
}

static int enbw_cmc_config_switch(unsigned long addr)
{
	struct spi_slave *spi;
	char *ptr = (char *)addr;
	int value, reg;
	int ret = 0;

	debug("configure switch with file on addr: 0x%lx\n", addr);

	spi = enbw_cmc_init_spi();
	if (!spi)
		return -EINVAL;

	while (ptr != NULL) {
		ptr = enbw_cmc_getvalue(ptr, &reg);
		if (ptr != NULL) {
			ptr = enbw_cmc_getvalue(ptr, &value);
			if ((ptr != NULL) && (value >= 0))
				if (enbw_cmc_switch_write(spi, reg, value)) {
					/* error writing to switch */
					ptr = NULL;
					ret = -EINVAL;
				}
		}
	}

	spi_release_bus(spi);
	spi_free_slave(spi);
	return ret;
}

static int do_switch(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long addr;

	if (argc < 2)
		return cmd_usage(cmdtp);

	addr = simple_strtoul(argv[1], NULL, 16);
	enbw_cmc_config_switch(addr);

	return 0;
}

U_BOOT_CMD(switch, 3, 1, do_switch,
	"switch addr",
	"[addr]"
);

/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
	struct spi_slave *spi;
	const char *s;
	size_t len = 0;
	int config = 1;

	davinci_emac_mii_mode_sel(0);

	/* send a config file to the switch */
	s = hwconfig_subarg("switch", "config", &len);
	if (len) {
		unsigned long addr = simple_strtoul(s, NULL, 16);

		config = enbw_cmc_config_switch(addr);
	}

	if (config) {
		/*
		 * no valid config file -> do we have some args in
		 * hwconfig ?
		 */
		if ((hwconfig_subarg("switch", "lan", &len)) ||
		    (hwconfig_subarg("switch", "lmn", &len))) {
			/* If so start switch */
			spi = enbw_cmc_init_spi();
			if (spi) {
				if (enbw_cmc_switch_write(spi, 1, 0))
					config = 0;
				udelay(10000);
				if (enbw_cmc_switch_write(spi, 1, 1))
					config = 0;
				spi_release_bus(spi);
				spi_free_slave(spi);
			}
		} else {
			config = 0;
		}
	}
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	if (config) {
		if (hwconfig_subarg_cmp("switch", "lan", "on"))
			/* Switch port lan on */
			enbw_cmc_switch(1, 1);
		else
			enbw_cmc_switch(1, 0);

		if (hwconfig_subarg_cmp("switch", "lmn", "on"))
			/* Switch port pwl on */
			enbw_cmc_switch(2, 1);
		else
			enbw_cmc_switch(2, 0);
	}

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

ulong post_word_load(void)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_POST_WORD_ADDR;

	return in_be32(&reg->scratch2);
}

void post_word_store(ulong value)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_POST_WORD_ADDR;

	/*
	 * write RTC kick register to enable write
	 * for RTC Scratch registers. Cratch0 and 1 are
	 * used for bootcount values.
	 */
	writel(RTC_KICK0R_WE, &reg->kick0r);
	writel(RTC_KICK1R_WE, &reg->kick1r);
	out_be32(&reg->scratch2, value);
}

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

	/*
	 * set some gpio pins to low, this is needed early,
	 * so we have no gpio Interface here
	 * gpios:
	 * 8[8]  Mode PV select  low
	 * 8[9]  Debug Rx Enable low
	 * 8[10] Mode Select PV  low
	 * 8[11] Counter Interface RS485 Rx-Enable low
	 */
	gpio = davinci_gpio_bank8;
	clrbits_le32(&gpio->dir, 0x00000f00);
	clrbits_le32(&gpio->out_data, 0x0f00);
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
