/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <ppc4xx.h>
#include <asm/processor.h>
#include <asm/io.h>

static void wait_init_complete(void)
{
	u32 val;

	do {
		mfsdram(mem_mcsts, val);
	} while (!(val & 0x80000000));
}

/*
 * early_sdram_init()
 *
 * As the name already indicates, this function is called very early
 * from start.S and configures the SDRAM with fixed values. This is needed,
 * since the 440EP has no internal SRAM and the 4kB NAND_SPL loader has
 * not enough free space to implement the complete I2C SPD DDR autodetection
 * routines. Therefore the Bamboo only supports the onboard 64MBytes of SDRAM
 * when booting from NAND flash.
 *
 * Note:
 * As found out by Eugene O'Brien <eugene.obrien@advantechamt.com>, the fixed
 * DDR setup has problems (U-Boot crashes randomly upon TFTP), when the DIMM
 * modules are still plugged in. So it is recommended to remove the DIMM
 * modules while using the NAND booting code with the fixed SDRAM setup!
 */
void early_sdram_init(void)
{
	/*
	 * Soft-reset SDRAM controller.
	 */
	mtsdr(sdr_srst, SDR0_SRST_DMC);
	mtsdr(sdr_srst, 0x00000000);

	/*
	 * Disable memory controller.
	 */
	mtsdram(mem_cfg0, 0x00000000);

	/*
	 * Setup some default
	 */
	mtsdram(mem_uabba, 0x00000000); /* ubba=0 (default)		*/
	mtsdram(mem_slio, 0x00000000);	/* rdre=0 wrre=0 rarw=0		*/
	mtsdram(mem_devopt, 0x00000000); /* dll=0 ds=0 (normal)		*/
	mtsdram(mem_wddctr, 0x00000000); /* wrcp=0 dcd=0		*/
	mtsdram(mem_clktr, 0x40000000); /* clkp=1 (90 deg wr) dcdt=0	*/

	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram(mem_b0cr, 0x00082001);
	mtsdram(mem_tr0, 0x41094012);
	mtsdram(mem_tr1, 0x8080083d);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x00*/
	mtsdram(mem_rtr, 0x04100000);	/* Interval 7.8µs @ 133MHz PLB	*/
	mtsdram(mem_cfg1, 0x00000000);	/* Self-refresh exit, disable PM*/

	/*
	 * Enable the controller, then wait for DCEN to complete
	 */
	mtsdram(mem_cfg0, 0x80000000); /* DCEN=1, PMUD=0*/
	wait_init_complete();
}

long int initdram(int board_type)
{
	/*
	 * Nothing to do here, just return size of fixed SDRAM setup
	 */
	return CFG_MBYTES_SDRAM << 20;
}
