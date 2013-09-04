/*
 * Common board functions for siemens AM335X based boards
 * (C) Copyright 2013 Siemens Schweiz AG
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FACTORYSET_H
#define __FACTORYSET_H

#define MAX_STRING_LENGTH	32

struct factorysetcontainer {
	uchar mac[6];
	int usb_vendor_id;
	int usb_product_id;
	int pxm50;
#if defined(CONFIG_VIDEO)
	unsigned char disp_name[MAX_STRING_LENGTH];
#endif
};

int factoryset_read_eeprom(int i2c_addr);
int factoryset_setenv(void);
extern struct factorysetcontainer factory_dat;

#endif /* __FACTORYSET_H */
