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
#include <mpc8xx.h>
#include <commproc.h>
#include <common.h>

#include "../common/fpga.h"

fpga_t fpga_list[] = {
    { "PUMA" , PUMA_CONF_BASE ,
      CONFIG_SYS_PC_PUMA_INIT , CONFIG_SYS_PC_PUMA_PROG , CONFIG_SYS_PC_PUMA_DONE  }
};
int fpga_count = sizeof(fpga_list) / sizeof(fpga_t);

void can_driver_enable (void);
void can_driver_disable (void);

#define	_NOT_USED_	0xFFFFFFFF

/*
 * PUMA access using UPM B
 */
const uint puma_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_,
	/*
	 * Precharge and MRS
	 */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Read. (Offset 8 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x0FFCF804, 0x0FFCF400, 0x3FFDFC47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPM RAM)
	 */
	0x7FFFFC07, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
};


ulong fpga_control (fpga_t* fpga, int cmd)
{
    volatile immap_t     *immr  = (immap_t *)CONFIG_SYS_IMMR;
    volatile memctl8xx_t *memctl = &immr->im_memctl;

    switch (cmd) {
    case FPGA_INIT_IS_HIGH:
	immr->im_ioport.iop_pcdir &= ~fpga->init_mask; /* input */
	return (immr->im_ioport.iop_pcdat & fpga->init_mask) ? 1:0;

    case FPGA_INIT_SET_LOW:
	immr->im_ioport.iop_pcdir |=  fpga->init_mask; /* output */
	immr->im_ioport.iop_pcdat &= ~fpga->init_mask;
	break;

    case FPGA_INIT_SET_HIGH:
	immr->im_ioport.iop_pcdir |= fpga->init_mask; /* output */
	immr->im_ioport.iop_pcdat |= fpga->init_mask;
	break;

    case FPGA_PROG_SET_LOW:
	immr->im_ioport.iop_pcdat &= ~fpga->prog_mask;
	break;

    case FPGA_PROG_SET_HIGH:
	immr->im_ioport.iop_pcdat |= fpga->prog_mask;
	break;

    case FPGA_DONE_IS_HIGH:
	return (immr->im_ioport.iop_pcdat & fpga->done_mask) ? 1:0;

    case FPGA_READ_MODE:
	/* disable FPGA in memory controller */
	memctl->memc_br4 = 0;
	memctl->memc_or4 = PUMA_CONF_OR_READ;
	memctl->memc_br4 = PUMA_CONF_BR_READ;

	/* (re-) enable CAN drivers */
	can_driver_enable ();

	break;

    case FPGA_LOAD_MODE:
	/* disable FPGA in memory controller */
	memctl->memc_br4 = 0;
	/*
	 * We must disable the CAN drivers first because
	 * they use UPM B, too.
	 */
	can_driver_disable ();
	/*
	 * Configure UPMB for FPGA
	 */
	upmconfig(UPMB,(uint *)puma_table,sizeof(puma_table)/sizeof(uint));
	memctl->memc_or4 = PUMA_CONF_OR_LOAD;
	memctl->memc_br4 = PUMA_CONF_BR_LOAD;
	break;

    case FPGA_GET_ID:
	return *(volatile ulong *)fpga->conf_base;

    case FPGA_INIT_PORTS:
	immr->im_ioport.iop_pcpar &= ~fpga->init_mask; /* INIT I/O */
	immr->im_ioport.iop_pcso  &= ~fpga->init_mask;
	immr->im_ioport.iop_pcdir &= ~fpga->init_mask;

	immr->im_ioport.iop_pcpar &= ~fpga->prog_mask; /* PROG Output */
	immr->im_ioport.iop_pcso  &= ~fpga->prog_mask;
	immr->im_ioport.iop_pcdir |=  fpga->prog_mask;

	immr->im_ioport.iop_pcpar &= ~fpga->done_mask; /* DONE Input */
	immr->im_ioport.iop_pcso  &= ~fpga->done_mask;
	immr->im_ioport.iop_pcdir &= ~fpga->done_mask;

	break;

    }
    return 0;
}
