// SPDX-License-Identifier: GPL-2.0+
/*
 * Support for booting from coreboot
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY	UCLASS_RTC

#include <command.h>
#include <dm.h>
#include <rtc.h>
#include <asm/cb_sysinfo.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

const struct sysinfo_t *get_table(void)
{
	if (!gd->arch.coreboot_table) {
		printf("No coreboot sysinfo table found\n");
		return NULL;
	}

	return &lib_sysinfo;
}

static int calc_sum(struct udevice *dev, uint start_bit, uint bit_count)
{
	uint start_byte = start_bit / 8;
	uint byte_count = bit_count / 8;
	int ret, i;
	uint sum;

	log_debug("Calc sum from %x: %x bytes\n", start_byte, byte_count);
	sum = 0;
	for (i = 0; i < bit_count / 8; i++) {
		ret = rtc_read8(dev, start_bit / 8 + i);
		if (ret < 0)
			return ret;
		sum += ret;
	}

	return (sum & 0xff) << 8 | (sum & 0xff00) >> 8;
}

/**
 * prep_cbcmos() - Prepare for a CMOS-RAM command
 *
 * @tab: coreboot table
 * @devnum: RTC device name to use, or NULL for the first one
 * @dep: Returns RTC device on success
 * Return: calculated checksum for CMOS RAM or -ve on error
 */
static int prep_cbcmos(const struct sysinfo_t *tab, const char *devname,
		       struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	if (!tab)
		return CMD_RET_FAILURE;
	if (devname)
		ret = uclass_get_device_by_name(UCLASS_RTC, devname, &dev);
	else
		ret = uclass_first_device_err(UCLASS_RTC, &dev);
	if (ret) {
		printf("Failed to get RTC device: %dE\n", ret);
		return ret;
	}

	ret = calc_sum(dev, tab->cmos_range_start,
		       tab->cmos_range_end + 1 - tab->cmos_range_start);
	if (ret < 0) {
		printf("Failed to read RTC device: %dE\n", ret);
		return ret;
	}
	*devp = dev;

	return ret;
}

static int do_cbcmos_check(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	const struct sysinfo_t *tab = get_table();
	struct udevice *dev;
	u16 cur, sum;
	int ret;

	ret = prep_cbcmos(tab, argv[1], &dev);
	if (ret < 0)
		return CMD_RET_FAILURE;
	sum = ret;

	ret = rtc_read16(dev, tab->cmos_checksum_location / 8, &cur);
	if (ret < 0) {
		printf("Failed to read RTC device: %dE\n", ret);
		return CMD_RET_FAILURE;
	}
	if (sum != cur) {
		printf("Checksum %04x error: calculated %04x\n", cur, sum);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_cbcmos_update(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	const struct sysinfo_t *tab = get_table();
	struct udevice *dev;
	u16 sum;
	int ret;

	ret = prep_cbcmos(tab, argv[1], &dev);
	if (ret < 0)
		return CMD_RET_FAILURE;
	sum = ret;

	ret = rtc_write16(dev, tab->cmos_checksum_location / 8, sum);
	if (ret < 0) {
		printf("Failed to read RTC device: %dE\n", ret);
		return CMD_RET_FAILURE;
	}
	printf("Checksum %04x written\n", sum);

	return 0;
}

U_BOOT_LONGHELP(cbcmos,
	"check     - check CMOS RAM\n"
	"cbcmos update    - Update CMOS-RAM checksum";
);

U_BOOT_CMD_WITH_SUBCMDS(cbcmos, "coreboot CMOS RAM", cbcmos_help_text,
	U_BOOT_SUBCMD_MKENT(check, 2, 1, do_cbcmos_check),
	U_BOOT_SUBCMD_MKENT(update, 2, 1, do_cbcmos_update));
