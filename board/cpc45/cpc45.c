/*
 * (C) Copyright 2001
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
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
#include <pci.h>

int sysControlDisplay(int digit, uchar ascii_code);			
extern void Plx9030Init(void);

	/* We have to clear the initial data area here. Couldn't have done it
	 * earlier because DRAM had not been initialized.
	 */
int board_pre_init(void)
{

	/* enable DUAL UART Mode on CPC45 */
	*(uchar*)DUART_DCR |= 0x1;	/* set DCM bit */

	return 0;
}

int checkboard(void)
{
/*
	char  revision = BOARD_REV;
*/
	ulong busfreq  = get_bus_freq(0);
	char  buf[32];

	printf("CPC45 ");
/*
	printf("Revision %d ", revision);
*/
	printf("Local Bus at %s MHz\n", strmhz(buf, busfreq));

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
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x0f, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_ENET0_IOADDR,
				       PCI_ENET0_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ }
};
#endif


struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_sandpoint_config_table,
#endif
};

void pci_init_board(void)
{
	pci_mpc824x_init(&hose);

	/* init PCI_to_LOCAL Bus BRIDGE */
	Plx9030Init();

	sysControlDisplay(0,' ');
	sysControlDisplay(1,'C');
	sysControlDisplay(2,'P');
	sysControlDisplay(3,'C');
	sysControlDisplay(4,' ');
	sysControlDisplay(5,'4');
	sysControlDisplay(6,'5');
	sysControlDisplay(7,' ');

}

/**************************************************************************
*
* sysControlDisplay - controls one of the Alphanum. Display digits.
*
* This routine will write an ASCII character to the display digit requested.
*
* SEE ALSO:
*
* RETURNS: NA
*/

int sysControlDisplay
    (
    int digit, 			/* number of digit 0..7 */
    uchar ascii_code		/* ASCII code */
    )
{
	if ((digit < 0) || (digit > 7))
		return (-1);

	*((volatile uchar*)(DISP_CHR_RAM + digit)) = ascii_code;

	return (0);
}

