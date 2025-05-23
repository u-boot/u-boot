/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 Gateworks Corporation
 */

#ifndef _FSA_H_
#define _FSA_H_

#define FSA_MAX	5

enum fsa_gpio_cfg {
	FSA_GPIO_NC,
	FSA_GPIO_UNCONFIGURED,
	FSA_GPIO_INPUT,
	FSA_GPIO_OUTPUT_LOW,
	FSA_GPIO_OUTPUT_HIGH,
};

struct fsa_gpio_desc {
	u8 offset;
	u8 config;
	u8 source;
	char name[13];
};

struct fsa_board_info {
	char model[16];			/* 0x00: model string */
	u8 mac[6];			/* 0x10: MAC base */
	u8 macno;			/* 0x16: number of mac addrs */
	u8 resv1;			/* 0x17: reserved */
	u32 serial;			/* 0x18: Serial Number */
	u8 mfgdate[4];			/* 0x1c: MFG date */
	u8 sockgpios;			/* 0x20: number of socket gpio descriptors */
	u8 ioexpgpios;			/* 0x21: number of io expander gpio descriptors */
	u8 resv2[220];			/* 0x22: reserved */
	u8 chksum[2];			/* 0xfe: */
};

struct fsa_user_info {
	char desc[32];			/* 0x000: user description */
	char overlay[16];		/* 0x020: dt-overlay suffice */
	struct fsa_gpio_desc gpios[20];	/* 0x030: gpio descriptors */
	u8 reserved[398];		/* 0x170: reserved */
	u8 chksum[2];			/* 0x2fe: */
};

int fsa_init(void);
int fsa_show(void);
int fsa_ft_fixup(void *fdt);

#endif // _FSA_H_
