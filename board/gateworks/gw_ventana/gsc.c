// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <common.h>
#include <i2c.h>
#include <linux/ctype.h>

#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>

#include "ventana_eeprom.h"
#include "gsc.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * The Gateworks System Controller will fail to ACK a master transaction if
 * it is busy, which can occur during its 1HZ timer tick while reading ADC's.
 * When this does occur, it will never be busy long enough to fail more than
 * 2 back-to-back transfers.  Thus we wrap i2c_read and i2c_write with
 * 3 retries.
 */
int gsc_i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = i2c_read(chip, addr, alen, buf, len);
		if (!ret)
			break;
		debug("%s: 0x%02x 0x%02x retry%d: %d\n", __func__, chip, addr,
		      n, ret);
		if (ret != -ENODEV)
			break;
		mdelay(10);
	}
	return ret;
}

int gsc_i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = i2c_write(chip, addr, alen, buf, len);
		if (!ret)
			break;
		debug("%s: 0x%02x 0x%02x retry%d: %d\n", __func__, chip, addr,
		      n, ret);
		if (ret != -ENODEV)
			break;
		mdelay(10);
	}
	mdelay(100);
	return ret;
}

int gsc_get_board_temp(void)
{
	const void *fdt = gd->fdt_blob;
	int node, reg, mode, val;
	const char *label;
	u8 buf[2];
	int ret;

	node = fdt_node_offset_by_compatible(fdt, -1, "gw,gsc-adc");
	if (node <= 0)
		return node;
	i2c_set_bus_num(0);

	/* iterate over hwmon nodes */
	node = fdt_first_subnode(fdt, node);
	while (node > 0) {
		reg = fdtdec_get_int(fdt, node, "reg", -1);
		mode = fdtdec_get_int(fdt, node, "gw,mode", -1);
		label = fdt_stringlist_get(fdt, node, "label", 0, NULL);

		if ((reg == -1) || (mode == -1) || !label) {
			printf("invalid dt:%s\n", fdt_get_name(fdt, node, NULL));
			continue;
		}

		if ((mode != 0) || strcmp(label, "temp"))
			continue;

		memset(buf, 0, sizeof(buf));
		ret = gsc_i2c_read(GSC_HWMON_ADDR, reg, 1, buf, sizeof(buf));
		val = buf[0] | buf[1] << 8;
		if (val >= 0) {
			if (val > 0x8000)
				val -= 0xffff;
			return val;
		}
		node = fdt_next_subnode(fdt, node);
	}

	return 0;
}

