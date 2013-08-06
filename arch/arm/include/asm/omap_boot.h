/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Sricharan R <r.sricharan@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* ROM code defines */
/* Boot device */
#define BOOT_DEVICE_MASK	0xFF
#define BOOT_DEVICE_OFFSET	0x8
#define DEV_DESC_PTR_OFFSET	0x4
#define DEV_DATA_PTR_OFFSET	0x18
#define BOOT_MODE_OFFSET	0x8
#define RESET_REASON_OFFSET	0x9
#define CH_FLAGS_OFFSET		0xA

#define CH_FLAGS_CHSETTINGS	(0x1 << 0)
#define CH_FLAGS_CHRAM		(0x1 << 1)
#define CH_FLAGS_CHFLASH	(0x1 << 2)
#define CH_FLAGS_CHMMCSD	(0x1 << 3)

#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	char *boot_message;
	unsigned int mem_boot_descriptor;
	unsigned char omap_bootdevice;
	unsigned char reset_reason;
	unsigned char ch_flags;
	unsigned long omap_bootmode;
};
#endif
