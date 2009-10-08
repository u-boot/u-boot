/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
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
 * Virtex2 FPGA configuration support for the GEN860T computer
 */

#include <common.h>
#include <virtex2.h>
#include <command.h>
#include "fpga.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA)

#if 0
#define GEN860T_FPGA_DEBUG
#endif

#ifdef GEN860T_FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define	PRINTF(fmt,args...)
#endif

/*
 * Port bit numbers for the Selectmap controls
 */
#define FPGA_INIT_BIT_NUM		22	/* PB22 */
#define FPGA_RESET_BIT_NUM		11	/* PC11 */
#define FPGA_DONE_BIT_NUM		16	/* PB16 */
#define FPGA_PROGRAM_BIT_NUM	7	/* PA7  */

/* Note that these are pointers to code that is in Flash.  They will be
 * relocated at runtime.
 */
Xilinx_Virtex2_Slave_SelectMap_fns fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_init_fn,
	fpga_err_fn,
	fpga_done_fn,
	fpga_clk_fn,
	fpga_cs_fn,
	fpga_wr_fn,
	fpga_read_data_fn,
	fpga_write_data_fn,
	fpga_busy_fn,
	fpga_abort_fn,
	fpga_post_config_fn
};

Xilinx_desc fpga[CONFIG_FPGA_COUNT] = {
	{Xilinx_Virtex2,
	 slave_selectmap,
	 XILINX_XC2V3000_SIZE,
	 (void *) &fpga_fns,
	 0}
};

/*
 * Display FPGA revision information
 */
void print_fpga_revision (void)
{
	vu_long *rev_p = (vu_long *) 0x60000008;

	printf ("FPGA Revision 0x%.8lx"
		" (Date %.2lx/%.2lx/%.2lx, Status \"%.1lx\", Version %.3lu)\n",
		*rev_p,
		((*rev_p >> 28) & 0xf),
		((*rev_p >> 20) & 0xff),
		((*rev_p >> 12) & 0xff),
		((*rev_p >> 8) & 0xf), (*rev_p & 0xff));
}


/*
 * Perform a simple test of the FPGA to processor interface using the FPGA's
 * inverting bus test register.  The great thing about doing a read/write
 * test on a register that inverts it's contents is that you avoid any
 * problems with bus charging.
 * Return 0 on failure, 1 on success.
 */
int test_fpga_ibtr (void)
{
	vu_long *ibtr_p = (vu_long *) 0x60000010;
	vu_long readback;
	vu_long compare;
	int i;
	int j;
	int k;
	int pass = 1;

	static const ulong bitpattern[] = {
		0xdeadbeef,	/* magic ID pattern for debug   */
		0x00000001,	/* single bit                                   */
		0x00000003,	/* two adjacent bits                    */
		0x00000007,	/* three adjacent bits                  */
		0x0000000F,	/* four adjacent bits                   */
		0x00000005,	/* two non-adjacent bits                */
		0x00000015,	/* three non-adjacent bits              */
		0x00000055,	/* four non-adjacent bits               */
		0xaaaaaaaa,	/* alternating 1/0                              */
	};

	for (i = 0; i < 1024; i++) {
		for (j = 0; j < 31; j++) {
			for (k = 0;
			     k < sizeof (bitpattern) / sizeof (bitpattern[0]);
			     k++) {
				*ibtr_p = compare = (bitpattern[k] << j);
				readback = *ibtr_p;
				if (readback != ~compare) {
					printf ("%s:%d: FPGA test fail: expected 0x%.8lx" " actual 0x%.8lx\n", __FUNCTION__, __LINE__, ~compare, readback);
					pass = 0;
					break;
				}
			}
			if (!pass)
				break;
		}
		if (!pass)
			break;
	}
	if (pass) {
		printf ("FPGA inverting bus test passed\n");
		print_fpga_revision ();
	} else {
		printf ("** FPGA inverting bus test failed\n");
	}
	return pass;
}


/*
 * Set the active-low FPGA reset signal.
 */
void fpga_reset (int assert)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	PRINTF ("%s:%d: RESET ", __FUNCTION__, __LINE__);
	if (assert) {
		immap->im_ioport.iop_pcdat &= ~(0x8000 >> FPGA_RESET_BIT_NUM);
		PRINTF ("asserted\n");
	} else {
		immap->im_ioport.iop_pcdat |= (0x8000 >> FPGA_RESET_BIT_NUM);
		PRINTF ("deasserted\n");
	}
}


/*
 * Initialize the SelectMap interface.  We assume that the mode and the
 * initial state of all of the port pins have already been set!
 */
void fpga_selectmap_init (void)
{
	PRINTF ("%s:%d: Initialize SelectMap interface\n", __FUNCTION__,
		__LINE__);
	fpga_pgm_fn (FALSE, FALSE, 0);	/* make sure program pin is inactive */
}


/*
 * Initialize the fpga.  Return 1 on success, 0 on failure.
 */
