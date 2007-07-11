/*
 * (C) Copyright 2003-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2004-2005
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
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

#include <common.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <asm/processor.h>
#include <i2c.h>
#ifdef CONFIG_OF_FLAT_TREE
#include <ft_build.h>
#endif /* CONFIG_OF_FLAT_TREE */

#include "fwupdate.h"

#ifndef CFG_RAMBOOT
/*
 * Helper function to initialize SDRAM controller.
 */
static void sdram_start(int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000000 |
						hi_addr_bit;

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 |
						hi_addr_bit;

	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000004 |
						hi_addr_bit;

	/* auto refresh, second time */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000004 |
						hi_addr_bit;

	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE;

	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | hi_addr_bit;
}
#endif /* CFG_RAMBOOT */

/*
 * Initalize SDRAM - configure SDRAM controller, detect memory size.
 */
long int initdram(int board_type)
{
	ulong dramsize = 0;
#ifndef CFG_RAMBOOT
	ulong test1, test2;

	/* configure SDRAM start/end for detection */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001e; /* 2G at 0x0 */

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = SDRAM_CONFIG1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = SDRAM_CONFIG2;

	sdram_start(0);
	test1 = get_ram_size((long *)CFG_SDRAM_BASE, 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CFG_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else
		dramsize = test2;

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20))
		dramsize = 0;

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 +
			__builtin_ffs(dramsize >> 20) - 1;
	} else
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */
#else /* CFG_RAMBOOT */
	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = *(vu_long *)MPC5XXX_SDRAM_CS0CFG & 0xFF;
	if (dramsize >= 0x13)
		dramsize = (1 << (dramsize - 0x13)) << 20;
	else
		dramsize = 0;
#endif /* CFG_RAMBOOT */

	/*
	 * On MPC5200B we need to set the special configuration delay in the
	 * DDR controller.  Refer to chapter 8.7.5 SDelay--MBAR + 0x0190 of
	 * the MPC5200B User's Manual.
	 */
	*(vu_long *)MPC5XXX_SDRAM_SDELAY = 0x04;
	__asm__ volatile ("sync");

	return dramsize;
}


int checkboard(void)
{
	puts("Board: CM1.QP1\n");
	return 0;
}


int board_early_init_r(void)
{
	/*
	 * Now, when we are in RAM, enable flash write access for detection
	 * process. Note that CS_BOOT cannot be cleared when executing in
	 * flash.
	 */
	*(vu_long *)MPC5XXX_BOOTCS_CFG &= ~0x1; /* clear RO */
	return 0;
}


#ifdef CONFIG_POST
int post_hotkeys_pressed(void)
{
	return 0;
}
#endif /* CONFIG_POST */


#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
void post_word_store(ulong a)
{
	vu_long *save_addr = (vu_long *)(MPC5XXX_SRAM + MPC5XXX_SRAM_POST_SIZE);
	*save_addr = a;
}


ulong post_word_load(void)
{
	vu_long *save_addr = (vu_long *)(MPC5XXX_SRAM + MPC5XXX_SRAM_POST_SIZE);
	return *save_addr;
}
#endif /* CONFIG_POST || CONFIG_LOGBUFFER */


#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	uchar buf[6];
	char str[18];

	/* Read ethaddr from EEPROM */
	if (i2c_read(CFG_I2C_EEPROM, CONFIG_MAC_OFFSET, 2, buf, 6) == 0) {
		sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		/* Check if MAC addr is owned by Schindler */
		if (strstr(str, "00:06:C3") != str) {
			printf(LOG_PREFIX "Warning - Illegal MAC address (%s)"
				" in EEPROM.\n", str);
			printf(LOG_PREFIX "Using MAC from environment\n");
		} else {
			printf(LOG_PREFIX "Using MAC (%s) from I2C EEPROM\n",
				str);
			setenv("ethaddr", str);
		}
	} else {
		printf(LOG_PREFIX "Warning - Unable to read MAC from I2C"
			" device at address %02X:%04X\n", CFG_I2C_EEPROM,
			CONFIG_MAC_OFFSET);
		printf(LOG_PREFIX "Using MAC from environment\n");
	}
	return 0;
#endif /* defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C) */
}
#endif /* CONFIG_MISC_INIT_R */


#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
#ifdef CONFIG_USB_STORAGE
	cm1_fwupdate();
#endif /* CONFIG_USB_STORAGE */
	return 0;
}
#endif /* CONFIG_LAST_STAGE_INIT */


#if defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP) */
