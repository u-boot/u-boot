/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Torsten Demke, FORCE Computers GmbH. torsten.demke@fci.com
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
#include <mpc824x.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>
#include <ide.h>
#include <netdev.h>
#include <timestamp.h>
#include "piix_pci.h"
#include "eXalion.h"

int checkboard (void)
{
	ulong busfreq = get_bus_freq (0);
	char buf[32];

	printf ("Board: eXalion MPC824x - CHRP (MAP B)\n");
	printf ("Built: %s at %s\n", U_BOOT_DATE, U_BOOT_TIME);
	printf ("Local Bus:  %s MHz\n", strmhz (buf, busfreq));

	return 0;
}

int checkflash (void)
{
	printf ("checkflash\n");
	flash_init ();
	return (0);
}

phys_size_t initdram (int board_type)
{
	int i, cnt;
	volatile uchar *base = CONFIG_SYS_SDRAM_BASE;
	volatile ulong *addr;
	ulong save[32];
	ulong val, ret = 0;

	for (i = 0, cnt = (CONFIG_SYS_MAX_RAM_SIZE / sizeof (long)) >> 1; cnt > 0;
	     cnt >>= 1) {
		addr = (volatile ulong *) base + cnt;
		save[i++] = *addr;
		*addr = ~cnt;
	}

	addr = (volatile ulong *) base;
	save[i] = *addr;
	*addr = 0;

	if (*addr != 0) {
		*addr = save[i];
		goto Done;
	}

	for (cnt = 1; cnt <= CONFIG_SYS_MAX_RAM_SIZE / sizeof (long); cnt <<= 1) {
		addr = (volatile ulong *) base + cnt;
		val = *addr;
		*addr = save[--i];
		if (val != ~cnt) {
			ulong new_bank0_end = cnt * sizeof (long) - 1;
			ulong mear1 = mpc824x_mpc107_getreg (MEAR1);
			ulong emear1 = mpc824x_mpc107_getreg (EMEAR1);

			mear1 = (mear1 & 0xFFFFFF00) |
				((new_bank0_end & MICR_ADDR_MASK) >>
				 MICR_ADDR_SHIFT);
			emear1 = (emear1 & 0xFFFFFF00) |
				((new_bank0_end & MICR_ADDR_MASK) >>
				 MICR_EADDR_SHIFT);
			mpc824x_mpc107_setreg (MEAR1, mear1);
			mpc824x_mpc107_setreg (EMEAR1, emear1);

			ret = cnt * sizeof (long);
			goto Done;
		}
	}

	ret = CONFIG_SYS_MAX_RAM_SIZE;
      Done:
	return ret;
}

int misc_init_r (void)
{
	pci_dev_t bdf;
	u32 val32;
	u8 val8;

	puts ("ISA:   ");
	bdf = pci_find_device (PIIX4_VENDOR_ID, PIIX4_ISA_DEV_ID, 0);
	if (bdf == -1) {
		puts ("Unable to find PIIX4 ISA bridge !\n");
		hang ();
	}

	/* set device for normal ISA instead EIO */
	pci_read_config_dword (bdf, PCI_CFG_PIIX4_GENCFG, &val32);
	val32 |= 0x00000001;
	pci_write_config_dword (bdf, PCI_CFG_PIIX4_GENCFG, val32);
	printf ("PIIX4 ISA bridge (%d,%d,%d)\n", PCI_BUS (bdf),
		PCI_DEV (bdf), PCI_FUNC (bdf));

	puts ("ISA:   ");
	bdf = pci_find_device (PIIX4_VENDOR_ID, PIIX4_IDE_DEV_ID, 0);
	if (bdf == -1) {
		puts ("Unable to find PIIX4 IDE controller !\n");
		hang ();
	}

	/* Init BMIBA register  */
	/* pci_read_config_dword(bdf, PCI_CFG_PIIX4_BMIBA, &val32); */
	/* val32 |= 0x1000; */
	/* pci_write_config_dword(bdf, PCI_CFG_PIIX4_BMIBA, val32); */

	/* Enable BUS master and IO access  */
	val32 = PCI_COMMAND_MASTER | PCI_COMMAND_IO;
	pci_write_config_dword (bdf, PCI_COMMAND, val32);

	/* Set latency  */
	pci_read_config_byte (bdf, PCI_LATENCY_TIMER, &val8);
	val8 = 0x40;
	pci_write_config_byte (bdf, PCI_LATENCY_TIMER, val8);

	/* Enable Primary ATA/IDE  */
	pci_read_config_dword (bdf, PCI_CFG_PIIX4_IDETIM, &val32);
	/* val32 = 0xa307a307; */
	val32 = 0x00008000;
	pci_write_config_dword (bdf, PCI_CFG_PIIX4_IDETIM, val32);


	printf ("PIIX4 IDE controller (%d,%d,%d)\n", PCI_BUS (bdf),
		PCI_DEV (bdf), PCI_FUNC (bdf));

	/* Try to get FAT working... */
	/* fat_register_read(ide_read); */


	return (0);
}

