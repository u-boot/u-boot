/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* includes */
#include <common.h>
#include <linux/ctype.h>
#include <pci.h>
#include <net.h>
#include "srom.h"

/* imports  */
extern int l2_cache_enable (int l2control);
extern int eepro100_write_eeprom (struct eth_device *dev, int location,
				  int addr_len, unsigned short data);
extern int read_eeprom (struct eth_device *dev, int location, int addr_len);

/*----------------------------------------------------------------------------*/
/*
 * read/write to nvram is only byte access
 */
void *nvram_read (void *dest, const long src, size_t count)
{
	uchar *d = (uchar *) dest;
	uchar *s = (uchar *) (CONFIG_ENV_MAP_ADRS + src);

	while (count--)
		*d++ = *s++;

	return dest;
}

void nvram_write (long dest, const void *src, size_t count)
{
	uchar *d = (uchar *) (CONFIG_ENV_MAP_ADRS + dest);
	uchar *s = (uchar *) src;

	while (count--)
		*d++ = *s++;
}

/*----------------------------------------------------------------------------*/
/*
 * handle sroms on ELPPC
 * fix ether address
 * set serial console as default
 */
int misc_init_r (void)
{
	revinfo eerev;
	u_char *ptr;
	u_int i, l, initSrom, copyNv;
	char buf[256];
	char hex[23] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0,
		0, 0, 0, 0, 10, 11, 12, 13, 14, 15
	};

	/* Clock setting for MPC107 i2c */
	mpc107_i2c_init (MPC107_EUMB_ADDR, 0x2b);

	/* Reset the EPIC */
	out32r (MPC107_EUMB_GCR, 0xa0000000);
	while (in32r (MPC107_EUMB_GCR) & 0x80000000);	/* Wait for reset to complete */
	out32r (MPC107_EUMB_GCR, 0x20000000);	/* Put into into mixed mode */
	while (in32r (MPC107_EUMB_IACKR) != 0xff);	/* Clear all pending interrupts */

	/*
	 * Check/Remake revision info
	 */
	initSrom = 0;
	copyNv = 0;

	/* read out current revision srom contens */
	mpc107_srom_load (0x0000, (u_char *) & eerev, sizeof (revinfo),
			  SECOND_DEVICE, FIRST_BLOCK);

	/* read out current nvram shadow image */
	nvram_read (buf, CONFIG_SYS_NV_SROM_COPY_ADDR, CONFIG_SYS_SROM_SIZE);

	if (strcmp (eerev.magic, "ELTEC") != 0) {
		/* srom is not initialized -> create a default revision info */
		for (i = 0, ptr = (u_char *) & eerev; i < sizeof (revinfo);
		     i++)
			*ptr++ = 0x00;
		strcpy (eerev.magic, "ELTEC");
		eerev.revrev[0] = 1;
		eerev.revrev[1] = 0;
		eerev.size = 0x00E0;
		eerev.category[0] = 0x01;

		/* node id from dead e128 as default */
		eerev.etheraddr[0] = 0x00;
		eerev.etheraddr[1] = 0x00;
		eerev.etheraddr[2] = 0x5B;
		eerev.etheraddr[3] = 0x00;
		eerev.etheraddr[4] = 0x2E;
		eerev.etheraddr[5] = 0x4D;

		/* cache config word for ELPPC */
		memset(&eerev.res[0], 0, 4);

		initSrom = 1;	/* force dialog */
		copyNv = 1;	/* copy to nvram */
	}

	if ((copyNv == 0)
	    && (el_srom_checksum ((u_char *) & eerev, CONFIG_SYS_SROM_SIZE) !=
		el_srom_checksum ((u_char *) buf, CONFIG_SYS_SROM_SIZE))) {
		printf ("Invalid revision info copy in nvram !\n");
		printf ("Press key:\n  <c> to copy current revision info to nvram.\n");
		printf ("  <r> to reenter revision info.\n");
		printf ("=> ");
		if (0 != readline (NULL)) {
			switch ((char) toupper (console_buffer[0])) {
			case 'C':
				copyNv = 1;
				break;
			case 'R':
				copyNv = 1;
				initSrom = 1;
				break;
			}
		}
	}

	if (initSrom) {
		memcpy (buf, &eerev.revision[0][0], 14);	/* save all revision info */
		printf ("Enter revision number (0-9): %c  ",
			eerev.revision[0][0]);
		if (0 != readline (NULL)) {
			eerev.revision[0][0] =
				(char) toupper (console_buffer[0]);
			memcpy (&eerev.revision[1][0], buf, 12);	/* shift rest of rev info */
		}

		printf ("Enter revision character (A-Z): %c  ",
			eerev.revision[0][1]);
		if (1 == readline (NULL)) {
			eerev.revision[0][1] =
				(char) toupper (console_buffer[0]);
		}

		printf ("Enter board name (V-XXXX-XXXX): %s  ",
			(char *) &eerev.board);
		if (11 == readline (NULL)) {
			for (i = 0; i < 11; i++)
				eerev.board[i] =
					(char) toupper (console_buffer[i]);
			eerev.board[11] = '\0';
		}

		printf ("Enter serial number: %s ", (char *) &eerev.serial);
		if (6 == readline (NULL)) {
			for (i = 0; i < 6; i++)
				eerev.serial[i] = console_buffer[i];
			eerev.serial[6] = '\0';
		}

		printf ("Enter ether node ID with leading zero (HEX): %02x%02x%02x%02x%02x%02x  ", eerev.etheraddr[0], eerev.etheraddr[1], eerev.etheraddr[2], eerev.etheraddr[3], eerev.etheraddr[4], eerev.etheraddr[5]);
		if (12 == readline (NULL)) {
			for (i = 0; i < 12; i += 2)
				eerev.etheraddr[i >> 1] =
					(char) (16 *
						hex[toupper
						    (console_buffer[i]) -
						    '0'] +
						hex[toupper
						    (console_buffer[i + 1]) -
						    '0']);
		}

		l = strlen ((char *) &eerev.text);
		printf ("Add to text section (max 64 chr): %s ",
			(char *) &eerev.text);
		if (0 != readline (NULL)) {
			for (i = l; i < 63; i++)
				eerev.text[i] = console_buffer[i - l];
			eerev.text[63] = '\0';
		}

		/* prepare network eeprom */
		memset (buf, 0, 128);

		buf[0] = eerev.etheraddr[1];
		buf[1] = eerev.etheraddr[0];
		buf[2] = eerev.etheraddr[3];
		buf[3] = eerev.etheraddr[2];
		buf[4] = eerev.etheraddr[5];
		buf[5] = eerev.etheraddr[4];

		buf[20] = 0x48;
		buf[21] = 0xB2;

		buf[22] = 0x00;
		buf[23] = 0x04;

		buf[24] = 0x14;
		buf[25] = 0x33;

		printf ("\nSRom:  Writing i82559 info ........ ");
		if (eepro100_srom_store ((unsigned short *) buf) == -1)
			printf ("FAILED\n");
		else
			printf ("OK\n");

		/* update CRC */
		eerev.crc =
			el_srom_checksum ((u_char *) eerev.board, eerev.size);

		/* write new values */
		printf ("\nSRom:  Writing revision info ...... ");
		if (mpc107_srom_store
		    ((BLOCK_SIZE - sizeof (revinfo)), (u_char *) & eerev,
		     sizeof (revinfo), SECOND_DEVICE, FIRST_BLOCK) == -1)
			printf ("FAILED\n\n");
		else
			printf ("OK\n\n");

		/* write new values as shadow image to nvram */
		nvram_write (CONFIG_SYS_NV_SROM_COPY_ADDR, (void *) &eerev,
			     CONFIG_SYS_SROM_SIZE);

	}

	/*if (initSrom) */
	/* copy current values as shadow image to nvram */
	if (initSrom == 0 && copyNv == 1)
		nvram_write (CONFIG_SYS_NV_SROM_COPY_ADDR, (void *) &eerev,
			     CONFIG_SYS_SROM_SIZE);

	/* update environment */
	sprintf (buf, "%02x:%02x:%02x:%02x:%02x:%02x",
		 eerev.etheraddr[0], eerev.etheraddr[1],
		 eerev.etheraddr[2], eerev.etheraddr[3],
		 eerev.etheraddr[4], eerev.etheraddr[5]);
	setenv ("ethaddr", buf);

	/* print actual board identification */
	printf ("Ident: %s  Ser %s  Rev %c%c\n",
		eerev.board, (char *) &eerev.serial,
		eerev.revision[0][0], eerev.revision[0][1]);

	return (0);
}

/*----------------------------------------------------------------------------*/
