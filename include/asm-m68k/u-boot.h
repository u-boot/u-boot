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
 */

#ifndef __U_BOOT_H__
#define __U_BOOT_H__

/*
 * Board information passed to Linux kernel from U-Boot
 *
 * include/asm-ppc/u-boot.h
 */

#ifndef __ASSEMBLY__
#include <linux/types.h>

typedef struct bd_info {
	unsigned long	bi_memstart;	/* start of DRAM memory */
	unsigned long	bi_memsize;	/* size	 of DRAM memory in bytes */
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size	 of FLASH memory */
	unsigned long	bi_flashoffset; /* reserved area for startup monitor */
	unsigned long	bi_sramstart;	/* start of SRAM memory */
	unsigned long	bi_sramsize;	/* size	 of SRAM memory */
	unsigned long	bi_bootflags;	/* boot / reboot flag (for LynxOS) */
	unsigned long	bi_boot_params; /* where this board expects params */
	unsigned long	bi_ip_addr;	/* IP Address */
	unsigned char	bi_enetaddr[6]; /* Ethernet adress */
	unsigned short	bi_ethspeed;	/* Ethernet speed in Mbps */
	unsigned long	bi_intfreq;	/* Internal Freq, in MHz */
	unsigned long	bi_busfreq;	/* Bus Freq, in MHz */
	unsigned long	bi_baudrate;	/* Console Baudrate */
} bd_t;

#endif /* __ASSEMBLY__ */
/* The following data structure is placed in DPRAM to allow for a
 * minimum set of global variables during system initialization
 * (until we have set up the memory controller so that we can use
 * RAM).
 *
 * Keep it *SMALL* and remember to set CFG_INIT_DATA_SIZE > sizeof(init_data_t)
 */
typedef struct	init_data {
	unsigned long	cpu_clk;	/* VCOOUT = CPU clock in Hz!		*/
	unsigned long	env_addr;	/* Address  of Environment struct	*/
	unsigned long	env_valid;	/* Checksum of Environment valid?	*/
	unsigned long	relocated;	/* Relocat. offset when running in RAM	*/
	unsigned long	have_console;	/* serial_init() was called		*/
#ifdef CONFIG_LCD
	unsigned long	lcd_base;	/* Base address of LCD frambuffer mem	*/
#endif
} init_data_t;
#endif	/* __U_BOOT_H__ */
