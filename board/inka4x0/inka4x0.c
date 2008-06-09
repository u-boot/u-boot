/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2004
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

#if defined(CONFIG_DDR_MT46V16M16)
#include "mt46v16m16-75.h"
#elif defined(CONFIG_SDR_MT48LC16M16A2)
#include "mt48lc16m16a2-75.h"
#elif defined(CONFIG_DDR_MT46V32M16)
#include "mt46v32m16.h"
#elif defined(CONFIG_DDR_HYB25D512160BF)
#include "hyb25d512160bf.h"
#elif defined(CONFIG_DDR_K4H511638C)
#include "k4h511638c.h"
#else
#error "INKA4x0 SDRAM: invalid chip type specified!"
#endif

#ifndef CFG_RAMBOOT
static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000000 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set mode register: extended mode */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_EMODE;
	__asm__ volatile ("sync");

	/* set mode register: reset DLL */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE | 0x04000000;
	__asm__ volatile ("sync");
#endif

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
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *	      use of CFG_SDRAM_BASE. The code does not work if CFG_SDRAM_BASE
 *	      is something else than 0x00000000.
 */

phys_size_t initdram (int board_type)
{
	ulong dramsize = 0;
#ifndef CFG_RAMBOOT
	long test1, test2;

	/* setup SDRAM chip selects */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001c; /* 512MB at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x40000000; /* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = SDRAM_CONFIG1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = SDRAM_CONFIG2;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set tap delay */
	*(vu_long *)MPC5XXX_CDM_PORCFG = SDRAM_TAPDELAY;
	__asm__ volatile ("sync");
#endif

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CFG_SDRAM_BASE, 0x20000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CFG_SDRAM_BASE, 0x20000000);
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
	if (dramsize > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 +
			__builtin_ffs(dramsize >> 20) - 1;
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */
	}

	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize; /* disabled */
#else /* CFG_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = *(vu_long *)MPC5XXX_SDRAM_CS0CFG & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}
#endif /* CFG_RAMBOOT */

	return dramsize;
}

int checkboard (void)
{
	puts ("Board: INKA 4X0\n");
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

int misc_init_f (void)
{
	char tmp[10];
	int i, br;

	i = getenv_r("brightness", tmp, sizeof(tmp));
	br = (i > 0)
		? (int) simple_strtoul (tmp, NULL, 10)
		: CFG_BRIGHTNESS;
	if (br > 255)
		br = 255;

	/* Initialize GPIO output pins.
	 */
	/* Configure GPT as GPIO output (and set them as they control low-active LEDs */
	*(vu_long *)MPC5XXX_GPT0_ENABLE =
	*(vu_long *)MPC5XXX_GPT1_ENABLE =
	*(vu_long *)MPC5XXX_GPT2_ENABLE =
	*(vu_long *)MPC5XXX_GPT3_ENABLE =
	*(vu_long *)MPC5XXX_GPT4_ENABLE =
	*(vu_long *)MPC5XXX_GPT5_ENABLE = 0x34;

	/* Configure GPT7 as PWM timer, 1kHz, no ints. */
	*(vu_long *)MPC5XXX_GPT7_ENABLE = 0;/* Disable */
	*(vu_long *)MPC5XXX_GPT7_COUNTER = 0x020000fe;
	*(vu_long *)MPC5XXX_GPT7_PWMCFG = (br << 16);
	*(vu_long *)MPC5XXX_GPT7_ENABLE = 0x3;/* Enable PWM mode and start */

	/* Configure PSC3_6,7 as GPIO output */
	*(vu_long *)MPC5XXX_GPIO_ENABLE |= 0x00003000;
	*(vu_long *)MPC5XXX_GPIO_DIR |= 0x00003000;

	/* Configure PSC3_8 as GPIO output, no interrupt */
	*(vu_long *)MPC5XXX_GPIO_SI_ENABLE |= 0x04000000;
	*(vu_long *)MPC5XXX_GPIO_SI_DIR |= 0x04000000;
	*(vu_long *)MPC5XXX_GPIO_SI_IEN &= ~0x04000000;

	/* Configure PSC3_9 and GPIO_WKUP6,7 as GPIO output */
	*(vu_long *)MPC5XXX_WU_GPIO_ENABLE |= 0xc4000000;
	*(vu_long *)MPC5XXX_WU_GPIO_DIR |= 0xc4000000;

	/* Set LR mirror bit because it is low-active */
	*(vu_long *) MPC5XXX_WU_GPIO_DATA_O    |= GPIO_WKUP_7;
	/*
	 * Reset Coral-P graphics controller
	 */
	*(vu_long *) MPC5XXX_WU_GPIO_ENABLE |= GPIO_PSC3_9;
	*(vu_long *) MPC5XXX_WU_GPIO_DIR    |= GPIO_PSC3_9;
	*(vu_long *) MPC5XXX_WU_GPIO_DATA_O   |= GPIO_PSC3_9;
	return 0;
}

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET)

void init_ide_reset (void)
{
	debug ("init_ide_reset\n");

	/* Configure PSC1_4 as GPIO output for ATA reset */
	*(vu_long *) MPC5XXX_WU_GPIO_ENABLE |= GPIO_PSC1_4;
	*(vu_long *) MPC5XXX_WU_GPIO_DIR    |= GPIO_PSC1_4;
	/* Deassert reset */
	*(vu_long *) MPC5XXX_WU_GPIO_DATA_O   |= GPIO_PSC1_4;
}

void ide_set_reset (int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

	if (idereset) {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA_O &= ~GPIO_PSC1_4;
		/* Make a delay. MPC5200 spec says 25 usec min */
		udelay(500000);
	} else {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA_O |=  GPIO_PSC1_4;
	}
}
#endif
