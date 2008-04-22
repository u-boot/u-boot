/*
 * (C) Copyright 2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
