// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <common.h>
#include <command.h>
#include <hang.h>
#include <hexdump.h>
#include <i2c.h>
#include <linux/delay.h>
#include <dm/uclass.h>

#include "gsc.h"

DECLARE_GLOBAL_DATA_PTR;

struct venice_board_info som_info;
struct venice_board_info base_info;
char venice_model[32];
uint32_t venice_serial;

/* return a mac address from EEPROM info */
int gsc_getmac(int index, uint8_t *address)
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

/* System Controller registers */
enum {
	GSC_SC_CTRL0		= 0,
	GSC_SC_CTRL1		= 1,
	GSC_SC_STATUS		= 10,
	GSC_SC_FWCRC		= 12,
	GSC_SC_FWVER		= 14,
	GSC_SC_WP		= 15,
	GSC_SC_RST_CAUSE	= 16,
	GSC_SC_THERM_PROTECT	= 19,
};

/* System Controller Control1 bits */
enum {
	GSC_SC_CTRL1_WDTIME	= 4, /* 1 = 60s timeout, 0 = 30s timeout */
	GSC_SC_CTRL1_WDEN	= 5, /* 1 = enable, 0 = disable */
	GSC_SC_CTRL1_BOOT_CHK   = 6, /* 1 = enable alt boot check */
	GSC_SC_CTRL1_WDDIS	= 7, /* 1 = disable boot watchdog */
};

/* System Controller Interrupt bits */
enum {
	GSC_SC_IRQ_PB		= 0, /* Pushbutton switch */
	GSC_SC_IRQ_SECURE	= 1, /* Secure Key erase operation complete */
	GSC_SC_IRQ_EEPROM_WP	= 2, /* EEPROM write violation */
	GSC_SC_IRQ_GPIO		= 4, /* GPIO change */
	GSC_SC_IRQ_TAMPER	= 5, /* Tamper detect */
	GSC_SC_IRQ_WATCHDOG	= 6, /* Watchdog trip */
	GSC_SC_IRQ_PBLONG	= 7, /* Pushbutton long hold */
};

/* System Controller WP bits */
enum {
	GSC_SC_WP_ALL		= 0, /* Write Protect All EEPROM regions */
	GSC_SC_WP_BOARDINFO	= 1, /* Write Protect Board Info region */
};

/* System Controller Reset Cause */
enum {
	GSC_SC_RST_CAUSE_VIN		= 0,
	GSC_SC_RST_CAUSE_PB		= 1,
	GSC_SC_RST_CAUSE_WDT		= 2,
	GSC_SC_RST_CAUSE_CPU		= 3,
	GSC_SC_RST_CAUSE_TEMP_LOCAL	= 4,
	GSC_SC_RST_CAUSE_TEMP_REMOTE	= 5,
	GSC_SC_RST_CAUSE_SLEEP		= 6,
	GSC_SC_RST_CAUSE_BOOT_WDT	= 7,
	GSC_SC_RST_CAUSE_BOOT_WDT_MAN	= 8,
	GSC_SC_RST_CAUSE_SOFT_PWR	= 9,
	GSC_SC_RST_CAUSE_MAX		= 10,
};

#include <dm/device.h>
static struct udevice *gsc_get_dev(int busno, int slave)
{
	struct udevice *dev, *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, busno, &bus);
	if (ret) {
		printf("GSC     : failed I2C%d probe: %d\n", busno, ret);
		return NULL;
	}
	ret = dm_i2c_probe(bus, slave, 0, &dev);
	if (ret)
		return NULL;

	return dev;
}

