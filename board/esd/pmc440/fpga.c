/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com.
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
#include <asm/io.h>
#include <spartan2.h>
#include <spartan3.h>
#include <command.h>
#include "fpga.h"
#include "pmc440.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA)

#define USE_SP_CODE

#ifdef USE_SP_CODE
Xilinx_Spartan3_Slave_Parallel_fns pmc440_fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_init_fn,
	NULL, /* err */
	fpga_done_fn,
	fpga_clk_fn,
	fpga_cs_fn,
	fpga_wr_fn,
	NULL, /* rdata */
	fpga_wdata_fn,
	fpga_busy_fn,
	fpga_abort_fn,
	fpga_post_config_fn,
};
#else
Xilinx_Spartan3_Slave_Serial_fns pmc440_fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_clk_fn,
	fpga_init_fn,
	fpga_done_fn,
	fpga_wr_fn,
	fpga_post_config_fn,
};
#endif

Xilinx_Spartan2_Slave_Serial_fns ngcc_fpga_fns = {
	ngcc_fpga_pre_config_fn,
	ngcc_fpga_pgm_fn,
	ngcc_fpga_clk_fn,
	ngcc_fpga_init_fn,
	ngcc_fpga_done_fn,
	ngcc_fpga_wr_fn,
	ngcc_fpga_post_config_fn
};

Xilinx_desc fpga[CONFIG_FPGA_COUNT] = {
	XILINX_XC3S1200E_DESC(
#ifdef USE_SP_CODE
		slave_parallel,
#else
		slave_serial,
#endif
		(void *)&pmc440_fpga_fns,
		0),
	XILINX_XC2S200_DESC(
		slave_serial,
		(void *)&ngcc_fpga_fns,
		0)
};


/*
 * Set the active-low FPGA reset signal.
 */
void fpga_reset(int assert)
{
	debug("%s:%d: RESET ", __FUNCTION__, __LINE__);
	if (assert) {
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) & ~GPIO1_FPGA_DATA);
		debug("asserted\n");
	} else {
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) | GPIO1_FPGA_DATA);
		debug("deasserted\n");
	}
}


/*
 * Initialize the SelectMap interface.  We assume that the mode and the
 * initial state of all of the port pins have already been set!
 */
void fpga_serialslave_init(void)
{
	debug("%s:%d: Initialize serial slave interface\n", __FUNCTION__,
	      __LINE__);
	fpga_pgm_fn(FALSE, FALSE, 0);	/* make sure program pin is inactive */
}


/*
 * Set the FPGA's active-low SelectMap program line to the specified level
 */
int fpga_pgm_fn(int assert, int flush, int cookie)
{
	debug("%s:%d: FPGA PROGRAM ",
	      __FUNCTION__, __LINE__);

	if (assert) {
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) & ~GPIO1_FPGA_PRG);
		debug("asserted\n");
	} else {
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) | GPIO1_FPGA_PRG);
		debug("deasserted\n");
	}
	return assert;
}


/*
 * Test the state of the active-low FPGA INIT line.  Return 1 on INIT
 * asserted (low).
 */
int fpga_init_fn(int cookie)
{
	if (in_be32((void*)GPIO1_IR) & GPIO1_FPGA_INIT)
		return 0;
	else
		return 1;
}

#ifdef USE_SP_CODE
int fpga_abort_fn(int cookie)
{
	return 0;
}


int fpga_cs_fn(int assert_cs, int flush, int cookie)
{
	return assert_cs;
}


int fpga_busy_fn(int cookie)
{
	return 1;
}
#endif


/*
 * Test the state of the active-high FPGA DONE pin
 */
int fpga_done_fn(int cookie)
{
	if (in_be32((void*)GPIO1_IR) & GPIO1_FPGA_DONE)
		return 1;
	else
		return 0;
}


/*
 * FPGA pre-configuration function. Just make sure that
 * FPGA reset is asserted to keep the FPGA from starting up after
 * configuration.
 */
