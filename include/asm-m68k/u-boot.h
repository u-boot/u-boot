/*
 * (C) Copyright 2000 - 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __U_BOOT_H__
#define __U_BOOT_H__

/*
 * Board information passed to Linux kernel from U-Boot
 *
 * include/asm-ppc/u-boot.h
 */

#ifndef __ASSEMBLY__

typedef struct bd_info {
	unsigned long bi_memstart;	/* start of DRAM memory */
	unsigned long bi_memsize;	/* size  of DRAM memory in bytes */
	unsigned long bi_flashstart;	/* start of FLASH memory */
	unsigned long bi_flashsize;	/* size  of FLASH memory */
	unsigned long bi_flashoffset;	/* reserved area for startup monitor */
	unsigned long bi_sramstart;	/* start of SRAM memory */
	unsigned long bi_sramsize;	/* size  of SRAM memory */
	unsigned long bi_mbar_base;	/* base of internal registers */
	unsigned long bi_bootflags;	/* boot / reboot flag (for LynxOS) */
	unsigned long bi_boot_params;	/* where this board expects params */
	unsigned long bi_ip_addr;	/* IP Address */
	unsigned char bi_enetaddr[6];	/* Ethernet adress */
	unsigned short bi_ethspeed;	/* Ethernet speed in Mbps */
	unsigned long bi_intfreq;	/* Internal Freq, in MHz */
	unsigned long bi_busfreq;	/* Bus Freq, in MHz */
#ifdef CONFIG_PCI
	unsigned long bi_pcifreq;	/* pci Freq in MHz */
#endif
#ifdef CONFIG_EXTRA_CLOCK
	unsigned long bi_inpfreq;	/* input Freq in MHz */
	unsigned long bi_vcofreq;	/* vco Freq in MHz */
	unsigned long bi_flbfreq;	/* Flexbus Freq in MHz */
#endif
	unsigned long bi_baudrate;	/* Console Baudrate */

#ifdef CONFIG_HAS_ETH1
	/* second onboard ethernet port */
	unsigned char bi_enet1addr[6];
#endif
#ifdef CONFIG_HAS_ETH2
	/* third onboard ethernet port */
	unsigned char bi_enet2addr[6];
#endif
#ifdef CONFIG_HAS_ETH3
	unsigned char bi_enet3addr[6];
#endif
} bd_t;

#endif				/* __ASSEMBLY__ */

#endif				/* __U_BOOT_H__ */
