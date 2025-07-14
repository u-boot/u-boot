// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <gsc.h>
#include <hexdump.h>
#include <i2c.h>
#include <dm/device.h>
#include <dm/uclass.h>

#include "eeprom.h"
#include "../fsa.h"

/* I2C */
#define SOM_EEPROM_BUSNO		0
#define SOM_EEPROM_ADDR			0x51
#define BASEBOARD_EEPROM_BUSNO		1
#define BASEBOARD_EEPROM_ADDR		0x52

struct venice_board_info som_info;
struct venice_board_info base_info;
char venice_model[64];
char venice_som_model[32];
char venice_baseboard_model[32];
u32 venice_serial;

/* return a mac address from EEPROM info */
int eeprom_getmac(int index, uint8_t *address)
{
	int i, j;
	u32 maclow, machigh;
	u64 mac;

	j = 0;
	if (som_info.macno) {
		maclow = som_info.mac[5];
		maclow |= som_info.mac[4] << 8;
		maclow |= som_info.mac[3] << 16;
		maclow |= som_info.mac[2] << 24;
		machigh = som_info.mac[1];
		machigh |= som_info.mac[0] << 8;
		mac = machigh;
		mac <<= 32;
		mac |= maclow;
		for (i = 0; i < som_info.macno; i++, j++) {
			if (index == j)
				goto out;
		}
	}

	maclow = base_info.mac[5];
	maclow |= base_info.mac[4] << 8;
	maclow |= base_info.mac[3] << 16;
	maclow |= base_info.mac[2] << 24;
	machigh = base_info.mac[1];
	machigh |= base_info.mac[0] << 8;
	mac = machigh;
	mac <<= 32;
	mac |= maclow;
	for (i = 0; i < base_info.macno; i++, j++) {
		if (index == j)
			goto out;
	}

	return -EINVAL;

out:
	mac += i;
	address[0] = (mac >> 40) & 0xff;
	address[1] = (mac >> 32) & 0xff;
	address[2] = (mac >> 24) & 0xff;
	address[3] = (mac >> 16) & 0xff;
	address[4] = (mac >> 8) & 0xff;
	address[5] = (mac >> 0) & 0xff;

	return 0;
}

static int eeprom_read(int busno, int slave, int alen, struct venice_board_info *info)
{
	int i;
	int chksum;
	unsigned char *buf = (unsigned char *)info;
	struct udevice *dev, *bus;
	int ret;

	/* probe device */
	ret = uclass_get_device_by_seq(UCLASS_I2C, busno, &bus);
	if (ret)
		return ret;
	ret = dm_i2c_probe(bus, slave, 0, &dev);
	if (ret)
		return ret;

	/* read eeprom config section */
	memset(info, 0, sizeof(*info));
	ret = i2c_set_chip_offset_len(dev, alen);
	if (ret) {
		puts("EEPROM: Failed to set alen\n");
		return ret;
	}
	ret = dm_i2c_read(dev, 0x00, buf, sizeof(*info));
	if (ret) {
		if (slave == SOM_EEPROM_ADDR)
			printf("EEPROM: Failed to read EEPROM\n");
		return ret;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < (int)sizeof(*info) - 2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != ((chksum >> 8) & 0xff)) ||
	    (info->chksum[1] != (chksum & 0xff))) {
		printf("EEPROM: I2C%d@0x%02x: Invalid Checksum\n", busno, slave);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, buf, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	/* sanity check valid model */
	if (info->model[0] != 'G' || info->model[1] != 'W') {
		printf("EEPROM: I2C%d@0x%02x: Invalid Model in EEPROM\n", busno, slave);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, buf, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	return 0;
}

static int fsa_eeprom_read(const char *base, int fsa, struct fsa_board_info *info)
{
	int i;
	int chksum;
	unsigned char *buf = (unsigned char *)info;
	struct udevice *dev, *bus;
	int ret;
	u8 reg;

	/* probe mux */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 2, &bus);
	if (!ret)
		ret = dm_i2c_probe(bus, 0x70, 0, &dev);
	if (ret)
		return ret;
	/* steer mux */
	if (!strncmp(base, "GW82", 4)) {
		if (fsa < 3)
			reg = (fsa == 1) ? BIT(1) : BIT(0);
		else
			return -EINVAL;
	}
	dm_i2c_write(dev, 0x00, &reg, 1);

	/* get eeprom */
	ret = dm_i2c_probe(bus, 0x54, 0, &dev);
	if (ret)
		return ret;

	/* read eeprom config section */
	ret = dm_i2c_read(dev, 0x00, buf, sizeof(*info));
	if (ret)
		return ret;

	/* validate checksum */
	for (chksum = 0, i = 0; i < (int)sizeof(*info) - 2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != ((chksum >> 8) & 0xff)) ||
	    (info->chksum[1] != (chksum & 0xff))) {
		printf("FSA%d EEPROM (board): %s: Invalid Checksum\n", fsa, dev->name);
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	return 0;
}

