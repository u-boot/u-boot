/*
 * (C) Copyright 2002
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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
#include <mpc8260.h>
#include <common.h>
#include "../common/fpga.h"

fpga_t fpga_list[] = {
	{"FIOX", CFG_FIOX_BASE,
	 CFG_PD_FIOX_INIT, CFG_PD_FIOX_PROG, CFG_PD_FIOX_DONE}
	,
	{"FDOHM", CFG_FDOHM_BASE,
	 CFG_PD_FDOHM_INIT, CFG_PD_FDOHM_PROG, CFG_PD_FDOHM_DONE}
};
int fpga_count = sizeof (fpga_list) / sizeof (fpga_t);


ulong fpga_control (fpga_t * fpga, int cmd)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	switch (cmd) {
	case FPGA_INIT_IS_HIGH:
		immr->im_ioport.iop_pdird &= ~fpga->init_mask;	/* input */
		return (immr->im_ioport.iop_pdatd & fpga->init_mask) ? 1 : 0;

	case FPGA_INIT_SET_LOW:
		immr->im_ioport.iop_pdird |= fpga->init_mask;	/* output */
		immr->im_ioport.iop_pdatd &= ~fpga->init_mask;
		break;

	case FPGA_INIT_SET_HIGH:
		immr->im_ioport.iop_pdird |= fpga->init_mask;	/* output */
		immr->im_ioport.iop_pdatd |= fpga->init_mask;
		break;

	case FPGA_PROG_SET_LOW:
		immr->im_ioport.iop_pdatd &= ~fpga->prog_mask;
		break;

	case FPGA_PROG_SET_HIGH:
		immr->im_ioport.iop_pdatd |= fpga->prog_mask;
		break;

	case FPGA_DONE_IS_HIGH:
		return (immr->im_ioport.iop_pdatd & fpga->done_mask) ? 1 : 0;

	case FPGA_READ_MODE:
		break;

	case FPGA_LOAD_MODE:
		break;

	case FPGA_GET_ID:
		if (fpga->conf_base == CFG_FIOX_BASE) {
			ulong ver =
				*(volatile ulong *) (fpga->conf_base + 0x10);
			return ((ver >> 10) & 0xf) + ((ver >> 2) & 0xf0);
		} else if (fpga->conf_base == CFG_FDOHM_BASE) {
			return (*(volatile ushort *) fpga->conf_base) & 0xff;
		} else {
			return *(volatile ulong *) fpga->conf_base;
		}

	case FPGA_INIT_PORTS:
		immr->im_ioport.iop_ppard &= ~fpga->init_mask;	/* INIT I/O */
		immr->im_ioport.iop_psord &= ~fpga->init_mask;
		immr->im_ioport.iop_pdird &= ~fpga->init_mask;

		immr->im_ioport.iop_ppard &= ~fpga->prog_mask;	/* PROG Output */
		immr->im_ioport.iop_psord &= ~fpga->prog_mask;
		immr->im_ioport.iop_pdird |= fpga->prog_mask;

		immr->im_ioport.iop_ppard &= ~fpga->done_mask;	/* DONE Input */
		immr->im_ioport.iop_psord &= ~fpga->done_mask;
		immr->im_ioport.iop_pdird &= ~fpga->done_mask;

		break;

	}
	return 0;
}
