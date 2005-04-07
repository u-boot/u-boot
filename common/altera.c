/*
 * (C) Copyright 2003
 * Steven Scholz, imc Measurement & Control, steven.scholz@imc-berlin.de
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
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
 *
 */

/*
 *  Altera FPGA support
 */
#include <common.h>
#include <ACEX1K.h>

/* Define FPGA_DEBUG to get debug printf's */
/* #define FPGA_DEBUG */

#ifdef	FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#if (CONFIG_FPGA & CFG_FPGA_ALTERA)

/* Local Static Functions */
static int altera_validate (Altera_desc * desc, char *fn);

/* ------------------------------------------------------------------------- */
int altera_load( Altera_desc *desc, void *buf, size_t bsize )
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (!altera_validate (desc, __FUNCTION__)) {
		printf ("%s: Invalid device descriptor\n", __FUNCTION__);
	} else {
		switch (desc->family) {
		case Altera_ACEX1K:
#if (CONFIG_FPGA & CFG_ACEX1K)
			PRINTF ("%s: Launching the ACEX1K Loader...\n",
					__FUNCTION__);
			ret_val = ACEX1K_load (desc, buf, bsize);
#else
			printf ("%s: No support for ACEX1K devices.\n",
					__FUNCTION__);
#endif
			break;

		default:
			printf ("%s: Unsupported family type, %d\n",
					__FUNCTION__, desc->family);
		}
	}

	return ret_val;
}

int altera_dump( Altera_desc *desc, void *buf, size_t bsize )
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (!altera_validate (desc, __FUNCTION__)) {
		printf ("%s: Invalid device descriptor\n", __FUNCTION__);
	} else {
		switch (desc->family) {
		case Altera_ACEX1K:
#if (CONFIG_FPGA & CFG_ACEX)
			PRINTF ("%s: Launching the ACEX1K Reader...\n",
					__FUNCTION__);
			ret_val = ACEX1K_dump (desc, buf, bsize);
#else
			printf ("%s: No support for ACEX1K devices.\n",
					__FUNCTION__);
#endif
			break;

		default:
			printf ("%s: Unsupported family type, %d\n",
					__FUNCTION__, desc->family);
		}
	}

	return ret_val;
}

int altera_info( Altera_desc *desc )
{
	int ret_val = FPGA_FAIL;

	if (altera_validate (desc, __FUNCTION__)) {
		printf ("Family:        \t");
		switch (desc->family) {
		case Altera_ACEX1K:
			printf ("ACEX1K\n");
			break;
			/* Add new family types here */
		default:
			printf ("Unknown family type, %d\n", desc->family);
		}

		printf ("Interface type:\t");
		switch (desc->iface) {
		case passive_serial:
			printf ("Passive Serial (PS)\n");
			break;
		case passive_parallel_synchronous:
			printf ("Passive Parallel Synchronous (PPS)\n");
			break;
		case passive_parallel_asynchronous:
			printf ("Passive Parallel Asynchronous (PPA)\n");
			break;
		case passive_serial_asynchronous:
			printf ("Passive Serial Asynchronous (PSA)\n");
			break;
		case altera_jtag_mode:		/* Not used */
			printf ("JTAG Mode\n");
			break;
			/* Add new interface types here */
		default:
			printf ("Unsupported interface type, %d\n", desc->iface);
		}

		printf ("Device Size:   \t%d bytes\n"
				"Cookie:        \t0x%x (%d)\n",
				desc->size, desc->cookie, desc->cookie);

		if (desc->iface_fns) {
			printf ("Device Function Table @ 0x%p\n", desc->iface_fns);
			switch (desc->family) {
			case Altera_ACEX1K:
#if (CONFIG_FPGA & CFG_ACEX1K)
				ACEX1K_info (desc);
#else
				/* just in case */
				printf ("%s: No support for ACEX1K devices.\n",
						__FUNCTION__);
#endif
				break;
				/* Add new family types here */
			default:
				/* we don't need a message here - we give one up above */
				break;
			}
		} else {
			printf ("No Device Function Table.\n");
		}

		ret_val = FPGA_SUCCESS;
	} else {
		printf ("%s: Invalid device descriptor\n", __FUNCTION__);
	}

	return ret_val;
}

int altera_reloc( Altera_desc *desc, ulong reloc_offset)
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (!altera_validate (desc, __FUNCTION__)) {
		printf ("%s: Invalid device descriptor\n", __FUNCTION__);
	} else {
		switch (desc->family) {
		case Altera_ACEX1K:
#if (CONFIG_FPGA & CFG_ACEX1K)
			ret_val = ACEX1K_reloc (desc, reloc_offset);
#else
			printf ("%s: No support for ACEX devices.\n",
					__FUNCTION__);
#endif
			break;
			/* Add new family types here */
		default:
			printf ("%s: Unsupported family type, %d\n",
					__FUNCTION__, desc->family);
		}
	}

	return ret_val;
}

/* ------------------------------------------------------------------------- */

static int altera_validate (Altera_desc * desc, char *fn)
{
	int ret_val = FALSE;

	if (desc) {
		if ((desc->family > min_altera_type) &&
			(desc->family < max_altera_type)) {
			if ((desc->iface > min_altera_iface_type) &&
				(desc->iface < max_altera_iface_type)) {
				if (desc->size) {
					ret_val = TRUE;
				} else {
					printf ("%s: NULL part size\n", fn);
				}
			} else {
				printf ("%s: Invalid Interface type, %d\n",
					fn, desc->iface);
			}
		} else {
			printf ("%s: Invalid family type, %d\n", fn, desc->family);
		}
	} else {
		printf ("%s: NULL descriptor!\n", fn);
	}

	return ret_val;
}

/* ------------------------------------------------------------------------- */

#endif /* CONFIG_FPGA & CFG_FPGA_ALTERA */
