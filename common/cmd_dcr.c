/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * AMCC 4XX DCR Functions
 */

#include <common.h>
#include <cli.h>
#include <config.h>
#include <command.h>
#include <console.h>

unsigned long get_dcr (unsigned short);
unsigned long set_dcr (unsigned short, unsigned long);

/* =======================================================================
 * Interpreter command to retrieve an AMCC PPC 4xx Device Control Register
 * =======================================================================
 */
int do_getdcr ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[] )
{
	unsigned short dcrn;	/* Device Control Register Num */
	unsigned long value;	/* DCR's value */

	unsigned long get_dcr (unsigned short);

	/* Validate arguments */
	if (argc < 2)
		return CMD_RET_USAGE;

	/* Get a DCR */
	dcrn = (unsigned short) simple_strtoul (argv[1], NULL, 16);
	value = get_dcr (dcrn);

	printf ("%04x: %08lx\n", dcrn, value);

	return 0;
}


/* ======================================================================
 * Interpreter command to set an AMCC PPC 4xx Device Control Register
 * ======================================================================
*/
int do_setdcr (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	unsigned short dcrn;	/* Device Control Register Num */
	unsigned long value;

	/* DCR's value */
	int nbytes;

	/* Validate arguments */
	if (argc < 2)
		return CMD_RET_USAGE;

	/* Set a DCR */
	dcrn = (unsigned short) simple_strtoul (argv[1], NULL, 16);
	do {
		value = get_dcr (dcrn);
		printf ("%04x: %08lx", dcrn, value);
		nbytes = cli_readline(" ? ");
		if (nbytes == 0) {
			/*
			 * <CR> pressed as only input, don't modify current
			 * location and exit command.
			 */
			nbytes = 1;
			return 0;
		} else {
			unsigned long i;
			char *endp;

			i = simple_strtoul (console_buffer, &endp, 16);
			nbytes = endp - console_buffer;
			if (nbytes)
				set_dcr (dcrn, i);
		}
	} while (nbytes);

	return 0;
}

/* =======================================================================
 * Interpreter command to retrieve an register value through AMCC PPC 4xx
 * Device Control Register inderect addressing.
 * =======================================================================
 */
int do_getidcr (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned short adr_dcrn;	/* Device Control Register Num for Address */
	unsigned short dat_dcrn;	/* Device Control Register Num for Data */
	unsigned short offset;		/* Register's offset */
	unsigned long value;		/* Register's value */
	char *ptr = NULL;
	char buf[80];

	/* Validate arguments */
	if (argc < 3)
		return CMD_RET_USAGE;

	/* Find out whether ther is '.' (dot) symbol in the first parameter. */
	strncpy (buf, argv[1], sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0; /* will guarantee zero-end string */
	ptr = strchr (buf, '.');

	if (ptr != NULL) {
		/* First parameter has format adr_dcrn.dat_dcrn */
		*ptr++ = 0; /* erase '.', create zero-end string */
		adr_dcrn = (unsigned short) simple_strtoul (buf, NULL, 16);
		dat_dcrn = (unsigned short) simple_strtoul (ptr, NULL, 16);
	} else {
		/*
		 * First parameter has format adr_dcrn; dat_dcrn will be
		 * calculated as adr_dcrn+1.
		 */
		adr_dcrn = (unsigned short) simple_strtoul (buf, NULL, 16);
		dat_dcrn = adr_dcrn+1;
	}

	/* Register's offset */
	offset = (unsigned short) simple_strtoul (argv[2], NULL, 16);

	/* Disable interrupts */
	disable_interrupts ();
	/* Set offset */
	set_dcr (adr_dcrn, offset);
	/* get data */
	value = get_dcr (dat_dcrn);
	/* Enable interrupts */
	enable_interrupts ();

	printf ("%04x.%04x-%04x Read  %08lx\n", adr_dcrn, dat_dcrn, offset, value);

	return 0;
}

/* =======================================================================
 * Interpreter command to update an register value through AMCC PPC 4xx
 * Device Control Register inderect addressing.
 * =======================================================================
 */
int do_setidcr (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	unsigned short adr_dcrn;	/* Device Control Register Num for Address */
	unsigned short dat_dcrn;	/* Device Control Register Num for Data */
	unsigned short offset;		/* Register's offset */
	unsigned long value;		/* Register's value */
	char *ptr = NULL;
	char buf[80];

	/* Validate arguments */
	if (argc < 4)
		return CMD_RET_USAGE;

	/* Find out whether ther is '.' (dot) symbol in the first parameter. */
	strncpy (buf, argv[1], sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0; /* will guarantee zero-end string */
	ptr = strchr (buf, '.');

	if (ptr != NULL) {
		/* First parameter has format adr_dcrn.dat_dcrn */
		*ptr++ = 0;	/* erase '.', create zero-end string */
		adr_dcrn = (unsigned short) simple_strtoul (buf, NULL, 16);
		dat_dcrn = (unsigned short) simple_strtoul (ptr, NULL, 16);
	} else {
		/*
		 * First parameter has format adr_dcrn; dat_dcrn will be
		 * calculated as adr_dcrn+1.
		 */
		adr_dcrn = (unsigned short) simple_strtoul (buf, NULL, 16);
		dat_dcrn = adr_dcrn+1;
	}

	/* Register's offset */
	offset = (unsigned short) simple_strtoul (argv[2], NULL, 16);
	/* New value */
	value  = (unsigned  long) simple_strtoul (argv[3], NULL, 16);

	/* Disable interrupts */
	disable_interrupts ();
	/* Set offset */
	set_dcr (adr_dcrn, offset);
	/* set data */
	set_dcr (dat_dcrn, value);
	/* Enable interrupts */
	enable_interrupts ();

	printf ("%04x.%04x-%04x Write %08lx\n", adr_dcrn, dat_dcrn, offset, value);

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	getdcr,	2,	1,	do_getdcr,
	"Get an AMCC PPC 4xx DCR's value",
	"dcrn - return a DCR's value."
);
U_BOOT_CMD(
	setdcr,	2,	1,	do_setdcr,
	"Set an AMCC PPC 4xx DCR's value",
	"dcrn - set a DCR's value."
);

U_BOOT_CMD(
	getidcr,	3,	1,	do_getidcr,
	"Get a register value via indirect DCR addressing",
	"adr_dcrn[.dat_dcrn] offset - write offset to adr_dcrn, read value from dat_dcrn."
);

U_BOOT_CMD(
	setidcr,	4,	1,	do_setidcr,
	"Set a register value via indirect DCR addressing",
	"adr_dcrn[.dat_dcrn] offset value - write offset to adr_dcrn, write value to dat_dcrn."
);
