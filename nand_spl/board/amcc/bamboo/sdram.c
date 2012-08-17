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
#include <asm/ppc4xx.h>
#include <asm/processor.h>
#include <asm/io.h>

static void wait_init_complete(void)
{
	u32 val;

	do {
		mfsdram(SDRAM0_MCSTS, val);
	} while (!(val & 0x80000000));
}

/*
 * phys_size_t initdram(int board_type)
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
phys_size_t initdram(int board_type)
{
	/*
	 * Soft-reset SDRAM controller.
	 */
	mtsdr(SDR0_SRST, SDR0_SRST_DMC);
	mtsdr(SDR0_SRST, 0x00000000);

	/*
	 * Disable memory controller.
	 */
	mtsdram(SDRAM0_CFG0, 0x00000000);

	/*
	 * Setup some default
	 */
	mtsdram(SDRAM0_UABBA, 0x00000000); /* ubba=0 (default)		*/
	mtsdram(SDRAM0_SLIO, 0x00000000);	/* rdre=0 wrre=0 rarw=0		*/
	mtsdram(SDRAM0_DEVOPT, 0x00000000); /* dll=0 ds=0 (normal)		*/
	mtsdram(SDRAM0_WDDCTR, 0x00000000); /* wrcp=0 dcd=0		*/
	mtsdram(SDRAM0_CLKTR, 0x40000000); /* clkp=1 (90 deg wr) dcdt=0	*/

	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram(SDRAM0_B0CR, 0x00082001);
	mtsdram(SDRAM0_TR0, 0x41094012);
	mtsdram(SDRAM0_TR1, 0x8080083d);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x00*/
	mtsdram(SDRAM0_RTR, 0x04100000);	/* Interval 7.8µs @ 133MHz PLB	*/
	mtsdram(SDRAM0_CFG1, 0x00000000);	/* Self-refresh exit, disable PM*/

	/*
	 * Enable the controller, then wait for DCEN to complete
	 */
	mtsdram(SDRAM0_CFG0, 0x80000000); /* DCEN=1, PMUD=0*/
	wait_init_complete();

	return CONFIG_SYS_MBYTES_SDRAM << 20;
}
