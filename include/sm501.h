/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * (C) Copyright 2005
 * Martin Krause TQ-Systems GmbH martin.krause@tqs.de
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

/*
 * Basic video support for SMI SM501 "Voyager" graphic controller
 */

#ifndef _SM501_H_
#define _SM501_H_

#define PCI_VENDOR_SM		0x126f
#define PCI_DEVICE_SM501	0x0501

typedef struct {
	unsigned int Index;
	unsigned int Value;
} SMI_REGS;

/* Board specific functions                                                  */
unsigned int board_video_init (void);
void board_validate_screen (unsigned int base);
const SMI_REGS *board_get_regs (void);
int board_get_width (void);
int board_get_height (void);
unsigned int board_video_get_fb (void);

#endif /* _SM501_H_ */