/*
 * Show/Init PCI devices on the specified bus number.
 */

void pci_eXalion_fixup_irq (struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char line;

	switch (PCI_DEV (dev)) {
	case 16:
		line = PCI_INT_A;
		break;
	case 17:
		line = PCI_INT_B;
		break;
	case 18:
		line = PCI_INT_C;
		break;
	case 19:
		line = PCI_INT_D;
		break;
#if defined (CONFIG_MPC8245)
	case 20:
		line = PCI_INT_A;
		break;
	case 21:
		line = PCI_INT_B;
		break;
	case 22:
		line = PCI_INT_NA;
		break;
#endif
	default:
		line = PCI_INT_A;
		break;
	}
	pci_hose_write_config_byte (hose, dev, PCI_INTERRUPT_LINE, line);
}


/*
 * Initialize PCI Devices, report devices found.
 */
#ifndef CONFIG_PCI_PNP
#if defined (CONFIG_MPC8240)
static struct pci_config_table pci_eXalion_config_table[] = {
	{
	 /* Intel 82559ER ethernet controller */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 18, 0x00,
	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				     PCI_ENET0_MEMADDR,
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
	{
	 /* Intel 82371AB PIIX4 PCI to ISA bridge */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 20, 0x00,
	 pci_cfgfunc_config_device, {0,
				     0,
				     PCI_COMMAND_IO | PCI_COMMAND_MASTER}},
	{
	 /* Intel 82371AB PIIX4 IDE controller */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 20, 0x01,
	 pci_cfgfunc_config_device, {0,
				     0,
				     PCI_COMMAND_IO | PCI_COMMAND_MASTER}},
	{}
};
#elif defined (CONFIG_MPC8245)
static struct pci_config_table pci_eXalion_config_table[] = {
	{
	 /* Intel 82559ER ethernet controller */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 17, 0x00,
	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				     PCI_ENET0_MEMADDR,
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
	{
	 /* Intel 82559ER ethernet controller */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 18, 0x00,
	 pci_cfgfunc_config_device, {PCI_ENET1_IOADDR,
				     PCI_ENET1_MEMADDR,
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
	{
	 /* Broadcom BCM5690 Gigabit switch */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 20, 0x00,
	 pci_cfgfunc_config_device, {PCI_ENET2_IOADDR,
				     PCI_ENET2_MEMADDR,
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
	{
	 /* Broadcom BCM5690 Gigabit switch */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 21, 0x00,
	 pci_cfgfunc_config_device, {PCI_ENET3_IOADDR,
				     PCI_ENET3_MEMADDR,
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
	{
	 /* Intel 82371AB PIIX4 PCI to ISA bridge */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 22, 0x00,
	 pci_cfgfunc_config_device, {0,
				     0,
				     PCI_COMMAND_IO | PCI_COMMAND_MASTER}},
	{
	 /* Intel 82371AB PIIX4 IDE controller */
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 22, 0x01,
	 pci_cfgfunc_config_device, {0,
				     0,
				     PCI_COMMAND_IO | PCI_COMMAND_MASTER}},
	{}
};
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif

#endif /* #ifndef CONFIG_PCI_PNP */

struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table:pci_eXalion_config_table,
	fixup_irq:pci_eXalion_fixup_irq,
#endif
};

void pci_init_board (void)
{
	pci_mpc824x_init (&hose);
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
