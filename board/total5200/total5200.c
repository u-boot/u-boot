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
#include <pci.h>

#include "sdram.h"

#if CONFIG_TOTAL5200_REV==2
#include "mt48lc32m16a2-75.h"
#else
#include "mt48lc16m16a2-75.h"
#endif

long int initdram (int board_type)
{
	sdram_conf_t sdram_conf;

	sdram_conf.ddr = SDRAM_DDR;
	sdram_conf.mode = SDRAM_MODE;
	sdram_conf.emode = 0;
	sdram_conf.control = SDRAM_CONTROL;
	sdram_conf.config1 = SDRAM_CONFIG1;
	sdram_conf.config2 = SDRAM_CONFIG2;
#if defined(CONFIG_MPC5200)
	sdram_conf.tapdelay = 0;
#endif
#if defined(CONFIG_MGT5100)
	sdram_conf.addrsel = SDRAM_ADDRSEL;
#endif
	return mpc5xxx_sdram_init (&sdram_conf);
}

int checkboard (void)
{
#if defined(CONFIG_MPC5200)
#if CONFIG_TOTAL5200_REV==2
	puts ("Board: Total5200 Rev.2 ");
#else
	puts ("Board: Total5200 ");
#endif
#elif defined(CONFIG_MGT5100)
	puts ("Board: Total5100 ");
#endif

/*
 * Retrieve FPGA Revision.
 */
printf ("(FPGA %08X)\n", *(vu_long *) (CFG_FPGA_BASE + 0x400));

/*
 * Take all peripherals in power-up mode.
 */
#if CONFIG_TOTAL5200_REV==2
	*(vu_char *) (CFG_CPLD_BASE + 0x46) = 0x70;
#else
	*(vu_long *) (CFG_CPLD_BASE + 0x400) = 0x70;
#endif

	return 0;
}

#if defined(CONFIG_MGT5100)
int board_early_init_r(void)
{
	/*
	 * Now, when we are in RAM, enable CS0
	 * because CS_BOOT cannot be written.
	 */
	*(vu_long *)MPC5XXX_ADDECR &= ~(1 << 25); /* disable CS_BOOT */
	*(vu_long *)MPC5XXX_ADDECR |= (1 << 16); /* enable CS0 */

	return 0;
}
#endif

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#if defined (CFG_CMD_IDE) && defined (CONFIG_IDE_RESET)

/* IRDA_1 aka PSC6_3 (pin C13) */
#define GPIO_IRDA_1	0x20000000UL

void init_ide_reset (void)
{
	debug ("init_ide_reset\n");

    	/* Configure IRDA_1 (PSC6_3) as GPIO output for ATA reset */
	*(vu_long *) MPC5XXX_GPIO_ENABLE |= GPIO_IRDA_1;
	*(vu_long *) MPC5XXX_GPIO_DIR    |= GPIO_IRDA_1;
}

void ide_set_reset (int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

	if (idereset) {
		*(vu_long *) MPC5XXX_GPIO_DATA_O &= ~GPIO_IRDA_1;
	} else {
		*(vu_long *) MPC5XXX_GPIO_DATA_O |=  GPIO_IRDA_1;
	}
}
#endif /* defined (CFG_CMD_IDE) && defined (CONFIG_IDE_RESET) */
