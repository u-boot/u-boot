/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@freescale.com.
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

#include "sdram.h"

#ifndef CONFIG_SYS_RAMBOOT
static void mpc5xxx_sdram_start (sdram_conf_t *sdram_conf, int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = sdram_conf->control | 0x80000000 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = sdram_conf->control | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

	if (sdram_conf->ddr) {
		/* set mode register: extended mode */
		*(vu_long *)MPC5XXX_SDRAM_MODE = sdram_conf->emode;
		__asm__ volatile ("sync");

		/* set mode register: reset DLL */
		*(vu_long *)MPC5XXX_SDRAM_MODE = sdram_conf->mode | 0x04000000;
		__asm__ volatile ("sync");
	}

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = sdram_conf->control | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = sdram_conf->control | 0x80000004 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = sdram_conf->mode;
	__asm__ volatile ("sync");

	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = sdram_conf->control | hi_addr_bit;
	__asm__ volatile ("sync");
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *            use of CONFIG_SYS_SDRAM_BASE. The code does not work if CONFIG_SYS_SDRAM_BASE
 *            is something else than 0x00000000.
 */

long int mpc5xxx_sdram_init (sdram_conf_t *sdram_conf)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001e;/* 2G at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x80000000;/* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = sdram_conf->config1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = sdram_conf->config2;
	__asm__ volatile ("sync");

	if (sdram_conf->ddr) {
		/* set tap delay */
		*(vu_long *)MPC5XXX_CDM_PORCFG = sdram_conf->tapdelay;
		__asm__ volatile ("sync");
	}

	/* find RAM size using SDRAM CS0 only */
	mpc5xxx_sdram_start(sdram_conf, 0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	mpc5xxx_sdram_start(sdram_conf, 1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		mpc5xxx_sdram_start(sdram_conf, 0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20)) {
		dramsize = 0;
	}

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 + __builtin_ffs(dramsize >> 20) - 1;
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */
	}

	/* let SDRAM CS1 start right after CS0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize + 0x0000001e;/* 2G */

	/* find RAM size using SDRAM CS1 only */
	mpc5xxx_sdram_start(sdram_conf, 0);
	test1 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x80000000);
	mpc5xxx_sdram_start(sdram_conf, 1);
	test2 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x80000000);
	if (test1 > test2) {
		mpc5xxx_sdram_start(sdram_conf, 0);
		dramsize2 = test1;
	} else {
		dramsize2 = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20)) {
		dramsize2 = 0;
	}

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize
			| (0x13 + __builtin_ffs(dramsize2 >> 20) - 1);
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize; /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = *(vu_long *)MPC5XXX_SDRAM_CS0CFG & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = *(vu_long *)MPC5XXX_SDRAM_CS1CFG & 0xFF;
	if (dramsize2 >= 0x13) {
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	} else {
		dramsize2 = 0;
	}

#endif /* CONFIG_SYS_RAMBOOT */

	return dramsize + dramsize2;
}
