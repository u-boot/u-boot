/*
 * Copyright 2011 Linaro Limited
 * Aneesh V <aneesh@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/setup.h>
#include <asm/arch/sys_proto.h>
static void do_cancel_out(u32 *num, u32 *den, u32 factor)
{
	while (1) {
		if (((*num)/factor*factor == (*num)) &&
		   ((*den)/factor*factor == (*den))) {
			(*num) /= factor;
			(*den) /= factor;
		} else
			break;
	}
}

#ifdef CONFIG_FASTBOOT_FLASH
static void omap_set_fastboot_cpu(void)
{
	char *cpu;
	u32 cpu_rev = omap_revision();

	switch (cpu_rev) {
	case DRA752_ES1_0:
	case DRA752_ES1_1:
	case DRA752_ES2_0:
		cpu = "DRA752";
		break;
	case DRA722_ES1_0:
	case DRA722_ES2_0:
		cpu = "DRA722";
		break;
	default:
		cpu = NULL;
		printf("Warning: fastboot.cpu: unknown CPU rev: %u\n", cpu_rev);
	}

	setenv("fastboot.cpu", cpu);
}

static void omap_set_fastboot_secure(void)
{
	const char *secure;
	u32 dev = get_device_type();

	switch (dev) {
	case EMU_DEVICE:
		secure = "EMU";
		break;
	case HS_DEVICE:
		secure = "HS";
		break;
	case GP_DEVICE:
		secure = "GP";
		break;
	default:
		secure = NULL;
		printf("Warning: fastboot.secure: unknown CPU sec: %u\n", dev);
	}

	setenv("fastboot.secure", secure);
}

static void omap_set_fastboot_board_rev(void)
{
	const char *board_rev;

	board_rev = getenv("board_rev");
	if (board_rev == NULL)
		printf("Warning: fastboot.board_rev: unknown board revision\n");

	setenv("fastboot.board_rev", board_rev);
}

#ifdef CONFIG_FASTBOOT_FLASH_MMC_DEV
static u32 omap_mmc_get_part_size(const char *part)
{
	int res;
	struct blk_desc *dev_desc;
	disk_partition_t info;
	u64 sz = 0;

	dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
		error("invalid mmc device\n");
		return 0;
	}

	res = part_get_info_by_name(dev_desc, part, &info);
	if (res < 0) {
		error("cannot find partition: '%s'\n", part);
		return 0;
	}

	/* Calculate size in bytes */
	sz = (info.size * (u64)info.blksz);
	/* to KiB */
	sz >>= 10;

	return (u32)sz;
}

static void omap_set_fastboot_userdata_size(void)
{
	char buf[16];
	u32 sz_kb;

	sz_kb = omap_mmc_get_part_size("userdata");
	if (sz_kb == 0) {
		buf[0] = '\0';
		printf("Warning: fastboot.userdata_size: unable to calc\n");
	} else {
		sprintf(buf, "%u", sz_kb);
	}

	setenv("fastboot.userdata_size", buf);
}
#else
static inline void omap_set_fastboot_userdata_size(void)
{
}
#endif /* CONFIG_FASTBOOT_FLASH_MMC_DEV */
void omap_set_fastboot_vars(void)
{
	omap_set_fastboot_cpu();
	omap_set_fastboot_secure();
	omap_set_fastboot_board_rev();
	omap_set_fastboot_userdata_size();
}
#endif /* CONFIG_FASTBOOT_FLASH */

/*
 * Cancel out the denominator and numerator of a fraction
 * to get smaller numerator and denominator.
 */
void cancel_out(u32 *num, u32 *den, u32 den_limit)
{
	do_cancel_out(num, den, 2);
	do_cancel_out(num, den, 3);
	do_cancel_out(num, den, 5);
	do_cancel_out(num, den, 7);
	do_cancel_out(num, den, 11);
	do_cancel_out(num, den, 13);
	do_cancel_out(num, den, 17);
	while ((*den) > den_limit) {
		*num /= 2;
		/*
		 * Round up the denominator so that the final fraction
		 * (num/den) is always <= the desired value
		 */
		*den = (*den + 1) / 2;
	}
}

__weak void omap_die_id(unsigned int *die_id)
{
	die_id[0] = die_id[1] = die_id[2] = die_id[3] = 0;
}

void omap_die_id_serial(void)
{
	unsigned int die_id[4] = { 0 };
	char serial_string[17] = { 0 };

	omap_die_id((unsigned int *)&die_id);

	if (!getenv("serial#")) {
		snprintf(serial_string, sizeof(serial_string),
			"%08x%08x", die_id[0], die_id[3]);

		setenv("serial#", serial_string);
	}
}

void omap_die_id_get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	unsigned long long serial;

	serial_string = getenv("serial#");

	if (serial_string) {
		serial = simple_strtoull(serial_string, NULL, 16);

		serialnr->high = (unsigned int) (serial >> 32);
		serialnr->low = (unsigned int) (serial & 0xffffffff);
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}

void omap_die_id_usbethaddr(void)
{
	unsigned int die_id[4] = { 0 };
	unsigned char mac[6] = { 0 };

	omap_die_id((unsigned int *)&die_id);

	if (!getenv("usbethaddr")) {
		/*
		 * Create a fake MAC address from the processor ID code.
		 * First byte is 0x02 to signify locally administered.
		 */
		mac[0] = 0x02;
		mac[1] = die_id[3] & 0xff;
		mac[2] = die_id[2] & 0xff;
		mac[3] = die_id[1] & 0xff;
		mac[4] = die_id[0] & 0xff;
		mac[5] = (die_id[0] >> 8) & 0xff;

		eth_setenv_enetaddr("usbethaddr", mac);
	}
}

void omap_die_id_display(void)
{
	unsigned int die_id[4] = { 0 };

	omap_die_id(die_id);

	printf("OMAP die ID: %08x%08x%08x%08x\n", die_id[3], die_id[2],
		die_id[1], die_id[0]);
}