int fpga_pre_config_fn(int cookie)
{
	debug("%s:%d: FPGA pre-configuration\n", __FUNCTION__, __LINE__);
	fpga_reset(TRUE);

	/* release init# */
	out_be32((void*)GPIO0_OR, in_be32((void*)GPIO0_OR) | GPIO0_FPGA_FORCEINIT);
	/* disable PLD IOs */
	out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) | GPIO1_IOEN_N);
	return 0;
}


/*
 * FPGA post configuration function. Blip the FPGA reset line and then see if
 * the FPGA appears to be running.
 */
int fpga_post_config_fn(int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;
	int rc=0;
	char *s;

	debug("%s:%d: FPGA post configuration\n", __FUNCTION__, __LINE__);

	/* enable PLD0..7 pins */
	out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) & ~GPIO1_IOEN_N);

	fpga_reset(TRUE);
	udelay (100);
	fpga_reset(FALSE);
	udelay (100);

	FPGA_OUT32(&fpga->status, (gd->board_type << STATUS_HWREV_SHIFT) & STATUS_HWREV_MASK);

	/* NGCC/CANDES only: enable ledlink */
	if ((s = getenv("bd_type")) &&
	    ((!strcmp(s, "ngcc")) || (!strcmp(s, "candes"))))
		FPGA_SETBITS(&fpga->ctrla, 0x29f8c000);

	return rc;
}


int fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	if (assert_clk)
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) | GPIO1_FPGA_CLK);
	else
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) & ~GPIO1_FPGA_CLK);

	return assert_clk;
}


int fpga_wr_fn(int assert_write, int flush, int cookie)
{
	if (assert_write)
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) | GPIO1_FPGA_DATA);
	else
		out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) & ~GPIO1_FPGA_DATA);

	return assert_write;
}

#ifdef USE_SP_CODE
int fpga_wdata_fn(uchar data, int flush, int cookie)
{
	uchar val = data;
	ulong or = in_be32((void*)GPIO1_OR);
	int i = 7;
	do {
		/* Write data */
		if (val & 0x80)
			or = (or & ~GPIO1_FPGA_CLK) | GPIO1_FPGA_DATA;
		else
			or = or & ~(GPIO1_FPGA_CLK | GPIO1_FPGA_DATA);

		out_be32((void*)GPIO1_OR, or);

		/* Assert the clock */
		or |= GPIO1_FPGA_CLK;
		out_be32((void*)GPIO1_OR, or);
		val <<= 1;
		i --;
	} while (i > 0);

	/* Write last data bit (the 8th clock comes from the sp_load() code */
	if (val & 0x80)
		or = (or & ~GPIO1_FPGA_CLK) | GPIO1_FPGA_DATA;
	else
		or = or & ~(GPIO1_FPGA_CLK | GPIO1_FPGA_DATA);

	out_be32((void*)GPIO1_OR, or);

	return 0;
}
#endif

#define NGCC_FPGA_PRG  CLOCK_EN
#define NGCC_FPGA_DATA RESET_OUT
#define NGCC_FPGA_DONE CLOCK_IN
#define NGCC_FPGA_INIT IRIGB_R_IN
#define NGCC_FPGA_CLK  CLOCK_OUT

void ngcc_fpga_serialslave_init(void)
{
	debug("%s:%d: Initialize serial slave interface\n",
	      __FUNCTION__, __LINE__);

	/* make sure program pin is inactive */
	ngcc_fpga_pgm_fn (FALSE, FALSE, 0);
}

/*
 * Set the active-low FPGA reset signal.
 */
void ngcc_fpga_reset(int assert)
{
	debug("%s:%d: RESET ", __FUNCTION__, __LINE__);

	if (assert) {
		FPGA_CLRBITS(NGCC_CTRL_BASE, NGCC_CTRL_FPGARST_N);
		debug("asserted\n");
	} else {
		FPGA_SETBITS(NGCC_CTRL_BASE, NGCC_CTRL_FPGARST_N);
		debug("deasserted\n");
	}
}


