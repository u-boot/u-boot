/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com
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
 *  Xilinx FPGA support
 */

#include <common.h>
#include <virtex2.h>
#include <spartan2.h>
#include <spartan3.h>

#if 0
#define FPGA_DEBUG
#endif

/* Define FPGA_DEBUG to get debug printf's */
#ifdef	FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* Local Static Functions */
static int xilinx_validate (Xilinx_desc * desc, char *fn);

/* ------------------------------------------------------------------------- */

int xilinx_load(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (!xilinx_validate (desc, (char *)__FUNCTION__)) {
		printf ("%s: Invalid device descriptor\n", __FUNCTION__);
	} else
		switch (desc->family) {
		case Xilinx_Spartan2:
#if defined(CONFIG_FPGA_SPARTAN2)
			PRINTF ("%s: Launching the Spartan-II Loader...\n",
					__FUNCTION__);
			ret_val = Spartan2_load (desc, buf, bsize);
#else
			printf ("%s: No support for Spartan-II devices.\n",
					__FUNCTION__);
#endif
			break;
		case Xilinx_Spartan3:
#if defined(CONFIG_FPGA_SPARTAN3)
			PRINTF ("%s: Launching the Spartan-III Loader...\n",
					__FUNCTION__);
			ret_val = Spartan3_load (desc, buf, bsize);
#else
			printf ("%s: No support for Spartan-III devices.\n",
					__FUNCTION__);
#endif
			break;
		case Xilinx_Virtex2:
#if defined(CONFIG_FPGA_VIRTEX2)
			PRINTF ("%s: Launching the Virtex-II Loader...\n",
					__FUNCTION__);
			ret_val = Virtex2_load (desc, buf, bsize);
#else
			printf ("%s: No support for Virtex-II devices.\n",
					__FUNCTION__);
#endif
			break;

		default:
			printf ("%s: Unsupported family type, %d\n",
					__FUNCTION__, desc->family);
		}

	return ret_val;
}

int xilinx_dump(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (!xilinx_validate (desc, (char *)__FUNCTION__)) {
		printf ("%s: Invalid device descriptor\n", __FUNCTION__);
	} else
		switch (desc->family) {
		case Xilinx_Spartan2:
#if defined(CONFIG_FPGA_SPARTAN2)
			PRINTF ("%s: Launching the Spartan-II Reader...\n",
					__FUNCTION__);
			ret_val = Spartan2_dump (desc, buf, bsize);
#else
			printf ("%s: No support for Spartan-II devices.\n",
					__FUNCTION__);
#endif
			break;
		case Xilinx_Spartan3:
#if defined(CONFIG_FPGA_SPARTAN3)
			PRINTF ("%s: Launching the Spartan-III Reader...\n",
					__FUNCTION__);
			ret_val = Spartan3_dump (desc, buf, bsize);
#else
			printf ("%s: No support for Spartan-III devices.\n",
					__FUNCTION__);
#endif
			break;
		case Xilinx_Virtex2:
#if defined( CONFIG_FPGA_VIRTEX2)
			PRINTF ("%s: Launching the Virtex-II Reader...\n",
					__FUNCTION__);
			ret_val = Virtex2_dump (desc, buf, bsize);
#else
			printf ("%s: No support for Virtex-II devices.\n",
					__FUNCTION__);
#endif
			break;

		default:
			printf ("%s: Unsupported family type, %d\n",
					__FUNCTION__, desc->family);
		}

	return ret_val;
}

int xilinx_info (Xilinx_desc * desc)
{
	int ret_val = FPGA_FAIL;

	if (xilinx_validate (desc, (char *)__FUNCTION__)) {
		printf ("Family:        \t");
		switch (desc->family) {
		case Xilinx_Spartan2:
			printf ("Spartan-II\n");
			break;
		case Xilinx_Spartan3:
			printf ("Spartan-III\n");
			break;
		case Xilinx_Virtex2:
			printf ("Virtex-II\n");
			break;
			/* Add new family types here */
		default:
			printf ("Unknown family type, %d\n", desc->family);
		}

		printf ("Interface type:\t");
		switch (desc->iface) {
		case slave_serial:
			printf ("Slave Serial\n");
			break;
		case master_serial:	/* Not used */
			printf ("Master Serial\n");
			break;
		case slave_parallel:
			printf ("Slave Parallel\n");
			break;
		case jtag_mode:		/* Not used */
			printf ("JTAG Mode\n");
			break;
		case slave_selectmap:
			printf ("Slave SelectMap Mode\n");
			break;
		case master_selectmap:
			printf ("Master SelectMap Mode\n");
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
			case Xilinx_Spartan2:
#if defined(CONFIG_FPGA_SPARTAN2)
				Spartan2_info (desc);
#else
				/* just in case */
				printf ("%s: No support for Spartan-II devices.\n",
						__FUNCTION__);
#endif
				break;
			case Xilinx_Spartan3:
#if defined(CONFIG_FPGA_SPARTAN3)
				Spartan3_info (desc);
#else
				/* just in case */
				printf ("%s: No support for Spartan-III devices.\n",
						__FUNCTION__);
#endif
				break;
			case Xilinx_Virtex2:
#if defined(CONFIG_FPGA_VIRTEX2)
				Virtex2_info (desc);
#else
				/* just in case */
				printf ("%s: No support for Virtex-II devices.\n",
						__FUNCTION__);
#endif
				break;
				/* Add new family types here */
			default:
				/* we don't need a message here - we give one up above */
				;
			}
		} else
			printf ("No Device Function Table.\n");

		ret_val = FPGA_SUCCESS;
	} else {
		printf ("%s: Invalid device descriptor\n", __FUNCTION__);
	}

	return ret_val;
}

/* ------------------------------------------------------------------------- */

static int xilinx_validate (Xilinx_desc * desc, char *fn)
{
	int ret_val = FALSE;

	if (desc) {
		if ((desc->family > min_xilinx_type) &&
			(desc->family < max_xilinx_type)) {
			if ((desc->iface > min_xilinx_iface_type) &&
				(desc->iface < max_xilinx_iface_type)) {
				if (desc->size) {
					ret_val = TRUE;
				} else
					printf ("%s: NULL part size\n", fn);
			} else
				printf ("%s: Invalid Interface type, %d\n",
						fn, desc->iface);
		} else
			printf ("%s: Invalid family type, %d\n", fn, desc->family);
	} else
		printf ("%s: NULL descriptor!\n", fn);

	return ret_val;
}
