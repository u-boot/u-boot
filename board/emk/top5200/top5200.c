/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003
 * Reinhard Meyer, EMK Elektronik GmbH, r.meyer@emk-elektronik.de
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

/*****************************************************************************
 * initialize SDRAM/DDRAM controller.
 * TBD: get data from I2C EEPROM
 *****************************************************************************/
long int initdram (int board_type)
{
	ulong dramsize = 0;
#ifndef CFG_RAMBOOT
#if 0
	ulong	t;
	ulong	tap_del;
#endif

	#define	MODE_EN		0x80000000
	#define	SOFT_PRE	2
	#define	SOFT_REF	4

	/* configure SDRAM start/end */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = (CFG_SDRAM_BASE & 0xFFF00000) | CFG_DRAM_RAM_SIZE;
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x80000000;	/* disabled */

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = CFG_DRAM_CONFIG1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = CFG_DRAM_CONFIG2;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = CFG_DRAM_CONTROL | MODE_EN;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = CFG_DRAM_CONTROL | MODE_EN | SOFT_PRE;
#ifdef CFG_DRAM_DDR
	/* set extended mode register */
	*(vu_short *)MPC5XXX_SDRAM_MODE = CFG_DRAM_EMODE;
#endif
	/* set mode register */
	*(vu_short *)MPC5XXX_SDRAM_MODE = CFG_DRAM_MODE | 0x0400;
	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = CFG_DRAM_CONTROL | MODE_EN | SOFT_PRE;
	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = CFG_DRAM_CONTROL | MODE_EN | SOFT_REF;
	/* set mode register */
	*(vu_short *)MPC5XXX_SDRAM_MODE = CFG_DRAM_MODE;
	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = CFG_DRAM_CONTROL;
	/* write default TAP delay */
	*(vu_long *)MPC5XXX_CDM_PORCFG = CFG_DRAM_TAP_DEL << 24;

#if 0
	for (tap_del = 0; tap_del < 32; tap_del++)
	{
		*(vu_long *)MPC5XXX_CDM_PORCFG = tap_del << 24;

		printf ("\nTAP Delay:%x Filling DRAM...", *(vu_long *)MPC5XXX_CDM_PORCFG);
		for (t = 0; t < 0x04000000; t+=4)
			*(vu_long *) t = t;
		printf ("Checking DRAM...\n");
		for (t = 0; t < 0x04000000; t+=4)
		{
			ulong	rval = *(vu_long *) t;
			if (rval != t)
			{
				printf ("mismatch at %x: ", t);
				printf (" 1.read %x", rval);
				printf (" 2.read %x", *(vu_long *) t);
				printf (" 3.read %x", *(vu_long *) t);
				break;
			}
		}
	}
#endif
#endif /* CFG_RAMBOOT */

	dramsize = ((1 << (*(vu_long *)MPC5XXX_SDRAM_CS0CFG - 0x13)) << 20);

	/* return total ram size */
	return dramsize;
}

/*****************************************************************************
 * print board identification
 *****************************************************************************/
int checkboard (void)
{
#if defined (CONFIG_EVAL5200)
	puts ("Board: EMK TOP5200 on EVAL5200\n");
#else
#if defined (CONFIG_LITE5200)
	puts ("Board: LITE5200\n");
#else
#if defined (CONFIG_MINI5200)
	puts ("Board: EMK TOP5200 on MINI5200\n");
#else
	puts ("Board: EMK TOP5200\n");
#endif
#endif
#endif
	return 0;
}

/*****************************************************************************
 * prepare for FLASH detection
 *****************************************************************************/
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

/*****************************************************************************
 * finalize FLASH setup
 *****************************************************************************/
void flash_afterinit(uint bank, ulong start, ulong size)
{
	if (bank == 0) { /* adjust mapping */
		*(vu_long *)MPC5XXX_BOOTCS_START =
		*(vu_long *)MPC5XXX_CS0_START =	START_REG(start);
		*(vu_long *)MPC5XXX_BOOTCS_STOP =
		*(vu_long *)MPC5XXX_CS0_STOP = STOP_REG(start, size);
	}
}

/*****************************************************************************
 * otherinits after RAM is there and we are relocated to RAM
 * note: though this is an int function, nobody cares for the result!
 *****************************************************************************/
int misc_init_r (void)
{
#if !defined (CONFIG_LITE5200)
	/* read 'factory' part of EEPROM */
	extern void read_factory_r (void);
	read_factory_r ();
#endif
	return (0);
}

/*****************************************************************************
 * initialize the PCI system
 *****************************************************************************/
#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

/*****************************************************************************
 * provide the IDE Reset Function
 *****************************************************************************/
#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET)

void init_ide_reset (void)
{
	debug ("init_ide_reset\n");

	/* Configure PSC1_4 as GPIO output for ATA reset */
	*(vu_long *) MPC5XXX_WU_GPIO_ENABLE |= GPIO_PSC1_4;
	*(vu_long *) MPC5XXX_WU_GPIO_DIR    |= GPIO_PSC1_4;
}

void ide_set_reset (int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

	if (idereset) {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA_O &= ~GPIO_PSC1_4;
	} else {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA_O |=  GPIO_PSC1_4;
	}
}
#endif