/* display hardware monitor ADC channels */
int gsc_hwmon(void)
{
	const void *fdt = gd->fdt_blob;
	int node, reg, mode, len, val, offset;
	const char *label;
	u8 buf[2];
	int ret;

	node = fdt_node_offset_by_compatible(fdt, -1, "gw,gsc-adc");
	if (node <= 0)
		return node;
	i2c_set_bus_num(0);

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
		ret = gsc_i2c_read(GSC_HWMON_ADDR, reg, 1, buf, sizeof(buf));
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

int gsc_info(int verbose)
{
	unsigned char buf[16];

	i2c_set_bus_num(0);
	if (gsc_i2c_read(GSC_SC_ADDR, 0, 1, buf, 16))
		return CMD_RET_FAILURE;

	printf("GSC:   v%d", buf[GSC_SC_FWVER]);
	printf(" 0x%04x", buf[GSC_SC_FWCRC] | buf[GSC_SC_FWCRC+1]<<8);
	printf(" WDT:%sabled", (buf[GSC_SC_CTRL1] & (1<<GSC_SC_CTRL1_WDEN))
		? "en" : "dis");
	if (buf[GSC_SC_STATUS] & (1 << GSC_SC_IRQ_WATCHDOG)) {
		buf[GSC_SC_STATUS] &= ~(1 << GSC_SC_IRQ_WATCHDOG);
		puts(" WDT_RESET");
		gsc_i2c_write(GSC_SC_ADDR, GSC_SC_STATUS, 1,
			      &buf[GSC_SC_STATUS], 1);
	}
	printf(" board temp at %dC", gsc_get_board_temp() / 10);
	puts("\n");
	if (!verbose)
		return CMD_RET_SUCCESS;

	gsc_hwmon();

	return 0;
}

/*
 *  The Gateworks System Controller implements a boot
 *  watchdog (always enabled) as a workaround for IMX6 boot related
 *  errata such as:
 *    ERR005768 - no fix scheduled
 *    ERR006282 - fixed in silicon r1.2
 *    ERR007117 - fixed in silicon r1.3
 *    ERR007220 - fixed in silicon r1.3
 *    ERR007926 - no fix scheduled
 *  see http://cache.freescale.com/files/32bit/doc/errata/IMX6DQCE.pdf
 *
 * Disable the boot watchdog
 */
int gsc_boot_wd_disable(void)
{
	u8 reg;

	i2c_set_bus_num(CONFIG_I2C_GSC);
	if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1)) {
		reg |= (1 << GSC_SC_CTRL1_WDDIS);
		if (!gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return 0;
	}
	puts("Error: could not disable GSC Watchdog\n");
	return 1;
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
 */
const char *gsc_get_dtb_name(int level, char *buf, int sz)
{
	const char *model = (const char *)ventana_info.model;
	const char *pre = is_mx6dq() ? "imx6q-" : "imx6dl-";
	int modelno, rev_pcb, rev_bom;

	/* a few board models are dt equivalents to other models */
	if (strncasecmp(model, "gw5906", 6) == 0)
		model = "gw552x-d";
	else if (strncasecmp(model, "gw5908", 6) == 0)
		model = "gw53xx-f";
	else if (strncasecmp(model, "gw5905", 6) == 0)
		model = "gw5904-a";

	modelno = ((model[2] - '0') * 1000)
		  + ((model[3] - '0') * 100)
		  + ((model[4] - '0') * 10)
		  + (model[5] - '0');
	rev_pcb = tolower(get_pcb_rev(model));
	rev_bom = get_bom_rev(model);

	/* compare model/rev/bom in order of most specific to least */
	snprintf(buf, sz, "%s%04d", pre, modelno);
	switch (level) {
	case 0: /* full model first (ie gw5400-a1) */
		if (rev_bom) {
			snprintf(buf, sz, "%sgw%04d-%c%d", pre, modelno, rev_pcb, rev_bom);
			break;
		}
		fallthrough;
	case 1: /* don't care about bom rev (ie gw5400-a) */
		snprintf(buf, sz, "%sgw%04d-%c", pre, modelno, rev_pcb);
		break;
	case 2: /* don't care about the pcb rev (ie gw5400) */
		snprintf(buf, sz, "%sgw%04d", pre, modelno);
		break;
	case 3: /* look for generic model (ie gw540x) */
		snprintf(buf, sz, "%sgw%03dx", pre, modelno / 10);
		break;
	case 4: /* look for more generic model (ie gw54xx) */
		snprintf(buf, sz, "%sgw%02dxx", pre, modelno / 100);
		break;
	default: /* give up */
		return NULL;
	}

	return buf;
}

#if defined(CONFIG_CMD_GSC) && !defined(CONFIG_SPL_BUILD)
static int do_gsc_sleep(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	unsigned char reg;
	unsigned long secs = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	secs = dectoul(argv[1], NULL);
	printf("GSC Sleeping for %ld seconds\n", secs);

	i2c_set_bus_num(0);
	reg = (secs >> 24) & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 9, 1, &reg, 1))
		goto error;
	reg = (secs >> 16) & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 8, 1, &reg, 1))
		goto error;
	reg = (secs >> 8) & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 7, 1, &reg, 1))
		goto error;
	reg = secs & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 6, 1, &reg, 1))
		goto error;
	if (gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
		goto error;
	reg |= (1 << 2);
	if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
		goto error;
	reg &= ~(1 << 2);
	reg |= 0x3;
	if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
		goto error;

	return CMD_RET_SUCCESS;

error:
	printf("i2c error\n");
	return CMD_RET_FAILURE;
}

static int do_gsc_wd(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	unsigned char reg;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcasecmp(argv[1], "enable") == 0) {
		int timeout = 0;

		if (argc > 2)
			timeout = dectoul(argv[2], NULL);
		i2c_set_bus_num(0);
		if (gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		reg &= ~((1 << GSC_SC_CTRL1_WDEN) | (1 << GSC_SC_CTRL1_WDTIME));
		if (timeout == 60)
			reg |= (1 << GSC_SC_CTRL1_WDTIME);
		else
			timeout = 30;
		reg |= (1 << GSC_SC_CTRL1_WDEN);
		if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		printf("GSC Watchdog enabled with timeout=%d seconds\n",
		       timeout);
	} else if (strcasecmp(argv[1], "disable") == 0) {
		i2c_set_bus_num(0);
		if (gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		reg &= ~((1 << GSC_SC_CTRL1_WDEN) | (1 << GSC_SC_CTRL1_WDTIME));
		if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		printf("GSC Watchdog disabled\n");
	} else {
		return CMD_RET_USAGE;
	}
	return CMD_RET_SUCCESS;
}

static int do_gsc(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 2)
		return gsc_info(1);

	if (strcasecmp(argv[1], "wd") == 0)
		return do_gsc_wd(cmdtp, flag, --argc, ++argv);
	else if (strcasecmp(argv[1], "sleep") == 0)
		return do_gsc_sleep(cmdtp, flag, --argc, ++argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	gsc, 4, 1, do_gsc, "GSC configuration",
	"[wd enable [30|60]]|[wd disable]|[sleep <secs>]\n"
	);

#endif /* CONFIG_CMD_GSC */
