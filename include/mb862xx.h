/*
 * (C) Copyright 2007
 * DENX Software Engineering, Anatolij Gustschin, agust@denx.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * mb862xx.h - Graphic interface for Fujitsu CoralP/Lime
 */

#ifndef _MB862XX_H_
#define _MB862XX_H_

#define PCI_VENDOR_ID_FUJITSU	0x10CF
#define PCI_DEVICE_ID_CORAL_P	0x2019
#define PCI_DEVICE_ID_CORAL_PA	0x201E

typedef struct {
	unsigned int index;
	unsigned int value;
} gdc_regs;

const gdc_regs *board_get_regs (void);
unsigned int board_video_init (void);
void board_backlight_switch(int);

#endif /* _MB862XX_H_ */
