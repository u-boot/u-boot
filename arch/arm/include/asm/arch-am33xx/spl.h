/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
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
#ifndef	_ASM_ARCH_SPL_H_
#define	_ASM_SPL_H_

#define BOOT_DEVICE_XIP       	2
#define BOOT_DEVICE_NAND	5
#ifdef CONFIG_AM33XX
#define BOOT_DEVICE_MMC1	8
#define BOOT_DEVICE_MMC2	9	/* eMMC or daughter card */
#elif defined(CONFIG_TI814X)
#define BOOT_DEVICE_MMC1	9
#define BOOT_DEVICE_MMC2	8	/* ROM only supports 2nd instance */
#endif
#define BOOT_DEVICE_SPI		11
#define BOOT_DEVICE_UART	65
#define BOOT_DEVICE_USBETH	68
#define BOOT_DEVICE_CPGMAC	70
#define BOOT_DEVICE_MMC2_2      0xFF

#ifdef CONFIG_AM33XX
#define MMC_BOOT_DEVICES_START	BOOT_DEVICE_MMC1
#define MMC_BOOT_DEVICES_END	BOOT_DEVICE_MMC2
#elif defined(CONFIG_TI814X)
#define MMC_BOOT_DEVICES_START	BOOT_DEVICE_MMC2
#define MMC_BOOT_DEVICES_END	BOOT_DEVICE_MMC1
#endif
#endif
