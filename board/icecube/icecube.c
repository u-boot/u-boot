/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <mpc5xxx.h>

long int initdram (int board_type)
{
#ifndef CFG_RAMBOOT
	/* configure SDRAM start/end */
#if defined(CONFIG_MPC5200)
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x00000018;/* 32M at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x02000000;/* disabled */

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = 0xc2233a00;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = 0x88b70004;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0000;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0002;
	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x408d0000;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0002;
	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0004;
	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x008d0000;
	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0x504f0000;
#elif defined(CONFIG_MGT5100)
	*(vu_long *)MPC5XXX_SDRAM_START = 0x00000000;
	*(vu_long *)MPC5XXX_SDRAM_STOP = 0x000007ff;/* 64M */
	*(vu_long *)MPC5XXX_ADDECR |= (1 << 22); /* Enable SDRAM */

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = 0xc2222600;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = 0x88b70004;

	/* address select register */
	*(vu_long *)MPC5XXX_SDRAM_XLBSEL = 0x03000000;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd14f0000;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd14f0002;
	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x008d0000;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd14f0002;
	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd14f0004;
	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x008d0000;
	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0x514f0000;
#endif
#else
#ifdef CONFIG_MGT5100
	*(vu_long *)MPC5XXX_ADDECR |= (1 << 22); /* Enable SDRAM */
#endif
#endif
	/* return total ram size */
#if defined(CONFIG_MGT5100)
	return (64 * 1024 * 1024);
#elif defined(CONFIG_MPC5200)
	return (32 * 1024 * 1024);
#endif
}

int checkboard (void)
{
#if defined(CONFIG_MPC5200)
	puts ("Board: Motorola MPC5200 (IceCube)\n");
#elif defined(CONFIG_MGT5100)
	puts ("Board: Motorola MGT5100 (IceCube)\n");
#endif
	return 0;
}

void flash_preinit(void)
{
	/*
	 * Now, when we are in RAM, enable flash write
	 * access for detection process.
	 * Note that CS_BOOT cannot be cleared when
	 * executing in flash.
	 */
#if defined(CONFIG_MGT5100)
	*(vu_long *)MPC5XXX_ADDECR &= ~(1 << 25); /* disable CS_BOOT */
	*(vu_long *)MPC5XXX_ADDECR |= (1 << 16); /* enable CS0 */
#endif
	*(vu_long *)MPC5XXX_BOOTCS_CFG &= ~0x1; /* clear RO */
}
