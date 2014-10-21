/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <asm/errno.h>
#include <common.h>
#include <i2c.h>
#include <linux/ctype.h>

#include "gsc.h"

#define MINMAX(n, percent)	((n)*(100-percent)/100), ((n)*(100+percent)/100)

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

#ifdef CONFIG_CMD_GSC
static void read_hwmon(const char *name, uint reg, uint size, uint low,
		       uint high)
{
	unsigned char buf[3];
	uint ui;

	printf("%-8s:", name);
	memset(buf, 0, sizeof(buf));
	if (gsc_i2c_read(GSC_HWMON_ADDR, reg, 1, buf, size)) {
		puts("fRD\n");
	} else {
		ui = buf[0] | (buf[1]<<8) | (buf[2]<<16);
		if (ui == 0xffffff)
			printf("invalid");
		else if (ui < low)
			printf("%d Failed - Low", ui);
		else if (ui > high)
			printf("%d Failed - High", ui);
		else
			printf("%d", ui);
	}
	puts("\n");
}

int do_gsc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *model = getenv("model");

	i2c_set_bus_num(0);
	read_hwmon("Temp",     GSC_HWMON_TEMP, 2, 0, 9000);
	read_hwmon("VIN",      GSC_HWMON_VIN, 3, 8000, 60000);
	read_hwmon("VBATT",    GSC_HWMON_VBATT, 3, 1800, 3500);
	read_hwmon("VDD_3P3",  GSC_HWMON_VDD_3P3, 3, MINMAX(3300, 10));
	read_hwmon("VDD_HIGH", GSC_HWMON_VDD_HIGH, 3, MINMAX(3000, 10));
	read_hwmon("VDD_DDR",  GSC_HWMON_VDD_DDR, 3, MINMAX(1500, 10));
	read_hwmon("VDD_5P0",  GSC_HWMON_VDD_5P0, 3, MINMAX(5000, 10));
	read_hwmon("VDD_2P5",  GSC_HWMON_VDD_2P5, 3, MINMAX(2500, 10));
	read_hwmon("VDD_1P8",  GSC_HWMON_VDD_1P8, 3, MINMAX(1800, 10));

	switch (model[3]) {
	case '1': /* GW51xx */
		read_hwmon("VDD_CORE", GSC_HWMON_VDD_CORE, 3, MINMAX(1175, 10));
		read_hwmon("VDD_SOC",  GSC_HWMON_VDD_SOC, 3, MINMAX(1175, 10));
		break;
	case '2': /* GW52xx */
	case '3': /* GW53xx */
		read_hwmon("VDD_CORE", GSC_HWMON_VDD_CORE, 3, MINMAX(1175, 10));
		read_hwmon("VDD_SOC",  GSC_HWMON_VDD_SOC, 3, MINMAX(1175, 10));
		read_hwmon("VDD_1P0",  GSC_HWMON_VDD_1P0, 3, MINMAX(1000, 10));
		break;
	case '4': /* GW54xx */
		read_hwmon("VDD_CORE", GSC_HWMON_VDD_CORE, 3, MINMAX(1375, 10));
		read_hwmon("VDD_SOC",  GSC_HWMON_VDD_SOC, 3, MINMAX(1375, 10));
		read_hwmon("VDD_1P0",  GSC_HWMON_VDD_1P0, 3, MINMAX(1000, 10));
		break;
	case '5': /* GW55xx */
		read_hwmon("VDD_CORE", GSC_HWMON_VDD_CORE, 3, MINMAX(1175, 10));
		read_hwmon("VDD_SOC",  GSC_HWMON_VDD_SOC, 3, MINMAX(1175, 10));
		break;
	}
	return 0;
}

U_BOOT_CMD(gsc, 1, 1, do_gsc,
	   "GSC test",
	   ""
);

#endif /* CONFIG_CMD_GSC */
