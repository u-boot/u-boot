/*
 * (C) Copyright 2006
 * Heiko Schocher, hs@denx.de
 * Based on ACE1XK.c
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
 */

#include <common.h>		/* core U-Boot definitions */
#include <altera.h>
#include <ACEX1K.h>		/* ACEX device family */

#if (CONFIG_FPGA & (CFG_ALTERA | CFG_CYCLON2))

/* Define FPGA_DEBUG to get debug printf's */
#ifdef	FPGA_DEBUG
#define PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* Note: The assumption is that we cannot possibly run fast enough to
 * overrun the device (the Slave Parallel mode can free run at 50MHz).
 * If there is a need to operate slower, define CONFIG_FPGA_DELAY in
 * the board config file to slow things down.
 */
#ifndef CONFIG_FPGA_DELAY
#define CONFIG_FPGA_DELAY()
#endif

#ifndef CFG_FPGA_WAIT
#define CFG_FPGA_WAIT CFG_HZ/10		/* 100 ms */
#endif

static int CYC2_ps_load( Altera_desc *desc, void *buf, size_t bsize );
static int CYC2_ps_dump( Altera_desc *desc, void *buf, size_t bsize );
/* static int CYC2_ps_info( Altera_desc *desc ); */
static int CYC2_ps_reloc( Altera_desc *desc, ulong reloc_offset );

/* ------------------------------------------------------------------------- */
/* CYCLON2 Generic Implementation */
int CYC2_load (Altera_desc * desc, void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		PRINTF ("%s: Launching Passive Serial Loader\n", __FUNCTION__);
		ret_val = CYC2_ps_load (desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}

	return ret_val;
}

int CYC2_dump (Altera_desc * desc, void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		PRINTF ("%s: Launching Passive Serial Dump\n", __FUNCTION__);
		ret_val = CYC2_ps_dump (desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}

	return ret_val;
}

int CYC2_info( Altera_desc *desc )
{
	return FPGA_SUCCESS;
}

int CYC2_reloc (Altera_desc * desc, ulong reloc_offset)
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (desc->family != Altera_CYC2) {
		printf ("%s: Unsupported family type, %d\n",
				__FUNCTION__, desc->family);
		return FPGA_FAIL;
	} else
		switch (desc->iface) {
		case passive_serial:
			ret_val = CYC2_ps_reloc (desc, reloc_offset);
			break;

		/* Add new interface types here */

		default:
			printf ("%s: Unsupported interface type, %d\n",
					__FUNCTION__, desc->iface);
		}

	return ret_val;
}

