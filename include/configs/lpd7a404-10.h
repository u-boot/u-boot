/*
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
 * Logic LH7A400-10 card engine
 */

#ifndef __LPD7A404_10_H
#define __LPD7A404_10_H


#define CONFIG_ARM920T		1	/* arm920t core */
#define CONFIG_LH7A40X		1	/* Sharp LH7A40x SoC family */
#define CONFIG_LH7A404		1   /* Sharp LH7A404 SoC */

/* The system clock PLL input frequency */
#define CONFIG_SYS_CLK_FREQ		14745600   /* System Clock PLL Input (Hz) */

/* ticks per second */
#define CFG_HZ	(508469)

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0xc0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */

#define CFG_FLASH_BASE		0x00000000 /* Flash Bank #1 */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(64)	/* max number of sectors on one chip */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

/*----------------------------------------------------------------------
 * Using SMC91C111 LAN chip
 *
 * Default IO base of chip is 0x300, Card Engine has this address lines
 * (LAN chip) tied to Vcc, so we just care about the chip select
 */
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC91111_BASE	(0x70000000)
#undef CONFIG_SMC_USE_32_BIT
#define CONFIG_SMC_USE_IOFUNCS

#endif  /* __LPD7A404_10_H */
