/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
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
#include <netdev.h>

#define SDRAM_MODE      0x00CD0000
#define SDRAM_CONTROL   0x504F0000
#define SDRAM_CONFIG1   0xD2322800
#define SDRAM_CONFIG2   0x8AD70000

static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000000 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000004 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE;
	__asm__ volatile ("sync");

	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | hi_addr_bit;
	__asm__ volatile ("sync");
}

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *            use of CONFIG_SYS_SDRAM_BASE. The code does not work if CONFIG_SYS_SDRAM_BASE
 *            is something else than 0x00000000.
 */
phys_size_t initdram (int board_type)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	ulong test1, test2;

	/* setup SDRAM chip selects */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001e;/* 2G at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x80000000;/* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = SDRAM_CONFIG1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = SDRAM_CONFIG2;
	__asm__ volatile ("sync");

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20)) {
		dramsize = 0;
	}

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0)
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 + __builtin_ffs(dramsize >> 20) - 1;
	else
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */

	/* let SDRAM CS1 start right after CS0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize + 0x0000001e;/* 2G */

	/* find RAM size using SDRAM CS1 only */
	if (!dramsize)
		sdram_start(0);

	test2 = test1 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x80000000);

	if (!dramsize) {
		sdram_start(1);
		test2 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x80000000);
	}

	if (test1 > test2) {
		sdram_start(0);
		dramsize2 = test1;
	} else {
		dramsize2 = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20))
		dramsize2 = 0;

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize
			| (0x13 + __builtin_ffs(dramsize2 >> 20) - 1);
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize; /* disabled */
	}

	return dramsize + dramsize2;
}

int checkboard (void)
{
	puts ("Board: O2DNT\n");
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
	*(vu_long *)MPC5XXX_BOOTCS_CFG &= ~0x1; /* clear RO */
}

void flash_afterinit(ulong size)
{
	if (size == 0x800000) { /* adjust mapping */
		*(vu_long *)MPC5XXX_BOOTCS_START = *(vu_long *)MPC5XXX_CS0_START =
			START_REG(CONFIG_SYS_BOOTCS_START | size);

		*(vu_long *)MPC5XXX_BOOTCS_STOP = *(vu_long *)MPC5XXX_CS0_STOP =
			STOP_REG(CONFIG_SYS_BOOTCS_START | size, size);
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

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in FEC comes first */
	return pci_eth_init(bis);
}
