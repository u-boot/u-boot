// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2022 NXP
 */
#include <config.h>
#include <command.h>
#include <i2c.h>
#include <linux/ctype.h>

#define DEV_ID_QSFP		0x0c
#define DEV_ID_QSFP_PLUS	0x0d

static struct __attribute__ ((__packed__)) qsfp_eeprom_map {
	struct __attribute__ ((__packed__)) qsfp_low_mem {
	/*	field			byte address */
	u8	identifier;		/* 0 */
	u16	status;			/* 1-2 */
	u8	reserved[124];		/* 3-126 */
	u8	page_select;		/* 127 */
	} base;

	struct __attribute__ ((__packed__)) qsfp_page00_mem {
	/*	field			byte address */
	u8	identifier;		/* 128 */
	u8	ext_identifier;		/* 129 */
	u8	connector;		/* 130 */
	u8	compat[8];		/* 131-138 */
	u8	reserved[3];		/* 139-142 */
	u8	length_fiber[4];
	u8	length_copper;		/* 146 */
	u8	tech;			/* 147 */
	u8	vendor_name[16];	/* 148-163 */
	u8	reserved2;		/* 164 */
	u8	oui[3];			/* 165-167 */
	u8	pn[16];			/* 168-183 */
	u8	rev[2];			/* 184-185 */
	u8	reserved3[10];		/* 186-195 */
	u8	serial[16];		/* 196-211 */
	u8	date[8];		/* 212-219 */
	u8	reserved4[35];		/* 220-255 */
	} page0;
} qsfp;

unsigned char get_qsfp_compat0(void)
{
	int ret;
	char vendor[20] = {0};
	char serial[20] = {0};
	char pname[20] = {0};
	char mfgdt[20] = {0};
#ifdef CONFIG_DM_I2C
	struct udevice *dev;
#endif

	memset(&qsfp, 0, sizeof(qsfp));
#ifndef CONFIG_DM_I2C
	ret = i2c_read(I2C_SFP_EEPROM_ADDR,
		       0,
		       I2C_SFP_EEPROM_ADDR_LEN,
		       (void *)&qsfp,
		       sizeof(qsfp));
#else
	ret = i2c_get_chip_for_busnum(0, I2C_SFP_EEPROM_ADDR, 1, &dev);
	if (!ret)
		ret = dm_i2c_read(dev, 0, (void *)&qsfp, sizeof(qsfp));
#endif

	if (ret != 0) {
		debug("\nQSFP: no module detected\n");
		return 0;
	}
	/* check if QSFP type */
	if (qsfp.base.identifier != DEV_ID_QSFP_PLUS) {
		debug("\nQSFP: unrecognized module\n");
		return 0;
	}

	/* copy fields and trim the whitespaces and dump on screen */
	snprintf(vendor, sizeof(vendor), "%.16s", qsfp.page0.vendor_name);
	snprintf(serial, sizeof(serial), "%.16s", qsfp.page0.serial);
	snprintf(pname, sizeof(pname), "%.16s", qsfp.page0.pn);
	snprintf(mfgdt, sizeof(mfgdt), "%.2s/%.2s/%.2s",
		 &qsfp.page0.date[0], &qsfp.page0.date[2], &qsfp.page0.date[4]);

	printf("QSFP: detected %s %s s/n: %s mfgdt: %s\n",
	       strim(vendor), strim(pname), strim(serial), strim(mfgdt));

	/* return ethernet compatibility code*/
	return qsfp.page0.compat[0];
}
