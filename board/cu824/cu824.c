/*
 * (C) Copyright 2001
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>

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
#include <pci.h>

#define BOARD_REV_REG 0xFE80002B

int checkboard (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	char  revision = *(volatile char *)(BOARD_REV_REG);
	char  buf[32];

	puts ("Board: CU824 ");
	printf("Revision %d ", revision);
	printf("Local Bus at %s MHz\n", strmhz(buf, gd->bus_clk));

	return 0;
}

long int initdram(int board_type)
{
	int              i, cnt;
	volatile uchar * base      = CFG_SDRAM_BASE;
	volatile ulong * addr;
	ulong            save[32];
	ulong            val, ret  = 0;

	for (i=0, cnt=(CFG_MAX_RAM_SIZE / sizeof(long)) >> 1; cnt > 0; cnt >>= 1) {
		addr = (volatile ulong *)base + cnt;
		save[i++] = *addr;
		*addr = ~cnt;
	}

	addr = (volatile ulong *)base;
	save[i] = *addr;
	*addr = 0;

	if (*addr != 0) {
		*addr = save[i];
		goto Done;
	}

	for (cnt = 1; cnt <= CFG_MAX_RAM_SIZE / sizeof(long); cnt <<= 1) {
		addr = (volatile ulong *)base + cnt;
		val = *addr;
		*addr = save[--i];
		if (val != ~cnt) {
			ulong new_bank0_end = cnt * sizeof(long) - 1;
			ulong mear1  = mpc824x_mpc107_getreg(MEAR1);
			ulong emear1 = mpc824x_mpc107_getreg(EMEAR1);
			mear1 =  (mear1  & 0xFFFFFF00) |
			  ((new_bank0_end & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT);
			emear1 = (emear1 & 0xFFFFFF00) |
			  ((new_bank0_end & MICR_ADDR_MASK) >> MICR_EADDR_SHIFT);
			mpc824x_mpc107_setreg(MEAR1,  mear1);
			mpc824x_mpc107_setreg(EMEAR1, emear1);

			ret = cnt * sizeof(long);
			goto Done;
		}
	}

	ret = CFG_MAX_RAM_SIZE;
Done:
	return ret;
}

/*
 * Initialize PCI Devices, report devices found.
 */
#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_sandpoint_config_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_BUS(CFG_ETH_DEV_FN), PCI_DEV(CFG_ETH_DEV_FN), PCI_FUNC(CFG_ETH_DEV_FN),
	  pci_cfgfunc_config_device, { CFG_ETH_IOBASE,
				       0,
				       PCI_COMMAND_IO | PCI_COMMAND_MASTER }},
	{ }
};
#endif

struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_sandpoint_config_table,
#endif
};

void pci_init(void)
{
	pci_mpc824x_init(&hose);
}