static int gsc_read_eeprom(int bus, int slave, int alen, struct venice_board_info *info)
{
	int i;
	int chksum;
	unsigned char *buf = (unsigned char *)info;
	struct udevice *dev;
	int ret;

	/* probe device */
	dev = gsc_get_dev(bus, slave);
	if (!dev) {
		if (slave == GSC_EEPROM_ADDR)
			puts("ERROR: Failed to probe EEPROM\n");
		return -ENODEV;
	}

	/* read eeprom config section */
	memset(info, 0, sizeof(*info));
	ret = i2c_set_chip_offset_len(dev, alen);
	if (ret) {
		puts("EEPROM: Failed to set alen\n");
		return ret;
	}
	ret = dm_i2c_read(dev, 0x00, buf, sizeof(*info));
	if (ret) {
		if (slave == GSC_EEPROM_ADDR)
			printf("EEPROM: Failed to read EEPROM\n");
		return ret;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < (int)sizeof(*info) - 2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != chksum >> 8) ||
	    (info->chksum[1] != (chksum & 0xff))) {
		printf("EEPROM: I2C%d@0x%02x: Invalid Checksum\n", bus, slave);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, buf, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	/* sanity check valid model */
	if (info->model[0] != 'G' || info->model[1] != 'W') {
		printf("EEPROM: I2C%d@0x%02x: Invalid Model in EEPROM\n", bus, slave);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, buf, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	return 0;
}

static const char *gsc_get_rst_cause(struct udevice *dev)
{
	static char str[64];
	static const char * const names[] = {
		"VIN",
		"PB",
		"WDT",
		"CPU",
		"TEMP_L",
		"TEMP_R",
		"SLEEP",
		"BOOT_WDT1",
		"BOOT_WDT2",
		"SOFT_PWR",
	};
	unsigned char reg;

	/* reset cause */
	str[0] = 0;
	if (!dm_i2c_read(dev, GSC_SC_RST_CAUSE, &reg, 1)) {
		if (reg < ARRAY_SIZE(names))
			sprintf(str, "%s", names[reg]);
		else
			sprintf(str, "0x%02x", reg);
	}

	/* thermal protection */
	if (!dm_i2c_read(dev, GSC_SC_THERM_PROTECT, &reg, 1)) {
		strcat(str, " Thermal Protection ");
		if (reg & BIT(0))
			strcat(str, "Enabled");
		else
			strcat(str, "Disabled");
	}

	return str;
}

/* display hardware monitor ADC channels */
int gsc_hwmon(void)
{
	const void *fdt = gd->fdt_blob;
	struct udevice *dev;
	int node, reg, mode, len, val, offset;
	const char *label;
	u8 buf[2];
	int ret;

	node = fdt_node_offset_by_compatible(fdt, -1, "gw,gsc-adc");
	if (node <= 0)
		return node;

	/* probe device */
	dev = gsc_get_dev(GSC_BUSNO, GSC_HWMON_ADDR);
	if (!dev) {
		puts("ERROR: Failed to probe GSC HWMON\n");
		return -ENODEV;
	}

	/* iterate over hwmon nodes */
	node = fdt_first_subnode(fdt, node);
	while (node > 0) {
		reg = fdtdec_get_int(fdt, node, "reg", -1);
		mode = fdtdec_get_int(fdt, node, "gw,mode", -1);
		offset = fdtdec_get_int(fdt, node, "gw,voltage-offset-microvolt", 0);
		label = fdt_stringlist_get(fdt, node, "label", 0, NULL);

		if ((reg == -1) || (mode == -1) || !label)
			printf("invalid dt:%s\n", fdt_get_name(fdt, node, NULL));

		memset(buf, 0, sizeof(buf));
		ret = dm_i2c_read(dev, reg, buf, sizeof(buf));
		if (ret) {
			printf("i2c error: %d\n", ret);
			continue;
		}
		val = buf[0] | buf[1] << 8;
		if (val >= 0) {
			const u32 *div;
			int r[2];

			switch (mode) {
			case 0: /* temperature (C*10) */
				if (val > 0x8000)
					val -= 0xffff;
				printf("%-8s: %d.%ldC\n", label, val / 10, abs(val % 10));
				break;
			case 1: /* prescaled voltage */
				if (val != 0xffff)
					printf("%-8s: %d.%03dV\n", label, val / 1000, val % 1000);
				break;
			case 2: /* scaled based on ref volt and resolution */
				val *= 2500;
				val /= 1 << 12;

				/* apply pre-scaler voltage divider */
				div  = fdt_getprop(fdt, node, "gw,voltage-divider-ohms", &len);
				if (div && (len == sizeof(uint32_t) * 2)) {
					r[0] = fdt32_to_cpu(div[0]);
					r[1] = fdt32_to_cpu(div[1]);
					if (r[0] && r[1]) {
						val *= (r[0] + r[1]);
						val /= r[1];
					}
				}

				/* adjust by offset */
				val += (offset / 1000);

				printf("%-8s: %d.%03dV\n", label, val / 1000, val % 1000);
				break;
			}
		}
		node = fdt_next_subnode(fdt, node);
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
const char *gsc_get_dtb_name(int level, char *buf, int sz)
{
	const char *pre = "imx8mm-venice-gw";
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
		default:
			return NULL;
		}
	}

	return buf;
}

static int gsc_read(void)
{
	char rev_pcb;
	int rev_bom;
	int ret;

	ret = gsc_read_eeprom(GSC_BUSNO, GSC_EEPROM_ADDR, 1, &som_info);
	if (ret) {
		memset(&som_info, 0, sizeof(som_info));
		return ret;
	}

	/* read optional baseboard EEPROM */
	gsc_read_eeprom(BASEBOARD_EEPROM_BUSNO, BASEBOARD_EEPROM_ADDR,
			2, &base_info);

	/* create model strings */
	if (base_info.model[0]) {
		sprintf(venice_model, "GW%c%c%c%c-%c%c-",
			som_info.model[2], /* family */
			base_info.model[3], /* baseboard */
			base_info.model[4], base_info.model[5], /* subload of baseboard */
			som_info.model[4], som_info.model[5]); /* last 2digits of SOM */

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

	return 0;
}

static int gsc_info(int verbose)
{
	struct udevice *dev;
	unsigned char buf[16];

	printf("Model   : %s\n", venice_model);
	printf("Serial  : %d\n", som_info.serial);
	printf("MFGDate : %02x-%02x-%02x%02x\n",
	       som_info.mfgdate[0], som_info.mfgdate[1],
	       som_info.mfgdate[2], som_info.mfgdate[3]);
	if (base_info.model[0] && verbose > 1) {
		printf("SOM     : %s %d %02x-%02x-%02x%02x\n",
		       som_info.model, som_info.serial,
		       som_info.mfgdate[0], som_info.mfgdate[1],
		       som_info.mfgdate[2], som_info.mfgdate[3]);
		printf("BASE    : %s %d %02x-%02x-%02x%02x\n",
		       base_info.model, base_info.serial,
		       base_info.mfgdate[0], base_info.mfgdate[1],
		       base_info.mfgdate[2], base_info.mfgdate[3]);
	}

	/* Display RTC */
	puts("RTC     : ");
	dev = gsc_get_dev(GSC_BUSNO, GSC_RTC_ADDR);
	if (!dev) {
		puts("Failed to probe GSC RTC\n");
	} else {
		dm_i2c_read(dev, 0, buf, 6);
		printf("%d\n", buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
	}

	/* Display hwmon */
	gsc_hwmon();

	return 0;
}

int gsc_init(int quiet)
{
	unsigned char buf[16];
	struct udevice *dev;
	int ret;

	/*
	 * On a board with a missing/depleted backup battery for GSC, the
	 * board may be ready to probe the GSC before its firmware is
	 * running.  We will wait here indefinately for the GSC/EEPROM.
	 */
	while (1) {
		/* probe device */
		dev = gsc_get_dev(GSC_BUSNO, GSC_SC_ADDR);
		if (dev)
			break;
		mdelay(1);
	}

	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret) {
		puts("ERROR: Failed reading GSC\n");
		return ret;
	}
	gsc_read();

	/* banner */
	if (!quiet) {
		printf("GSC     : v%d 0x%04x", buf[GSC_SC_FWVER],
		       buf[GSC_SC_FWCRC] | buf[GSC_SC_FWCRC + 1] << 8);
		printf(" RST:%s", gsc_get_rst_cause(dev));
		printf("\n");
		gsc_info(1);
	}

	if (ret)
		hang();

	return ((16 << som_info.sdram_size) / 1024);
}

const char *gsc_get_model(void)
{
	return venice_model;
}

uint32_t gsc_get_serial(void)
{
	return venice_serial;
}

#if !(IS_ENABLED(CONFIG_SPL_BUILD))
static int gsc_sleep(unsigned long secs)
{
	unsigned char reg;
	struct udevice *dev;
	int ret;

	/* probe device */
	dev = gsc_get_dev(GSC_BUSNO, GSC_SC_ADDR);
	if (!dev)
		return -ENODEV;

	printf("GSC Sleeping for %ld seconds\n", secs);
	reg = (secs >> 24) & 0xff;
	ret = dm_i2c_write(dev, 9, &reg, 1);
	if (ret)
		goto err;
	reg = (secs >> 16) & 0xff;
	ret = dm_i2c_write(dev, 8, &reg, 1);
	if (ret)
		goto err;
	reg = (secs >> 8) & 0xff;
	ret = dm_i2c_write(dev, 7, &reg, 1);
	if (ret)
		goto err;
	reg = secs & 0xff;
	ret = dm_i2c_write(dev, 6, &reg, 1);
	if (ret)
		goto err;
	ret = dm_i2c_read(dev, GSC_SC_CTRL1, &reg, 1);
	if (ret)
		goto err;
	reg |= (1 << 2);
	ret = dm_i2c_write(dev, GSC_SC_CTRL1, &reg, 1);
	if (ret)
		goto err;
	reg &= ~(1 << 2);
	reg |= 0x3;
	ret = dm_i2c_write(dev, GSC_SC_CTRL1, &reg, 1);
	if (ret)
		goto err;

	return 0;

err:
	printf("i2c error\n");
	return ret;
}

static int gsc_boot_wd_disable(void)
{
	u8 reg;
	struct udevice *dev;
	int ret;

	/* probe device */
	dev = gsc_get_dev(GSC_BUSNO, GSC_SC_ADDR);
	if (!dev)
		return -ENODEV;

	ret = dm_i2c_read(dev, GSC_SC_CTRL1, &reg, 1);
	if (ret)
		goto err;
	reg |= (1 << GSC_SC_CTRL1_WDDIS);
	reg &= ~(1 << GSC_SC_CTRL1_BOOT_CHK);
	ret = dm_i2c_write(dev, GSC_SC_CTRL1, &reg, 1);
	if (ret)
		goto err;
	puts("GSC     : boot watchdog disabled\n");

	return 0;

err:
	printf("i2c error");
	return ret;
}

static int do_gsc(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return gsc_info(2);

	if (strcasecmp(argv[1], "sleep") == 0) {
		if (argc < 3)
			return CMD_RET_USAGE;
		if (!gsc_sleep(dectoul(argv[2], NULL)))
			return CMD_RET_SUCCESS;
	} else if (strcasecmp(argv[1], "hwmon") == 0) {
		if (!gsc_hwmon())
			return CMD_RET_SUCCESS;
	} else if (strcasecmp(argv[1], "wd-disable") == 0) {
		if (!gsc_boot_wd_disable())
			return CMD_RET_SUCCESS;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(gsc, 4, 1, do_gsc, "Gateworks System Controller",
	   "[sleep <secs>]|[hwmon]|[wd-disable]\n");
#endif
