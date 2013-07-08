/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Local network srom writing for first time run
 */

/* includes */
#include <common.h>
#include <pci.h>
#include <net.h>
#include "srom.h"

extern int eepro100_write_eeprom (struct eth_device *dev,
				  int location, int addr_len,
				  unsigned short data);

/*----------------------------------------------------------------------------*/

unsigned short eepro100_srom_checksum (unsigned short *sromdata)
{
	unsigned short sum = 0;
	unsigned int i;

	for (i = 0; i < (EE_SIZE - 1); i++) {
		sum += sromdata[i];
	}
	return (EE_CHECKSUM - sum);
}

/*----------------------------------------------------------------------------*/

int eepro100_srom_store (unsigned short *source)
{
	int count;
	struct eth_device onboard_dev;

	/* get onboard network iobase */
	pci_read_config_dword (PCI_BDF (0, 0x10, 0), PCI_BASE_ADDRESS_0,
			       (unsigned int *) &onboard_dev.iobase);
	onboard_dev.iobase &= ~0xf;

	source[63] = eepro100_srom_checksum (source);

	for (count = 0; count < EE_SIZE; count++) {
		if (eepro100_write_eeprom ((struct eth_device *) &onboard_dev,
					   count, EE_ADDR_BITS,
					   SROM_SHORT (source)) == -1) {
			return -1;
		}
		source++;
	}
	return 0;
}

/*----------------------------------------------------------------------------*/

#ifdef EEPRO100_SROM_CHECK

extern int read_eeprom (struct eth_device *dev, int location, int addr_len);

void eepro100_srom_load (unsigned short *destination)
{
	int count;
	struct eth_device onboard_dev;

#ifdef DEBUG
	int lr = 0;

	printf ("eepro100_srom_download:\n");
#endif

	/* get onboard network iobase */
	pci_read_config_dword (PCI_BDF (0, 0x10, 0), PCI_BASE_ADDRESS_0,
			       &onboard_dev.iobase);
	onboard_dev.iobase &= ~0xf;

	memset (destination, 0x65, 128);

	for (count = 0; count < 0x40; count++) {
		*destination++ = read_eeprom ((struct eth_device *) &onboard_dev,
					      count, EE_ADDR_BITS);
#ifdef DEBUG
		printf ("%04x ", *(destination - 1));
		if (lr++ == 7) {
			printf ("\n");
			lr = 0;
		}
#endif
	}
}
#endif /* EEPRO100_SROM_CHECK */

/*----------------------------------------------------------------------------*/