/* ------------------------------------------------------------------------- */
/* CYCLON2 Passive Serial Generic Implementation                                  */
static int CYC2_ps_load (Altera_desc * desc, void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume the worst */
	Altera_CYC2_Passive_Serial_fns *fn = desc->iface_fns;
	int	ret = 0;

	PRINTF ("%s: start with interface functions @ 0x%p\n",
			__FUNCTION__, fn);

	if (fn) {
		int cookie = desc->cookie;	/* make a local copy */
		unsigned long ts;		/* timestamp */

		PRINTF ("%s: Function Table:\n"
				"ptr:\t0x%p\n"
				"struct: 0x%p\n"
				"config:\t0x%p\n"
				"status:\t0x%p\n"
				"write:\t0x%p\n"
				"done:\t0x%p\n\n",
				__FUNCTION__, &fn, fn, fn->config, fn->status,
				fn->write, fn->done);
#ifdef CFG_FPGA_PROG_FEEDBACK
		printf ("Loading FPGA Device %d...", cookie);
#endif

		/*
		 * Run the pre configuration function if there is one.
		 */
		if (*fn->pre) {
			(*fn->pre) (cookie);
		}

		/* Establish the initial state */
		(*fn->config) (TRUE, TRUE, cookie);	/* Assert nCONFIG */

		udelay(2);		/* T_cfg > 2us	*/

		/* Wait for nSTATUS to be asserted */
		ts = get_timer (0);		/* get current time */
		do {
			CONFIG_FPGA_DELAY ();
			if (get_timer (ts) > CFG_FPGA_WAIT) {	/* check the time */
				puts ("** Timeout waiting for STATUS to go high.\n");
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
		} while (!(*fn->status) (cookie));

		/* Get ready for the burn */
		CONFIG_FPGA_DELAY ();

		ret = (*fn->write) (buf, bsize, TRUE, cookie);
		if (ret) {
			puts ("** Write failed.\n");
			(*fn->abort) (cookie);
			return FPGA_FAIL;
		}
#ifdef CFG_FPGA_PROG_FEEDBACK
		puts(" OK? ...");
#endif

		CONFIG_FPGA_DELAY ();

#ifdef CFG_FPGA_PROG_FEEDBACK
		putc (' ');			/* terminate the dotted line */
#endif

	/*
	 * Checking FPGA's CONF_DONE signal - correctly booted ?
	 */

	if ( ! (*fn->done) (cookie) ) {
		puts ("** Booting failed! CONF_DONE is still deasserted.\n");
		(*fn->abort) (cookie);
		return (FPGA_FAIL);
	}
#ifdef CFG_FPGA_PROG_FEEDBACK
	puts(" OK\n");
#endif

	ret_val = FPGA_SUCCESS;

#ifdef CFG_FPGA_PROG_FEEDBACK
	if (ret_val == FPGA_SUCCESS) {
		puts ("Done.\n");
	}
	else {
		puts ("Fail.\n");
	}
#endif
	(*fn->post) (cookie);

	} else {
		printf ("%s: NULL Interface function table!\n", __FUNCTION__);
	}

	return ret_val;
}

static int CYC2_ps_dump (Altera_desc * desc, void *buf, size_t bsize)
{
	/* Readback is only available through the Slave Parallel and         */
	/* boundary-scan interfaces.                                         */
	printf ("%s: Passive Serial Dumping is unavailable\n",
			__FUNCTION__);
	return FPGA_FAIL;
}

static int CYC2_ps_reloc (Altera_desc * desc, ulong reloc_offset)
{
	int ret_val = FPGA_FAIL;	/* assume the worst */
	Altera_CYC2_Passive_Serial_fns *fn_r, *fn =
			(Altera_CYC2_Passive_Serial_fns *) (desc->iface_fns);

	if (fn) {
		ulong addr;

		/* Get the relocated table address */
		addr = (ulong) fn + reloc_offset;
		fn_r = (Altera_CYC2_Passive_Serial_fns *) addr;

		if (!fn_r->relocated) {

			if (memcmp (fn_r, fn,
						sizeof (Altera_CYC2_Passive_Serial_fns))
				== 0) {
				/* good copy of the table, fix the descriptor pointer */
				desc->iface_fns = fn_r;
			} else {
				PRINTF ("%s: Invalid function table at 0x%p\n",
						__FUNCTION__, fn_r);
				return FPGA_FAIL;
			}

			PRINTF ("%s: Relocating descriptor at 0x%p\n", __FUNCTION__,
					desc);

			addr = (ulong) (fn->pre) + reloc_offset;
			fn_r->pre = (Altera_pre_fn) addr;

			addr = (ulong) (fn->config) + reloc_offset;
			fn_r->config = (Altera_config_fn) addr;

			addr = (ulong) (fn->status) + reloc_offset;
			fn_r->status = (Altera_status_fn) addr;

			addr = (ulong) (fn->done) + reloc_offset;
			fn_r->done = (Altera_done_fn) addr;

			addr = (ulong) (fn->write) + reloc_offset;
			fn_r->write = (Altera_write_fn) addr;

			addr = (ulong) (fn->abort) + reloc_offset;
			fn_r->abort = (Altera_abort_fn) addr;

			addr = (ulong) (fn->post) + reloc_offset;
			fn_r->post = (Altera_post_fn) addr;

			fn_r->relocated = TRUE;

		} else {
			/* this table has already been moved */
			/* XXX - should check to see if the descriptor is correct */
			desc->iface_fns = fn_r;
		}

		ret_val = FPGA_SUCCESS;
	} else {
		printf ("%s: NULL Interface function table!\n", __FUNCTION__);
	}

	return ret_val;
}

#endif /* (CONFIG_FPGA & (CFG_ALTERA | CFG_CYCLON2)) */