/* determine BOM revision from model */
int get_bom_rev(const char *str)
{
	int  rev_bom = 0;
	int i;

	for (i = strlen(str) - 1; i > 0; i--) {
		if (str[i] == '-')
			break;
		if (str[i] >= '1' && str[i] <= '9') {
			rev_bom = str[i] - '0';
			break;
		}
	}
	return rev_bom;
}

/* determine PCB revision from model */
char get_pcb_rev(const char *str)
{
	char rev_pcb = 'A';
	int i;

	for (i = strlen(str) - 1; i > 0; i--) {
		if (str[i] == '-')
			break;
		if (str[i] >= 'A') {
			rev_pcb = str[i];
			break;
		}
	}
	return rev_pcb;
}

/*
 * get dt name based on model and detail level:
 *
 * For boards that are a combination of a SoM plus a Baseboard:
 *   Venice SoM part numbers are GW70xx where xx is:
 *    7000-7019: same PCB with som dt of '0x'
 *    7020-7039: same PCB with som dt of '2x'
 *    7040-7059: same PCB with som dt of '4x'
 *    7060-7079: same PCB with som dt of '6x'
 *    7080-7099: same PCB with som dt of '8x'
 *   Venice Baseboard part numbers are GW7xxx where xxx is:
 *    7100-7199: same PCB with base dt of '71xx'
 *    7200-7299: same PCB with base dt of '72xx'
 *    7300-7399: same PCB with base dt of '73xx'
 *    7400-7499: same PCB with base dt of '74xx'
 *    7500-7599: same PCB with base dt of '75xx'
 *    7600-7699: same PCB with base dt of '76xx'
 *    7700-7799: same PCB with base dt of '77xx'
 *    7800-7899: same PCB with base dt of '78xx'
 *   DT name is comprised of:
 *    gw<base dt>-<som dt>-[base-pcb-rev][base-bom-rev][som-pcb-rev][som-bom-rev]
 *
 * For board models from 7900-7999 each PCB is unique with its own dt:
 *   DT name is comprised:
 *    gw<model>-[pcb-rev][bom-rev]
 *
 */
