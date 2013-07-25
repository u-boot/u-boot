/*
 * (C) Copyright 2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _AUTO_UPDATE_H_
#define _AUTO_UPDATE_H_

#define MBR_MAGIC       0x07081967
#define MBR_MAGIC_ADDR  0x100           /* offset 0x100 should be free space */

#define AU_MAGIC_FILE   "__auto_update"

#define AU_TYPEMASK     0x000000ff
#define AU_FLAGMASK     0xffff0000

#define AU_PROTECT      0x80000000

#define AU_SCRIPT       0x01
#define AU_FIRMWARE     (0x02 | AU_PROTECT)
#define AU_NOR          0x03
#define AU_NAND         0x04

struct au_image_s {
	char name[80];
	ulong start;
	ulong size;
	ulong type;
};

typedef struct au_image_s au_image_t;

int do_auto_update(void);
#ifdef CONFIG_AUTO_UPDATE_SHOW
void board_auto_update_show(int au_active);
#endif

#endif /* #ifndef _AUTO_UPDATE_H_ */
