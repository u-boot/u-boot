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
#include <pci.h>

#ifndef CFG_RAMBOOT
static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

#ifdef CONFIG_MPC5200_DDR
	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xf05f0f00 | hi_addr_bit;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xf05f0f02 | hi_addr_bit;
	/* set mode register: extended mode */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x40090000;
	/* set mode register: reset DLL */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x058d0000;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xf05f0f02 | hi_addr_bit;
	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xf05f0f04 | hi_addr_bit;
	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x018d0000;
	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0x705f0f00 | hi_addr_bit;
#else
	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0000 | hi_addr_bit;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0002 | hi_addr_bit;
	/* set mode register */
#if defined(CONFIG_MPC5200)
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x408d0000;
#elif defined(CONFIG_MGT5100)
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x008d0000;
#endif
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0002 | hi_addr_bit;
	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0xd04f0004 | hi_addr_bit;
	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = 0x008d0000;
	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = 0x504f0000 | hi_addr_bit;
#endif
}
#endif

long int initdram (int board_type)
{
	ulong dramsize = 0;
#ifdef CONFIG_MPC5200_DDR
	ulong dramsize2 = 0;
#endif
#ifndef CFG_RAMBOOT
	ulong test1, test2;

	/* configure SDRAM start/end */
#if defined(CONFIG_MPC5200)
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001e;/* 2G at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x80000000;/* disabled */

#ifdef CONFIG_MPC5200_DDR
	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = 0x73722930;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = 0x47770000;

	/* set tap delay to 0x10 */
	*(vu_long *)MPC5XXX_CDM_PORCFG = 0x10000000;
#else
	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = 0xc2233a00;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = 0x88b70004;
#endif

#elif defined(CONFIG_MGT5100)
	*(vu_long *)MPC5XXX_SDRAM_START = 0x00000000;
	*(vu_long *)MPC5XXX_SDRAM_STOP = 0x0000ffff;/* 2G */
	*(vu_long *)MPC5XXX_ADDECR |= (1 << 22); /* Enable SDRAM */

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = 0xc2222600;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = 0x88b70004;

	/* address select register */
	*(vu_long *)MPC5XXX_SDRAM_XLBSEL = 0x03000000;
#endif
	sdram_start(0);
	test1 = get_ram_size((ulong *)CFG_SDRAM_BASE, 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((ulong *)CFG_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}
#if defined(CONFIG_MPC5200)
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG =
		(0x13 + __builtin_ffs(dramsize >> 20) - 1);
#ifdef CONFIG_MPC5200_DDR
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize + 0x0000001e;/* 2G */
	sdram_start(0);
	test1 = get_ram_size((ulong *)(CFG_SDRAM_BASE + dramsize), 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((ulong *)(CFG_SDRAM_BASE + dramsize), 0x80000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize2 = test1;
	} else {
		dramsize2 = test2;
	}
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG =
		dramsize + (0x13 + __builtin_ffs(dramsize2 >> 20) - 1);
#else
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize; /* disabled */
#endif
#elif defined(CONFIG_MGT5100)
	*(vu_long *)MPC5XXX_SDRAM_STOP = ((dramsize - 1) >> 15);
#endif

#else	/* CFG_RAMBOOT */
#ifdef CONFIG_MGT5100
	*(vu_long *)MPC5XXX_ADDECR |= (1 << 22); /* Enable SDRAM */
	dramsize = ((*(vu_long *)MPC5XXX_SDRAM_STOP + 1) << 15);
#else
	dramsize = ((1 << (*(vu_long *)MPC5XXX_SDRAM_CS0CFG - 0x13)) << 20);
#ifdef CONFIG_MPC5200_DDR
	dramsize2 = ((1 << (*(vu_long *)MPC5XXX_SDRAM_CS1CFG - 0x13)) << 20);
#endif
#endif
#endif /* CFG_RAMBOOT */

#ifdef CONFIG_MPC5200_DDR
	dramsize += dramsize2;
#endif
	/* return total ram size */
	return dramsize;
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

void flash_afterinit(ulong size)
{
	if (size == 0x800000) { /* adjust mapping */
		*(vu_long *)MPC5XXX_BOOTCS_START = *(vu_long *)MPC5XXX_CS0_START =
			START_REG(CFG_BOOTCS_START | size);
		*(vu_long *)MPC5XXX_BOOTCS_STOP = *(vu_long *)MPC5XXX_CS0_STOP =
			STOP_REG(CFG_BOOTCS_START | size, size);
	}
}

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif
