/*
 * (C) Copyright 2004, Freescale, Inc
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com
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

/*
DESCRIPTION
Read Dram spd and base on its information to calculate the memory size,
characteristics to initialize the dram on MPC8220
*/

#include <common.h>
#include <mpc8220.h>
#include "i2cCore.h"
#include "dramSetup.h"

DECLARE_GLOBAL_DATA_PTR;

#define SPD_SIZE	CFG_SDRAM_SPD_SIZE
#define DRAM_SPD	(CFG_SDRAM_SPD_I2C_ADDR)<<1	/* on Board SPD eeprom */
#define TOTAL_BANK	CFG_SDRAM_TOTAL_BANKS

int spd_status (volatile i2c8220_t * pi2c, u8 sta_bit, u8 truefalse)
{
	int i;

	for (i = 0; i < I2C_POLL_COUNT; i++) {
		if ((pi2c->sr & sta_bit) == (truefalse ? sta_bit : 0))
			return (OK);
	}

	return (ERROR);
}

int spd_clear (volatile i2c8220_t * pi2c)
{
	pi2c->adr = 0;
	pi2c->fdr = 0;
	pi2c->cr = 0;
	pi2c->sr = 0;

	return (OK);
}

int spd_stop (volatile i2c8220_t * pi2c)
{
	pi2c->cr &= ~I2C_CTL_STA;	/* Generate stop signal         */
	if (spd_status (pi2c, I2C_STA_BB, 0) != OK)
		return ERROR;

	return (OK);
}

int spd_readbyte (volatile i2c8220_t * pi2c, u8 * readb, int *index)
{
	pi2c->sr &= ~I2C_STA_IF;	/* Clear Interrupt Bit          */
	*readb = pi2c->dr;	/* Read a byte                  */

	/*
	   Set I2C_CTRL_TXAK will cause Transfer pending and
	   set I2C_CTRL_STA will cause Interrupt pending
	 */
	if (*index != 2) {
		if (spd_status (pi2c, I2C_STA_CF, 1) != OK)	/* Transfer not complete?       */
			return ERROR;
	}

	if (*index != 1) {
		if (spd_status (pi2c, I2C_STA_IF, 1) != OK)
			return ERROR;
	}

	return (OK);
}

int readSpdData (u8 * spdData)
{
	volatile i2c8220_t *pi2cReg;
	volatile pcfg8220_t *pcfg;
	u8 slvAdr = DRAM_SPD;
	u8 Tmp;
	int Length = SPD_SIZE;
	int i = 0;

	/* Enable Port Configuration for SDA and SDL signals */
	pcfg = (volatile pcfg8220_t *) (MMAP_PCFG);
	__asm__ ("sync");
	pcfg->pcfg3 &= ~CFG_I2C_PORT3_CONFIG;
	__asm__ ("sync");

	/* Points the structure to I2c mbar memory offset */
	pi2cReg = (volatile i2c8220_t *) (MMAP_I2C);


	/* Clear FDR, ADR, SR and CR reg */
	pi2cReg->adr = 0;
	pi2cReg->fdr = 0;
	pi2cReg->cr = 0;
	pi2cReg->sr = 0;

	/* Set for fix XLB Bus Frequency */
	switch (gd->bus_clk) {
	case 60000000:
		pi2cReg->fdr = 0x15;
		break;
	case 70000000:
		pi2cReg->fdr = 0x16;
		break;
	case 80000000:
		pi2cReg->fdr = 0x3a;
		break;
	case 90000000:
		pi2cReg->fdr = 0x17;
		break;
	case 100000000:
		pi2cReg->fdr = 0x3b;
		break;
	case 110000000:
		pi2cReg->fdr = 0x18;
		break;
	case 120000000:
		pi2cReg->fdr = 0x19;
		break;
	case 130000000:
		pi2cReg->fdr = 0x1a;
		break;
	}

	pi2cReg->adr = CFG_I2C_SLAVE<<1;

	pi2cReg->cr = I2C_CTL_EN;	/* Set Enable         */

	/*
	   The I2C bus should be in Idle state. If the bus is busy,
	   clear the STA bit in control register
	 */
	if (spd_status (pi2cReg, I2C_STA_BB, 0) != OK) {
		if ((pi2cReg->cr & I2C_CTL_STA) == I2C_CTL_STA)
			pi2cReg->cr &= ~I2C_CTL_STA;

		/* Check again if it is still busy, return error if found */
		if (spd_status (pi2cReg, I2C_STA_BB, 1) == OK)
			return ERROR;
	}

	pi2cReg->cr |= I2C_CTL_TX;	/* Enable the I2c for TX, Ack   */
	pi2cReg->cr |= I2C_CTL_STA;	/* Generate start signal        */

	if (spd_status (pi2cReg, I2C_STA_BB, 1) != OK)
		return ERROR;


	/* Write slave address */
	pi2cReg->sr &= ~I2C_STA_IF;	/* Clear Interrupt              */
	pi2cReg->dr = slvAdr;	/* Write a byte                 */

	if (spd_status (pi2cReg, I2C_STA_CF, 1) != OK) {	/* Transfer not complete?       */
		spd_stop (pi2cReg);
		return ERROR;
	}

	if (spd_status (pi2cReg, I2C_STA_IF, 1) != OK) {
		spd_stop (pi2cReg);
		return ERROR;
	}


	/* Issue the offset to start */
	pi2cReg->sr &= ~I2C_STA_IF;	/* Clear Interrupt              */
	pi2cReg->dr = 0;	/* Write a byte                 */

	if (spd_status (pi2cReg, I2C_STA_CF, 1) != OK) {	/* Transfer not complete?       */
		spd_stop (pi2cReg);
		return ERROR;
	}

	if (spd_status (pi2cReg, I2C_STA_IF, 1) != OK) {
		spd_stop (pi2cReg);
		return ERROR;
	}


	/* Set repeat start */
	pi2cReg->cr |= I2C_CTL_RSTA;	/* Repeat Start                 */

	pi2cReg->sr &= ~I2C_STA_IF;	/* Clear Interrupt              */
	pi2cReg->dr = slvAdr | 1;	/* Write a byte                 */

	if (spd_status (pi2cReg, I2C_STA_CF, 1) != OK) {	/* Transfer not complete?       */
		spd_stop (pi2cReg);
		return ERROR;
	}

	if (spd_status (pi2cReg, I2C_STA_IF, 1) != OK) {
		spd_stop (pi2cReg);
		return ERROR;
	}

	if (((pi2cReg->sr & 0x07) == 0x07) || (pi2cReg->sr & 0x01))
		return ERROR;

	pi2cReg->cr &= ~I2C_CTL_TX;	/* Set receive mode             */

	if (((pi2cReg->sr & 0x07) == 0x07) || (pi2cReg->sr & 0x01))
		return ERROR;

	/* Dummy Read */
	if (spd_readbyte (pi2cReg, &Tmp, &i) != OK) {
		spd_stop (pi2cReg);
		return ERROR;
	}

	i = 0;
	while (Length) {
		if (Length == 2)
			pi2cReg->cr |= I2C_CTL_TXAK;

		if (Length == 1)
			pi2cReg->cr &= ~I2C_CTL_STA;

		if (spd_readbyte (pi2cReg, spdData, &Length) != OK) {
			return spd_stop (pi2cReg);
		}
		i++;
		Length--;
		spdData++;
	}

	/* Stop the service */
	spd_stop (pi2cReg);

	return OK;
}

int getBankInfo (int bank, draminfo_t * pBank)
{
	int status;
	int checksum;
	int count;
	u8 spdData[SPD_SIZE];


	if (bank > 2 || pBank == 0) {
		/* illegal values */
		return (-42);
	}

	status = readSpdData (&spdData[0]);
	if (status < 0)
		return (-1);

	/* check the checksum */
	for (count = 0, checksum = 0; count < LOC_CHECKSUM; count++)
		checksum += spdData[count];

	checksum = checksum - ((checksum / 256) * 256);

	if (checksum != spdData[LOC_CHECKSUM])
		return (-2);

	/* Get the memory type */
	if (!
	    ((spdData[LOC_TYPE] == TYPE_DDR)
	     || (spdData[LOC_TYPE] == TYPE_SDR)))
		/* not one of the types we support */
		return (-3);

	pBank->type = spdData[LOC_TYPE];

	/* Set logical banks */
	pBank->banks = spdData[LOC_LOGICAL_BANKS];

	/* Check that we have enough physical banks to cover the bank we are
	 * figuring out.  Odd-numbered banks correspond to the second bank
	 * on the device.
	 */
	if (bank & 1) {
		/* Second bank of a "device" */
		if (spdData[LOC_PHYS_BANKS] < 2)
			/* this bank doesn't exist on the "device" */
			return (-4);

		if (spdData[LOC_ROWS] & 0xf0)
			/* Two asymmetric banks */
			pBank->rows = spdData[LOC_ROWS] >> 4;
		else
			pBank->rows = spdData[LOC_ROWS];

		if (spdData[LOC_COLS] & 0xf0)
			/* Two asymmetric banks */
			pBank->cols = spdData[LOC_COLS] >> 4;
		else
			pBank->cols = spdData[LOC_COLS];
	} else {
		/* First bank of a "device" */
		pBank->rows = spdData[LOC_ROWS];
		pBank->cols = spdData[LOC_COLS];
	}

	pBank->width = spdData[LOC_WIDTH_HIGH] << 8 | spdData[LOC_WIDTH_LOW];
	pBank->bursts = spdData[LOC_BURSTS];
	pBank->CAS = spdData[LOC_CAS];
	pBank->CS = spdData[LOC_CS];
	pBank->WE = spdData[LOC_WE];
	pBank->Trp = spdData[LOC_Trp];
	pBank->Trcd = spdData[LOC_Trcd];
	pBank->buffered = spdData[LOC_Buffered] & 1;
	pBank->refresh = spdData[LOC_REFRESH];

	return (0);
}


/* checkMuxSetting -- given a row/column device geometry, return a mask
 *                    of the valid DRAM controller addr_mux settings for
 *                    that geometry.
 *
 *  Arguments:        u8 rows:     number of row addresses in this device
 *                    u8 columns:  number of column addresses in this device
 *
 *  Returns:          a mask of the allowed addr_mux settings for this
 *                    geometry.  Each bit in the mask represents a
 *                    possible addr_mux settings (for example, the
 *                    (1<<2) bit in the mask represents the 0b10 setting)/
 *
 */
u8 checkMuxSetting (u8 rows, u8 columns)
{
	muxdesc_t *pIdx, *pMux;
	u8 mask;
	int lrows, lcolumns;
	u32 mux[4] = { 0x00080c04, 0x01080d03, 0x02080e02, 0xffffffff };

	/* Setup MuxDescriptor in SRAM space */
	/* MUXDESC AddressRuns [] = {
	   { 0, 8, 12, 4 },         / setting, columns, rows, extra columns /
	   { 1, 8, 13, 3 },         / setting, columns, rows, extra columns /
	   { 2, 8, 14, 2 },         / setting, columns, rows, extra columns /
	   { 0xff }                 / list terminator /
	   }; */

	pIdx = (muxdesc_t *) & mux[0];

	/* Check rows x columns against each possible address mux setting */
	for (pMux = pIdx, mask = 0;; pMux++) {
		lrows = rows;
		lcolumns = columns;

		if (pMux->MuxValue == 0xff)
			break;	/* end of list */

		/* For a given mux setting, since we want all the memory in a
		 * device to be contiguous, we want the device "use up" the
		 * address lines such that there are no extra column or row
		 * address lines on the device.
		 */

		lcolumns -= pMux->Columns;
		if (lcolumns < 0)
			/* Not enough columns to get to the rows */
			continue;

		lrows -= pMux->Rows;
		if (lrows > 0)
			/* we have extra rows left -- can't do that! */
			continue;

		/* At this point, we either have to have used up all the
		 * rows or we have to have no columns left.
		 */

		if (lcolumns != 0 && lrows != 0)
			/* rows AND columns are left.  Bad! */
			continue;

		lcolumns -= pMux->MoreColumns;

		if (lcolumns <= 0)
			mask |= (1 << pMux->MuxValue);
	}

	return (mask);
}


u32 dramSetup (void)
{
	draminfo_t DramInfo[TOTAL_BANK];
	draminfo_t *pDramInfo;
	u32 size, temp, cfg_value, mode_value, refresh;
	u8 *ptr;
	u8 bursts, Trp, Trcd, type, buffered;
	u8 muxmask, rows, columns;
	int count, banknum;
	u32 *prefresh, *pIdx;
	u32 refrate[8] = { 15625, 3900, 7800, 31300,
		62500, 125000, 0xffffffff, 0xffffffff
	};
	volatile sysconf8220_t *sysconf;
	volatile memctl8220_t *memctl;

	sysconf = (volatile sysconf8220_t *) MMAP_MBAR;
	memctl = (volatile memctl8220_t *) MMAP_MEMCTL;

	/* Set everything in the descriptions to zero */
	ptr = (u8 *) & DramInfo[0];
	for (count = 0; count < sizeof (DramInfo); count++)
		*ptr++ = 0;

	for (banknum = 0; banknum < TOTAL_BANK; banknum++)
		sysconf->cscfg[banknum];

	/* Descriptions of row/column address muxing for various
	 * addr_mux settings.
	 */

	pIdx = prefresh = (u32 *) & refrate[0];

	/* Get all the info for all three logical banks */
	bursts = 0xff;
	Trp = 0;
	Trcd = 0;
	type = 0;
	buffered = 0xff;
	refresh = 0xffffffff;
	muxmask = 0xff;

	/* Two bank, CS0 and CS1 */
	for (banknum = 0, pDramInfo = &DramInfo[0];
	     banknum < TOTAL_BANK; banknum++, pDramInfo++) {
		pDramInfo->ordinal = banknum;	/* initial sorting */
		if (getBankInfo (banknum, pDramInfo) < 0)
			continue;

		/* get cumulative parameters of all three banks */
		if (type && pDramInfo->type != type)
			return 0;

		type = pDramInfo->type;
		rows = pDramInfo->rows;
		columns = pDramInfo->cols;

		/* This chip only supports 13 DRAM memory lines, but some devices
		 * have 14 rows.  To deal with this, ignore the 14th address line
		 * by limiting the number of rows (and columns) to 13.  This will
		 * mean that for 14-row devices we will only be able to use
		 * half of the memory, but it's better than nothing.
		 */
		if (rows > 13)
			rows = 13;
		if (columns > 13)
			columns = 13;

		pDramInfo->size =
			((1 << (rows + columns)) * pDramInfo->width);
		pDramInfo->size *= pDramInfo->banks;
		pDramInfo->size >>= 3;

		/* figure out which addr_mux configurations will support this device */
		muxmask &= checkMuxSetting (rows, columns);
		if (muxmask == 0)
			return 0;

		buffered = pDramInfo->buffered;
		bursts &= pDramInfo->bursts;	/* union of all bursts */
		if (pDramInfo->Trp > Trp)	/* worst case (longest) Trp */
			Trp = pDramInfo->Trp;

		if (pDramInfo->Trcd > Trcd)	/* worst case (longest) Trcd */
			Trcd = pDramInfo->Trcd;

		prefresh = pIdx;
		/* worst case (shortest) Refresh period */
		if (refresh > prefresh[pDramInfo->refresh & 7])
			refresh = prefresh[pDramInfo->refresh & 7];

	}			/* for loop */


	/* We only allow a burst length of 8! */
	if (!(bursts & 8))
		bursts = 8;

	/* Sort the devices.  In order to get each chip select region
	 * aligned properly, put the biggest device at the lowest address.
	 * A simple bubble sort will do the trick.
	 */
	for (banknum = 0, pDramInfo = &DramInfo[0];
	     banknum < TOTAL_BANK; banknum++, pDramInfo++) {
		int i;

		for (i = 0; i < TOTAL_BANK; i++) {
			if (pDramInfo->size < DramInfo[i].size &&
			    pDramInfo->ordinal < DramInfo[i].ordinal) {
				/* If the current bank is smaller, but if the ordinal is also
				 * smaller, swap the ordinals
				 */
				u8 temp8;

				temp8 = DramInfo[i].ordinal;
				DramInfo[i].ordinal = pDramInfo->ordinal;
				pDramInfo->ordinal = temp8;
			}
		}
	}


	/* Now figure out the base address for each bank.  While
	 * we're at it, figure out how much memory there is.
	 *
	 */
	size = 0;
	for (banknum = 0; banknum < TOTAL_BANK; banknum++) {
		int i;

		for (i = 0; i < TOTAL_BANK; i++) {
			if (DramInfo[i].ordinal == banknum
			    && DramInfo[i].size != 0) {
				DramInfo[i].base = size;
				size += DramInfo[i].size;
			}
		}
	}

	/* Set up the Drive Strength register */
	sysconf->sdramds = CFG_SDRAM_DRIVE_STRENGTH;

	/* ********************** Cfg 1 ************************* */

	/* Set the single read to read/write/precharge delay */
	cfg_value = CFG1_SRD2RWP ((type == TYPE_DDR) ? 7 : 0xb);

	/* Set the single write to read/write/precharge delay.
	 * This may or may not be correct.  The controller spec
	 * says "tWR", but "tWR" does not appear in the SPD.  It
	 * always seems to be 15nsec for the class of device we're
	 * using, which turns out to be 2 clock cycles at 133MHz,
	 * so that's what we're going to use.
	 *
	 * HOWEVER, because of a bug in the controller, for DDR
	 * we need to set this to be the same as the value
	 * calculated for bwt2rwp.
	 */
	cfg_value |= CFG1_SWT2RWP ((type == TYPE_DDR) ? 7 : 2);

	/* Set the Read CAS latency.  We're going to use a CL of
	 * 2.5 for DDR and 2 SDR.
	 */
	cfg_value |= CFG1_RLATENCY ((type == TYPE_DDR) ? 7 : 2);


	/* Set the Active to Read/Write delay.  This depends
	 * on Trcd which is reported as nanoseconds times 4.
	 * We want to calculate Trcd (in nanoseconds) times XLB clock (in Hz)
	 * which gives us a dimensionless quantity.  Play games with
	 * the divisions so we don't run out of dynamic ranges.
	 */
	/* account for megaherz and the times 4 */
	temp = (Trcd * (gd->bus_clk / 1000000)) / 4;

	/* account for nanoseconds and round up, with a minimum value of 2 */
	temp = ((temp + 999) / 1000) - 1;
	if (temp < 2)
		temp = 2;

	cfg_value |= CFG1_ACT2WR (temp);

	/* Set the precharge to active delay.  This depends
	 * on Trp which is reported as nanoseconds times 4.
	 * We want to calculate Trp (in nanoseconds) times XLB clock (in Hz)
	 * which gives us a dimensionless quantity.  Play games with
	 * the divisions so we don't run out of dynamic ranges.
	 */
	/* account for megaherz and the times 4 */
	temp = (Trp * (gd->bus_clk / 1000000)) / 4;

	/* account for nanoseconds and round up, then subtract 1, with a
	 * minumum value of 1 and a maximum value of 7.
	 */
	temp = (((temp + 999) / 1000) - 1) & 7;
	if (temp < 1)
		temp = 1;

	cfg_value |= CFG1_PRE2ACT (temp);

	/* Set refresh to active delay.  This depends
	 * on Trfc which is not reported in the SPD.
	 * We'll use a nominal value of 75nsec which is
	 * what the controller spec uses.
	 */
	temp = (75 * (gd->bus_clk / 1000000));
	/* account for nanoseconds and round up, then subtract 1 */
	cfg_value |= CFG1_REF2ACT (((temp + 999) / 1000) - 1);

	/* Set the write latency, using the values given in the controller spec */
	cfg_value |= CFG1_WLATENCY ((type == TYPE_DDR) ? 3 : 0);
	memctl->cfg1 = cfg_value;	/* cfg 1 */
	asm volatile ("sync");


	/* ********************** Cfg 2 ************************* */

	/* Set the burst read to read/precharge delay */
	cfg_value = CFG2_BRD2RP ((type == TYPE_DDR) ? 5 : 8);

	/* Set the burst write to read/precharge delay.  Semi-magic numbers
	 * based on the controller spec recommendations, assuming tWR is
	 * two clock cycles.
	 */
	cfg_value |= CFG2_BWT2RWP ((type == TYPE_DDR) ? 7 : 10);

	/* Set the Burst read to write delay.  Semi-magic numbers
	 * based on the DRAM controller documentation.
	 */
	cfg_value |= CFG2_BRD2WT ((type == TYPE_DDR) ? 7 : 0xb);

	/* Set the burst length -- must be 8!! Well, 7, actually, becuase
	 * it's burst lenght minus 1.
	 */
	cfg_value |= CFG2_BURSTLEN (7);
	memctl->cfg2 = cfg_value;	/* cfg 2 */
	asm volatile ("sync");


	/* ********************** mode ************************* */

	/* Set enable bit, CKE high/low bits, and the DDR/SDR mode bit,
	 * disable automatic refresh.
	 */
	cfg_value = CTL_MODE_ENABLE | CTL_CKE_HIGH |
		((type == TYPE_DDR) ? CTL_DDR_MODE : 0);

	/* Set the address mux based on whichever setting(s) is/are common
	 * to all the devices we have.  If there is more than one, choose
	 * one arbitrarily.
	 */
	if (muxmask & 0x4)
		cfg_value |= CTL_ADDRMUX (2);
	else if (muxmask & 0x2)
		cfg_value |= CTL_ADDRMUX (1);
	else
		cfg_value |= CTL_ADDRMUX (0);

	/* Set the refresh interval. */
	temp = ((refresh * (gd->bus_clk / 1000000)) / (1000 * 64)) - 1;
	cfg_value |= CTL_REFRESH_INTERVAL (temp);

	/* Set buffered/non-buffered memory */
	if (buffered)
		cfg_value |= CTL_BUFFERED;

	memctl->ctrl = cfg_value;	/* ctrl */
	asm volatile ("sync");

	if (type == TYPE_DDR) {
		/* issue precharge all */
		temp = cfg_value | CTL_PRECHARGE_CMD;
		memctl->ctrl = temp;	/* ctrl */
		asm volatile ("sync");
	}


	/* Set up mode value for CAS latency */
#if (CFG_SDRAM_CAS_LATENCY==5) /* CL=2.5 */
	mode_value = (MODE_MODE | MODE_BURSTLEN (MODE_BURSTLEN_8) |
		MODE_BT_SEQUENTIAL | MODE_CL (MODE_CL_2p5) | MODE_CMD);
#else
	mode_value = (MODE_MODE | MODE_BURSTLEN (MODE_BURSTLEN_8) |
		      MODE_BT_SEQUENTIAL | MODE_CL (MODE_CL_2) | MODE_CMD);
#endif
	asm volatile ("sync");

	/* Write Extended Mode  - enable DLL */
	if (type == TYPE_DDR) {
		temp = MODE_EXTENDED | MODE_X_DLL_ENABLE |
			MODE_X_DS_NORMAL | MODE_CMD;
		memctl->mode = (temp >> 16);	/* mode */
		asm volatile ("sync");

		/* Write Mode - reset DLL, set CAS latency */
		temp = mode_value | MODE_OPMODE (MODE_OPMODE_RESETDLL);
		memctl->mode = (temp >> 16);	/* mode */
		asm volatile ("sync");
	}

	/* Program the chip selects. */
	for (banknum = 0; banknum < TOTAL_BANK; banknum++) {
		if (DramInfo[banknum].size != 0) {
			u32 mask;
			int i;

			for (i = 0, mask = 1; i < 32; mask <<= 1, i++) {
				if (DramInfo[banknum].size & mask)
					break;
			}
			temp = (DramInfo[banknum].base & 0xfff00000) | (i -
									1);

			sysconf->cscfg[banknum] = temp;
			asm volatile ("sync");
		}
	}

	/* Wait for DLL lock */
	udelay (200);

	temp = cfg_value | CTL_PRECHARGE_CMD;	/* issue precharge all */
	memctl->ctrl = temp;	/* ctrl */
	asm volatile ("sync");

	temp = cfg_value | CTL_REFRESH_CMD;	/* issue precharge all */
	memctl->ctrl = temp;	/* ctrl */
	asm volatile ("sync");

	memctl->ctrl = temp;	/* ctrl */
	asm volatile ("sync");

	/* Write Mode - DLL normal */
	temp = mode_value | MODE_OPMODE (MODE_OPMODE_NORMAL);
	memctl->mode = (temp >> 16);	/* mode */
	asm volatile ("sync");

	/* Enable refresh, enable DQS's (if DDR), and lock the control register */
	cfg_value &= ~CTL_MODE_ENABLE;	/* lock register */
	cfg_value |= CTL_REFRESH_ENABLE;	/* enable refresh */

	if (type == TYPE_DDR)
		cfg_value |= CTL_DQSOEN (0xf);	/* enable DQS's for DDR */

	memctl->ctrl = cfg_value;	/* ctrl */
	asm volatile ("sync");

	return size;
}