#define snprintfcat(dest, sz, fmt, ...) \
	snprintf((dest) + strlen(dest), (sz) - strlen(dest), fmt, ##__VA_ARGS__)
const char *eeprom_get_dtb_name(int level, char *buf, int sz)
{
#ifdef CONFIG_IMX8MM
	const char *pre = "imx8mm-venice-gw";
#elif CONFIG_IMX8MN
	const char *pre = "imx8mn-venice-gw";
#elif CONFIG_IMX8MP
	const char *pre = "imx8mp-venice-gw";
#endif
	int model, rev_pcb, rev_bom;

	model = ((som_info.model[2] - '0') * 1000)
		+ ((som_info.model[3] - '0') * 100)
		+ ((som_info.model[4] - '0') * 10)
		+ (som_info.model[5] - '0');
	rev_pcb = tolower(get_pcb_rev(som_info.model));
	rev_bom = get_bom_rev(som_info.model);

	/* som + baseboard*/
	if (base_info.model[0]) {
		/* baseboard id: 7100-7199->71; 7200-7299->72; etc */
		int base = ((base_info.model[2] - '0') * 10) + (base_info.model[3] - '0');
		/* som id: 7000-7019->1; 7020-7039->2; etc */
		int som = ((model % 100) / 20) * 2;
		int rev_base_pcb = tolower(get_pcb_rev(base_info.model));
		int rev_base_bom = get_bom_rev(base_info.model);

		snprintf(buf, sz, "%s%2dxx-%dx", pre, base, som);
		/* GW79xx baseboards have no build options */
		if (base == 79) {
			base = (int)strtoul(base_info.model + 2, NULL, 10);
			snprintf(buf, sz, "%s%4d-%dx", pre, base, som);
		}
		switch (level) {
		case 0: /* full model (ie gw73xx-0x-a1a1) */
			if (rev_base_bom)
				snprintfcat(buf, sz, "-%c%d", rev_base_pcb, rev_base_bom);
			else
				snprintfcat(buf, sz, "-%c", rev_base_pcb);
			if (rev_bom)
				snprintfcat(buf, sz, "%c%d", rev_pcb, rev_bom);
			else
				snprintfcat(buf, sz, "%c", rev_pcb);
			break;
		case 1: /* don't care about SoM revision */
			if (rev_base_bom)
				snprintfcat(buf, sz, "-%c%d", rev_base_pcb, rev_base_bom);
			else
				snprintfcat(buf, sz, "-%c", rev_base_pcb);
			snprintfcat(buf, sz, "xx");
			break;
		case 2: /* don't care about baseboard revision */
			snprintfcat(buf, sz, "-xx");
			if (rev_bom)
				snprintfcat(buf, sz, "%c%d", rev_pcb, rev_bom);
			else
				snprintfcat(buf, sz, "%c", rev_pcb);
			break;
		case 3: /* don't care about SoM/baseboard revision */
			break;
		default:
			return NULL;
		}
	} else {
		snprintf(buf, sz, "%s%04d", pre, model);
		switch (level) {
		case 0: /* full model wth PCB and BOM revision first (ie gw7901-a1) */
			if (rev_bom)
				snprintfcat(buf, sz, "-%c%d", rev_pcb, rev_bom);
			else
				snprintfcat(buf, sz, "-%c", rev_pcb);
			break;
		case 1: /* don't care about BOM revision */
			snprintfcat(buf, sz, "-%c", rev_pcb);
			break;
		case 2: /* don't care about PCB or BOM revision */
			break;
		case 3: /* don't care about last digit of model */
			buf[strlen(buf) - 1] = 'x';
			break;
		case 4: /* don't care about last two digits of model */
			buf[strlen(buf) - 1] = 'x';
			buf[strlen(buf) - 2] = 'x';
			break;
		default:
			return NULL;
			break;
		}
	}

	return buf;
}

static int eeprom_info(bool verbose)
{
	printf("Model   : %s\n", venice_model);
	printf("Serial  : %d\n", som_info.serial);
	printf("MFGDate : %02x-%02x-%02x%02x\n",
	       som_info.mfgdate[0], som_info.mfgdate[1],
	       som_info.mfgdate[2], som_info.mfgdate[3]);
	if (base_info.model[0] && verbose) {
		printf("SOM     : %s %d %02x-%02x-%02x%02x\n",
		       som_info.model, som_info.serial,
		       som_info.mfgdate[0], som_info.mfgdate[1],
		       som_info.mfgdate[2], som_info.mfgdate[3]);
		printf("BASE    : %s %d %02x-%02x-%02x%02x\n",
		       base_info.model, base_info.serial,
		       base_info.mfgdate[0], base_info.mfgdate[1],
		       base_info.mfgdate[2], base_info.mfgdate[3]);
	}
	if (verbose)
		fsa_show();

	return 0;
}

struct venice_board_info *venice_eeprom_init(int quiet)
{
	char rev_pcb;
	int rev_bom;
	int ret;

	ret = eeprom_read(SOM_EEPROM_BUSNO, SOM_EEPROM_ADDR, 1, &som_info);
	if (ret) {
		puts("ERROR: Failed to probe EEPROM\n");
		memset(&som_info, 0, sizeof(som_info));
		return 0;
	}
	strlcpy(venice_som_model, som_info.model, sizeof(venice_som_model));

	/* read optional baseboard EEPROM */
	eeprom_read(BASEBOARD_EEPROM_BUSNO, BASEBOARD_EEPROM_ADDR, 2, &base_info);

	/* create model strings */
	if (base_info.model[0]) {
		sprintf(venice_model, "GW%c%c%c%c-%c%c-",
			base_info.model[2], /* family */
			base_info.model[3], /* baseboard */
			base_info.model[4], base_info.model[5], /* subload of baseboard */
			som_info.model[4], som_info.model[5]); /* last 2digits of SOM */
		strlcpy(venice_baseboard_model, base_info.model, sizeof(venice_baseboard_model));

		/* baseboard revision */
		rev_pcb = get_pcb_rev(base_info.model);
		rev_bom = get_bom_rev(base_info.model);
		if (rev_bom)
			sprintf(venice_model + strlen(venice_model), "%c%d", rev_pcb, rev_bom);
		else
			sprintf(venice_model + strlen(venice_model), "%c", rev_pcb);
		/* som revision */
		rev_pcb = get_pcb_rev(som_info.model);
		rev_bom = get_bom_rev(som_info.model);
		if (rev_bom)
			sprintf(venice_model + strlen(venice_model), "%c%d", rev_pcb, rev_bom);
		else
			sprintf(venice_model + strlen(venice_model), "%c", rev_pcb);
	} else {
		strcpy(venice_model, som_info.model);
	}
	venice_serial = som_info.serial;

	/* GW8xxx product family naming scheme */
	if (venice_model[2] == '8') {
		struct fsa_board_info fsa_info;
		int i = 0;
		int fsa;

		/* baseboard */
		if (base_info.model[0]) {
			rev_pcb = get_pcb_rev(base_info.model);
			rev_bom = get_bom_rev(base_info.model);
			venice_model[i++] = 'G';
			venice_model[i++] = 'W';
			venice_model[i++] = base_info.model[2]; /* baseboard */
			venice_model[i++] = base_info.model[3];
			venice_model[i++] = base_info.model[4]; /* subload */
			venice_model[i++] = base_info.model[5];
			venice_model[i++] = rev_pcb;
			if (rev_bom)
				venice_model[i++] = rev_bom;
			venice_model[i++] = '-';
			venice_model[i++] = 'S';
		} else {
			venice_model[i++] = 'G';
			venice_model[i++] = 'W';
		}

		/* som */
		rev_pcb = get_pcb_rev(som_info.model);
		rev_bom = get_bom_rev(som_info.model);
		venice_model[i++] = som_info.model[4];
		venice_model[i++] = som_info.model[5];
		venice_model[i++] = rev_pcb;
		if (rev_bom)
			venice_model[i++] = rev_bom;

		/* fsa */
		for (fsa = 1; fsa < FSA_MAX; fsa++) {
			if (!fsa_eeprom_read(venice_model, fsa, &fsa_info)) {
				venice_model[i++] = '-';
				venice_model[i++] = 'F';
				venice_model[i++] = '0' + fsa;
				venice_model[i++] = fsa_info.model[5];
				venice_model[i++] = fsa_info.model[6];
				venice_model[i++] = fsa_info.model[8];
				if (fsa_info.model[9])
					venice_model[i++] = fsa_info.model[9];
			}
		}

		/* append extra model info */
		if (som_info.config[0] >= 32 && som_info.config[0] < 0x7f) {
			venice_model[i++] = '-';
			strlcpy(venice_model + i, som_info.config, (sizeof(venice_model) - i) - 1);
			i += strlen(som_info.config);
			if (i >= sizeof(venice_model))
				i = sizeof(venice_model) - 1;
		}
		venice_model[i++] = 0;
	}

	if (!quiet)
		eeprom_info(false);

	if (!strncmp(venice_model, "GW7901-SP486", 12) &&
	    strcmp(venice_model, "GW7901-SP486-C")) {
		som_info.sdram_size++;
	}

	return &som_info;
}

void board_gsc_info(void)
{
	eeprom_info(true);
}

const char *eeprom_get_model(void)
{
	return venice_model;
}

const char *eeprom_get_som_model(void)
{
	return venice_som_model;
}

const char *eeprom_get_baseboard_model(void)
{
	return venice_baseboard_model;
}

u32 eeprom_get_serial(void)
{
	return venice_serial;
}
