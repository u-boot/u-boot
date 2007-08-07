/*
 * (C) Copyright 2007 Schindler Lift Inc.
 *
 * Author: Michel Marti <mma@objectxp.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __FW_UPDATE_H
#define __FW_UPDATE_H

/* Default prefix for output messages */
#define LOG_PREFIX	"CM5200:"

/* Extra debug macro */
#ifdef CONFIG_FWUPDATE_DEBUG
#define FW_DEBUG(fmt...) printf(LOG_PREFIX fmt)
#else
#define FW_DEBUG(fmt...)
#endif

/* Name of the directory holding firmware images */
#define FW_DIR		"nx-fw"
#define RESCUE_IMAGE	"nxrs.img"
#define LOAD_ADDR	0x400000
#define RS_BOOTARGS	"ramdisk=8192K"

/* Main function for fwupdate */
void cm5200_fwupdate(void);

#endif /* __FW_UPDATE_H */