int gen860t_init_fpga (void)
{
	int i;

	PRINTF ("%s:%d: Initialize FPGA interface\n",
		__FUNCTION__, __LINE__);
	fpga_init ();
	fpga_selectmap_init ();

	for (i = 0; i < CONFIG_FPGA_COUNT; i++) {
		PRINTF ("%s:%d: Adding fpga %d\n", __FUNCTION__, __LINE__, i);
		fpga_add (fpga_xilinx, &fpga[i]);
	}
	return 1;
}


/*
 * Set the FPGA's active-low SelectMap program line to the specified level
 */
int fpga_pgm_fn (int assert, int flush, int cookie)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	PRINTF ("%s:%d: FPGA PROGRAM ", __FUNCTION__, __LINE__);

	if (assert) {
		immap->im_ioport.iop_padat &=
			~(0x8000 >> FPGA_PROGRAM_BIT_NUM);
		PRINTF ("asserted\n");
	} else {
		immap->im_ioport.iop_padat |=
			(0x8000 >> FPGA_PROGRAM_BIT_NUM);
		PRINTF ("deasserted\n");
	}
	return assert;
}


/*
 * Test the state of the active-low FPGA INIT line.  Return 1 on INIT
 * asserted (low).
 */
int fpga_init_fn (int cookie)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	PRINTF ("%s:%d: INIT check... ", __FUNCTION__, __LINE__);
	if (immap->im_cpm.cp_pbdat & (0x80000000 >> FPGA_INIT_BIT_NUM)) {
		PRINTF ("high\n");
		return 0;
	} else {
		PRINTF ("low\n");
		return 1;
	}
}


/*
 * Test the state of the active-high FPGA DONE pin
 */
int fpga_done_fn (int cookie)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	PRINTF ("%s:%d: DONE check... ", __FUNCTION__, __LINE__);
	if (immap->im_cpm.cp_pbdat & (0x80000000 >> FPGA_DONE_BIT_NUM)) {
		PRINTF ("high\n");
		return FPGA_SUCCESS;
	} else {
		PRINTF ("low\n");
		return FPGA_FAIL;
	}
}


/*
 * Read FPGA SelectMap data.
 */
int fpga_read_data_fn (unsigned char *data, int cookie)
{
	vu_char *p = (vu_char *) SELECTMAP_BASE;

	*data = *p;
#if 0
	PRINTF ("%s: Read 0x%x into 0x%p\n", __FUNCTION__, (int) data, data);
#endif
	return (int) data;
}


/*
 * Write data to the FPGA SelectMap port
 */
int fpga_write_data_fn (unsigned char data, int flush, int cookie)
{
	vu_char *p = (vu_char *) SELECTMAP_BASE;

#if 0
	PRINTF ("%s: Write Data 0x%x\n", __FUNCTION__, (int) data);
#endif
	*p = data;
	return (int) data;
}


/*
 * Abort and FPGA operation
 */
int fpga_abort_fn (int cookie)
{
	PRINTF ("%s:%d: FPGA program sequence aborted\n",
		__FUNCTION__, __LINE__);
	return FPGA_FAIL;
}


/*
 * FPGA pre-configuration function. Just make sure that
 * FPGA reset is asserted to keep the FPGA from starting up after
 * configuration.
 */
int fpga_pre_config_fn (int cookie)
{
	PRINTF ("%s:%d: FPGA pre-configuration\n", __FUNCTION__, __LINE__);
	fpga_reset (TRUE);
	return 0;
}


/*
 * FPGA post configuration function. Blip the FPGA reset line and then see if
 * the FPGA appears to be running.
 */
int fpga_post_config_fn (int cookie)
{
	int rc;

	PRINTF ("%s:%d: FPGA post configuration\n", __FUNCTION__, __LINE__);
	fpga_reset (TRUE);
	udelay (1000);
	fpga_reset (FALSE);
	udelay (1000);

	/*
	 * Use the FPGA,s inverting bus test register to do a simple test of the
	 * processor interface.
	 */
	rc = test_fpga_ibtr ();
	return rc;
}


/*
 * Clock, chip select and write signal assert functions and error check
 * and busy functions.  These are only stubs because the GEN860T selectmap
 * interface handles sequencing of control signals automatically (it uses
 * a memory-mapped interface to the FPGA SelectMap port).  The design of
 * the interface guarantees that the SelectMap port cannot be overrun so
 * no busy check is needed.  A configuration error is signalled by INIT
 * going low during configuration, so there is no need for a separate error
 * function.
 */
int fpga_clk_fn (int assert_clk, int flush, int cookie)
{
	return assert_clk;
}

int fpga_cs_fn (int assert_cs, int flush, int cookie)
{
	return assert_cs;
}

int fpga_wr_fn (int assert_write, int flush, int cookie)
{
	return assert_write;
}

int fpga_err_fn (int cookie)
{
	return 0;
}

int fpga_busy_fn (int cookie)
{
	return 0;
}
#endif
