/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Gateworks Corporation
 */

#ifndef _GSC_H_
#define _GSC_H_

/* I2C slave addresses */
#define GSC_SC_ADDR             0x20
#define GSC_RTC_ADDR            0x68
#define GSC_HWMON_ADDR          0x29
#define GSC_EEPROM_ADDR         0x51

struct venice_board_info {
	u8 mac[6];		/* 0x00: MAC base */
	char equiv_dts[16];	/* 0x06: equivalent device-tree */
	u8 res0[2];	/* 0x16: reserved */
	u32 serial;	/* 0x18: Serial Number */
	u8 res1[4];	/* 0x1C: reserved */
	u8 mfgdate[4];	/* 0x20: MFG date */
	u8 macno;		/* 0x24: number of mac addrs */
	u8 res2[6];	/* 0x25 */
	u8 sdram_size;	/* 0x2B: (16 << n) MB */
	u8 sdram_speed;	/* 0x2C: (33.333 * n) MHz */
	u8 sdram_width;	/* 0x2D: (8 << n) bit */
	u8 res3[2];	/* 0x2E */
	char model[16];		/* 0x30: model string */
	u8 res4[14];	/* 0x40 */
	u8 chksum[2];	/* 0x4E */
};

int gsc_init(int quiet);
int gsc_hwmon(void);
const char *gsc_get_model(void);
const char *gsc_get_dtb_name(int level, char *buf, int len);
int gsc_getmac(int index, uint8_t *enetaddr);

#endif
