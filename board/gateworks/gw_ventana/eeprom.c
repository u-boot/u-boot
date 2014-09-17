/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <malloc.h>
#include <asm/bitops.h>

#include "gsc.h"
#include "ventana_eeprom.h"

/* read ventana EEPROM, check for validity, and return baseboard type */
int
read_eeprom(int bus, struct ventana_board_info *info)
{
	int i;
	int chksum;
	char baseboard;
	int type;
	unsigned char *buf = (unsigned char *)info;

	memset(info, 0, sizeof(*info));

	/*
	 * On a board with a missing/depleted backup battery for GSC, the
	 * board may be ready to probe the GSC before its firmware is
	 * running.  We will wait here indefinately for the GSC/EEPROM.
	 */
	while (1) {
		if (0 == i2c_set_bus_num(bus) &&
		    0 == i2c_probe(GSC_EEPROM_ADDR))
			break;
		mdelay(1);
	}

	/* read eeprom config section */
	if (gsc_i2c_read(GSC_EEPROM_ADDR, 0x00, 1, buf, sizeof(*info))) {
		puts("EEPROM: Failed to read EEPROM\n");
		return GW_UNKNOWN;
	}

	/* sanity checks */
	if (info->model[0] != 'G' || info->model[1] != 'W') {
		puts("EEPROM: Invalid Model in EEPROM\n");
		return GW_UNKNOWN;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < sizeof(*info)-2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != chksum>>8) ||
	    (info->chksum[1] != (chksum&0xff))) {
		puts("EEPROM: Failed EEPROM checksum\n");
		return GW_UNKNOWN;
	}

	/* original GW5400-A prototype */
	baseboard = info->model[3];
	if (strncasecmp((const char *)info->model, "GW5400-A", 8) == 0)
		baseboard = '0';

	switch (baseboard) {
	case '0': /* original GW5400-A prototype */
		type = GW54proto;
		break;
	case '1':
		type = GW51xx;
		break;
	case '2':
		type = GW52xx;
		break;
	case '3':
		type = GW53xx;
		break;
	case '4':
		type = GW54xx;
		break;
	case '5':
		type = GW552x;
		break;
	default:
		printf("EEPROM: Unknown model in EEPROM: %s\n", info->model);
		type = GW_UNKNOWN;
		break;
	}
	return type;
}

/* list of config bits that the bootloader will remove from dtb if not set */
struct ventana_eeprom_config econfig[] = {
	{ "eth0", "ethernet0", EECONFIG_ETH0 },
	{ "eth1", "ethernet1", EECONFIG_ETH1 },
	{ "sata", "ahci0", EECONFIG_SATA },
	{ "pcie", NULL, EECONFIG_PCIE},
	{ "lvds0", NULL, EECONFIG_LVDS0 },
	{ "lvds1", NULL, EECONFIG_LVDS1 },
	{ "usb0", NULL, EECONFIG_USB0 },
	{ "usb1", NULL, EECONFIG_USB1 },
	{ "mmc0", NULL, EECONFIG_SD0 },
	{ "mmc1", NULL, EECONFIG_SD1 },
	{ "mmc2", NULL, EECONFIG_SD2 },
	{ "mmc3", NULL, EECONFIG_SD3 },
	{ "uart0", NULL, EECONFIG_UART0 },
	{ "uart1", NULL, EECONFIG_UART1 },
	{ "uart2", NULL, EECONFIG_UART2 },
	{ "uart3", NULL, EECONFIG_UART3 },
	{ "uart4", NULL, EECONFIG_UART4 },
	{ "ipu0", NULL, EECONFIG_IPU0 },
	{ "ipu1", NULL, EECONFIG_IPU1 },
	{ "can0", NULL, EECONFIG_FLEXCAN },
	{ "i2c0", NULL, EECONFIG_I2C0 },
	{ "i2c1", NULL, EECONFIG_I2C1 },
	{ "i2c2", NULL, EECONFIG_I2C2 },
	{ "vpu", NULL, EECONFIG_VPU },
	{ "csi0", NULL, EECONFIG_CSI0 },
	{ "csi1", NULL, EECONFIG_CSI1 },
	{ "spi0", NULL, EECONFIG_ESPCI0 },
	{ "spi1", NULL, EECONFIG_ESPCI1 },
	{ "spi2", NULL, EECONFIG_ESPCI2 },
	{ "spi3", NULL, EECONFIG_ESPCI3 },
	{ "spi4", NULL, EECONFIG_ESPCI4 },
	{ "spi5", NULL, EECONFIG_ESPCI5 },
	{ "gps", "pps", EECONFIG_GPS },
	{ "hdmi_in", NULL, EECONFIG_HDMI_IN },
	{ "hdmi_out", NULL, EECONFIG_HDMI_OUT },
	{ "cvbs_in", NULL, EECONFIG_VID_IN },
	{ "cvbs_out", NULL, EECONFIG_VID_OUT },
	{ "nand", NULL, EECONFIG_NAND },
	{ /* Sentinel */ }
};