/*
 * Set the FPGA's active-low SelectMap program line to the specified level
 */
int ngcc_fpga_pgm_fn(int assert, int flush, int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	debug("%s:%d: FPGA PROGRAM ", __FUNCTION__, __LINE__);

	if (assert) {
		FPGA_CLRBITS(&fpga->ctrla, NGCC_FPGA_PRG);
		debug("asserted\n");
	} else {
		FPGA_SETBITS(&fpga->ctrla, NGCC_FPGA_PRG);
		debug("deasserted\n");
	}

	return assert;
}


/*
 * Test the state of the active-low FPGA INIT line.  Return 1 on INIT
 * asserted (low).
 */
int ngcc_fpga_init_fn(int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	debug("%s:%d: INIT check... ", __FUNCTION__, __LINE__);
	if (FPGA_IN32(&fpga->status) & NGCC_FPGA_INIT) {
		debug("high\n");
		return 0;
	} else {
		debug("low\n");
		return 1;
	}
}


/*
 * Test the state of the active-high FPGA DONE pin
 */
int ngcc_fpga_done_fn(int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	debug("%s:%d: DONE check... ", __FUNCTION__, __LINE__);
	if (FPGA_IN32(&fpga->status) & NGCC_FPGA_DONE) {
		debug("DONE high\n");
		return 1;
	} else {
		debug("low\n");
		return 0;
	}
}


/*
 * FPGA pre-configuration function.
 */
int ngcc_fpga_pre_config_fn(int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;
	debug("%s:%d: FPGA pre-configuration\n", __FUNCTION__, __LINE__);

	ngcc_fpga_reset(TRUE);
	FPGA_CLRBITS(&fpga->ctrla, 0xfffffe00);

	ngcc_fpga_reset(TRUE);
	return 0;
}


/*
 * FPGA post configuration function. Blip the FPGA reset line and then see if
 * the FPGA appears to be running.
 */
int ngcc_fpga_post_config_fn(int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	debug("%s:%d: NGCC FPGA post configuration\n", __FUNCTION__, __LINE__);

	udelay (100);
	ngcc_fpga_reset(FALSE);

	FPGA_SETBITS(&fpga->ctrla, 0x29f8c000);

	return 0;
}


int ngcc_fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	if (assert_clk)
		FPGA_SETBITS(&fpga->ctrla, NGCC_FPGA_CLK);
	else
		FPGA_CLRBITS(&fpga->ctrla, NGCC_FPGA_CLK);

	return assert_clk;
}


int ngcc_fpga_wr_fn(int assert_write, int flush, int cookie)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	if (assert_write)
		FPGA_SETBITS(&fpga->ctrla, NGCC_FPGA_DATA);
	else
		FPGA_CLRBITS(&fpga->ctrla, NGCC_FPGA_DATA);

	return assert_write;
}


/*
 * Initialize the fpga.  Return 1 on success, 0 on failure.
 */
int pmc440_init_fpga(void)
{
	char *s;

	debug("%s:%d: Initialize FPGA interface (relocation offset = 0x%.8lx)\n",
	      __FUNCTION__, __LINE__, gd->reloc_off);
	fpga_init(gd->reloc_off);

	fpga_serialslave_init ();
	debug("%s:%d: Adding fpga 0\n", __FUNCTION__, __LINE__);
	fpga_add (fpga_xilinx, &fpga[0]);

	/* NGCC only */
	if ((s = getenv("bd_type")) && !strcmp(s, "ngcc")) {
		ngcc_fpga_serialslave_init ();
		debug("%s:%d: Adding fpga 1\n", __FUNCTION__, __LINE__);
		fpga_add (fpga_xilinx, &fpga[1]);
	}

	return 0;
}
#endif /* CONFIG_FPGA */
