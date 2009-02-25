/*
 * (C) Copyright 2009 mGine co.
 * unsik Kim <donari75@gmail.com>
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

#ifndef MG_DISK_H_
#define MG_DISK_H_

#include <asm/types.h>

/* private driver data */
struct mg_drv_data {
	/* base address of mflash */
	u32 base;
	/* Initialize hard reset, write protect, deep power down pins.
	 * Set these pins to GPIO and output high
	 */
	void (*mg_ctrl_pin_init) (void);
	/* Set hard reset pin for given level
	 * level : logical level of hard reset pin (0 or 1)
	 */
	void (*mg_hdrst_pin) (u8 level);
};

struct mg_drv_data* mg_get_drv_data (void);

unsigned int mg_disk_init (void);
unsigned int mg_disk_read (u32 addr, u8 *buff, u32 len);
unsigned int mg_disk_write(u32 addr, u8 *buff, u32 len);
unsigned int mg_disk_write_sects(void *buff, u32 sect_num, u32 sect_cnt);
unsigned int mg_disk_read_sects(void *buff, u32 sect_num, u32 sect_cnt);

#endif /*MG_DISK_H_*/
