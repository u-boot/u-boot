/* pd67290.c - system configuration module for SPD67290
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
 *
 * (C) 2004 DENX Software Engineering, Heiko Schocher <hs@denx.de>
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>

/* imports */
#include <mpc824x.h>

static struct pci_device_id supported[] = {
	{PCI_VENDOR_ID_CIRRUS, PCI_DEVICE_ID_CIRRUS_6729},
	{}
};

/***************************************************************************
*
* SPD67290Init -
*
* RETURNS: -1 on error, 0 if OK
*/

int SPD67290Init (void)
{
	pci_dev_t devno;
	int idx = 0;		/* general index */
	ulong membaseCsr;	/* base address of device memory space */

	/* find PD67290 device */
	if ((devno = pci_find_devices (supported, idx++)) < 0) {
		printf ("No PD67290 device found !!\n");
		return -1;
	}
	/* - 0xfe000000 see MPC 8245 Users Manual Adress Map B */
	membaseCsr = PCMCIA_IO_BASE - 0xfe000000;

	/* set base address */
	pci_write_config_dword (devno, PCI_BASE_ADDRESS_0, membaseCsr);

	/* enable mapped memory and IO addresses */
	pci_write_config_dword (devno,
				PCI_COMMAND,
				PCI_COMMAND_MEMORY |
				PCI_COMMAND_IO | PCI_COMMAND_WAIT);
	return 0;
}