#ifdef CONFIG_CMD_EECONFIG
static struct ventana_eeprom_config *get_config(const char *name)
{
	struct ventana_eeprom_config *cfg = econfig;

	while (cfg->name) {
		if (0 == strcmp(name, cfg->name))
			return cfg;
		cfg++;
	}
	return NULL;
}

static u8 econfig_bytes[sizeof(ventana_info.config)];
static int econfig_init = -1;

int do_econfig(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct ventana_eeprom_config *cfg;
	struct ventana_board_info *info = &ventana_info;
	int i;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* initialize */
	if (econfig_init != 1) {
		memcpy(econfig_bytes, info->config, sizeof(econfig_bytes));
		econfig_init = 1;
	}

	/* list configs */
	if ((strncmp(argv[1], "list", 4) == 0)) {
		cfg = econfig;
		while (cfg->name) {
			printf("%s: %d\n", cfg->name,
			       test_bit(cfg->bit, econfig_bytes) ?  1 : 0);
			cfg++;
		}
	}

	/* save */
	else if ((strncmp(argv[1], "save", 4) == 0)) {
		unsigned char *buf = (unsigned char *)info;
		int chksum;

		/* calculate new checksum */
		memcpy(info->config, econfig_bytes, sizeof(econfig_bytes));
		for (chksum = 0, i = 0; i < sizeof(*info)-2; i++)
			chksum += buf[i];
		debug("old chksum:0x%04x\n",
		      (info->chksum[0] << 8) | info->chksum[1]);
		debug("new chksum:0x%04x\n", chksum);
		info->chksum[0] = chksum >> 8;
		info->chksum[1] = chksum & 0xff;

		/* write new config data */
		if (gsc_i2c_write(GSC_EEPROM_ADDR, info->config - (u8 *)info,
				  1, econfig_bytes, sizeof(econfig_bytes))) {
			printf("EEPROM: Failed updating config\n");
			return CMD_RET_FAILURE;
		}

		/* write new config data */
		if (gsc_i2c_write(GSC_EEPROM_ADDR, info->chksum - (u8 *)info,
				  1, info->chksum, 2)) {
			printf("EEPROM: Failed updating checksum\n");
			return CMD_RET_FAILURE;
		}

		printf("Config saved to EEPROM\n");
	}

	/* get config */
	else if (argc == 2) {
		cfg = get_config(argv[1]);
		if (cfg) {
			printf("%s: %d\n", cfg->name,
			       test_bit(cfg->bit, econfig_bytes) ? 1 : 0);
		} else {
			printf("invalid config: %s\n", argv[1]);
			return CMD_RET_FAILURE;
		}
	}

	/* set config */
	else if (argc == 3) {
		cfg = get_config(argv[1]);
		if (cfg) {
			if (simple_strtol(argv[2], NULL, 10)) {
				test_and_set_bit(cfg->bit, econfig_bytes);
				printf("Enabled %s\n", cfg->name);
			} else {
				test_and_clear_bit(cfg->bit, econfig_bytes);
				printf("Disabled %s\n", cfg->name);
			}
		} else {
			printf("invalid config: %s\n", argv[1]);
			return CMD_RET_FAILURE;
		}
	}

	else
		return CMD_RET_USAGE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	econfig, 3, 0, do_econfig,
	"EEPROM configuration",
	"list - list config\n"
	"save - save config to EEPROM\n"
	"<name> - get config 'name'\n"
	"<name> [0|1] - set config 'name' to value\n"
);

#endif /* CONFIG_CMD_EECONFIG */
