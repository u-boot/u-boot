/* I2cCore.c - MPC8220 PPC I2C Library */

/* Copyright 2004      Freescale Semiconductor, Inc. */

/*
modification history
--------------------
01c,29jun04,tcl	 1.3	removed CR. Added two bytes offset support.
01b,19jan04,tcl	 1.2	removed i2cMsDelay and sysDecGet. renamed i2cMsDelay
			back to sysMsDelay
01a,19jan04,tcl	 1.1	created and seperated from i2c.c
*/

/*
DESCRIPTION
This file contain I2C low level handling library functions
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vxWorks.h>
#include <sysLib.h>
#include <iosLib.h>
#include <logLib.h>
#include <tickLib.h>

/* BSP Includes */
#include "config.h"
#include "mpc8220.h"
#include "i2cCore.h"

#ifdef DEBUG_I2CCORE
int I2CCDbg = 0;
#endif

#define ABS(x)	((x < 0)? -x : x)

char *I2CERR[16] = {
	"Transfer in Progress\n",	/* 0 */
	"Transfer complete\n",
	"Not Addressed\n",		/* 2 */
	"Addressed as a slave\n",
	"Bus is Idle\n",		/* 4 */
	"Bus is busy\n",
	"Arbitration Lost\n",		/* 6 */
	"Arbitration on Track\n",
	"Slave receive, master writing to slave\n",	/* 8 */
	"Slave transmit, master reading from slave\n",
	"Interrupt is pending\n",	/* 10 */
	"Interrupt complete\n",
	"Acknowledge received\n",	/* 12 */
	"No acknowledge received\n",
	"Unknown status\n",		/* 14 */
	"\n"
};

/******************************************************************************
 *
 * chk_status - Check I2C status bit
 *
 * RETURNS: OK, or ERROR if the bit encounter
 *
 */

STATUS chk_status (PSI2C pi2c, UINT8 sta_bit, UINT8 truefalse)
{
	int i, status = 0;

	for (i = 0; i < I2C_POLL_COUNT; i++) {
		if ((pi2c->sr & sta_bit) == (truefalse ? sta_bit : 0))
			return (OK);
	}

	I2CCDBG (L2, ("--- sr %x stabit %x truefalse %d\n",
		      pi2c->sr, sta_bit, truefalse, 0, 0, 0));

	if (i == I2C_POLL_COUNT) {
		switch (sta_bit) {
		case I2C_STA_CF:
			status = 0;
			break;
		case I2C_STA_AAS:
			status = 2;
			break;
		case I2C_STA_BB:
			status = 4;
			break;
		case I2C_STA_AL:
			status = 6;
			break;
		case I2C_STA_SRW:
			status = 8;
			break;
		case I2C_STA_IF:
			status = 10;
			break;
		case I2C_STA_RXAK:
			status = 12;
			break;
		default:
			status = 14;
			break;
		}

		if (!truefalse)
			status++;

		I2CCDBG (NO, ("--- status %d\n", status, 0, 0, 0, 0, 0));
		I2CCDBG (NO, (I2CERR[status], 0, 0, 0, 0, 0, 0));
	}

	return (ERROR);
}

/******************************************************************************
 *
 * I2C Enable - Enable the I2C Controller
 *
 */
STATUS i2c_enable (SI2C * pi2c, PI2CSET pi2cSet)
{
	int fdr = pi2cSet->bit_rate;
	UINT8 adr = pi2cSet->i2c_adr;

	I2CCDBG (L2, ("i2c_enable fdr %d adr %x\n", fdr, adr, 0, 0, 0, 0));

	i2c_clear (pi2c);	/* Clear FDR, ADR, SR and CR reg */

	SetI2cFDR (pi2c, fdr);	/* Frequency			*/
	pi2c->adr = adr;

	pi2c->cr = I2C_CTL_EN;	/* Set Enable			*/

	/*
	   The I2C bus should be in Idle state. If the bus is busy,
	   clear the STA bit in control register
	 */
	if (chk_status (pi2c, I2C_STA_BB, 0) != OK) {
		if ((pi2c->cr & I2C_CTL_STA) == I2C_CTL_STA)
			pi2c->cr &= ~I2C_CTL_STA;

		/* Check again if it is still busy, return error if found */
		if (chk_status (pi2c, I2C_STA_BB, 1) == OK)
			return ERROR;
	}

	return (OK);
}

/******************************************************************************
 *
 * I2C Disable - Disable the I2C Controller
 *
 */
STATUS i2c_disable (PSI2C pi2c)
{
	i2c_clear (pi2c);

	pi2c->cr &= I2C_CTL_EN; /* Disable I2c			*/

	if ((pi2c->cr & I2C_CTL_STA) == I2C_CTL_STA)
		pi2c->cr &= ~I2C_CTL_STA;

	if (chk_status (pi2c, I2C_STA_BB, 0) != OK)
		return ERROR;

	return (OK);
}

/******************************************************************************
 *
 * I2C Clear - Clear the I2C Controller
 *
 */
STATUS i2c_clear (PSI2C pi2c)
{
	pi2c->adr = 0;
	pi2c->fdr = 0;
	pi2c->cr = 0;
	pi2c->sr = 0;

	return (OK);
}


STATUS i2c_start (PSI2C pi2c, PI2CSET pi2cSet)
{
#ifdef TWOBYTES
	UINT16 ByteOffset = pi2cSet->str_adr;
#else
	UINT8 ByteOffset = pi2cSet->str_adr;
#endif
#if 1
	UINT8 tmp = 0;
#endif
	UINT8 Addr = pi2cSet->slv_adr;

	pi2c->cr |= I2C_CTL_STA;	/* Generate start signal	*/

	if (chk_status (pi2c, I2C_STA_BB, 1) != OK)
		return ERROR;

	/* Write slave address */
	if (i2c_writebyte (pi2c, &Addr) != OK) {
		i2c_stop (pi2c);	/* Disable I2c			*/
		return ERROR;
	}
#ifdef TWOBYTES
#   if 0
	/* Issue the offset to start */
	if (i2c_write2byte (pi2c, &ByteOffset) != OK) {
		i2c_stop (pi2c);	/* Disable I2c			*/
		return ERROR;
	}
#endif
	tmp = (ByteOffset >> 8) & 0xff;
	if (i2c_writebyte (pi2c, &tmp) != OK) {
		i2c_stop (pi2c);	/* Disable I2c			*/
		return ERROR;
	}
	tmp = ByteOffset & 0xff;
	if (i2c_writebyte (pi2c, &tmp) != OK) {
		i2c_stop (pi2c);	/* Disable I2c			*/
		return ERROR;
	}
#else
	if (i2c_writebyte (pi2c, &ByteOffset) != OK) {
		i2c_stop (pi2c);	/* Disable I2c			*/
		return ERROR;
	}
#endif

	return (OK);
}

STATUS i2c_stop (PSI2C pi2c)
{
	pi2c->cr &= ~I2C_CTL_STA;	/* Generate stop signal		*/
	if (chk_status (pi2c, I2C_STA_BB, 0) != OK)
		return ERROR;

	return (OK);
}

/******************************************************************************
 *
 * Read Len bytes to the location pointed to by *Data from the device
 * with address Addr.
 */
int i2c_readblock (SI2C * pi2c, PI2CSET pi2cSet, UINT8 * Data)
{
	int i = 0;
	UINT8 Tmp;

/*    UINT8 ByteOffset = pi2cSet->str_adr; not used? */
	UINT8 Addr = pi2cSet->slv_adr;
	int Length = pi2cSet->xfer_size;

	I2CCDBG (L1, ("i2c_readblock addr %x data 0x%08x len %d offset %d\n",
		      Addr, (int) Data, Length, ByteOffset, 0, 0));

	if (pi2c->sr & I2C_STA_AL) {	/* Check if Arbitration lost	*/
		I2CCDBG (FN, ("Arbitration lost\n", 0, 0, 0, 0, 0, 0));
		pi2c->sr &= ~I2C_STA_AL;	/* Clear Arbitration status bit */
		return ERROR;
	}

	pi2c->cr |= I2C_CTL_TX; /* Enable the I2c for TX, Ack	*/

	if (i2c_start (pi2c, pi2cSet) == ERROR)
		return ERROR;

	pi2c->cr |= I2C_CTL_RSTA;	/* Repeat Start */

	Tmp = Addr | 1;

	if (i2c_writebyte (pi2c, &Tmp) != OK) {
		i2c_stop (pi2c);	/* Disable I2c	*/
		return ERROR;
	}

	if (((pi2c->sr & 0x07) == 0x07) || (pi2c->sr & 0x01))
		return ERROR;

	pi2c->cr &= ~I2C_CTL_TX;	/* Set receive mode	*/

	if (((pi2c->sr & 0x07) == 0x07) || (pi2c->sr & 0x01))
		return ERROR;

	/* Dummy Read */
	if (i2c_readbyte (pi2c, &Tmp, &i) != OK) {
		i2c_stop (pi2c);	/* Disable I2c	*/
		return ERROR;
	}

	i = 0;
	while (Length) {
		if (Length == 2)
			pi2c->cr |= I2C_CTL_TXAK;

		if (Length == 1)
			pi2c->cr &= ~I2C_CTL_STA;

		if (i2c_readbyte (pi2c, Data, &Length) != OK) {
			return i2c_stop (pi2c);
		}
		i++;
		Length--;
		Data++;
	}

	if (i2c_stop (pi2c) == ERROR)
		return ERROR;

	return i;
}

STATUS i2c_writeblock (SI2C * pi2c, PI2CSET pi2cSet, UINT8 * Data)
{
	int Length = pi2cSet->xfer_size;

#ifdef TWOBYTES
	UINT16 ByteOffset = pi2cSet->str_adr;
#else
	UINT8 ByteOffset = pi2cSet->str_adr;
#endif
	int j, k;

	I2CCDBG (L2, ("i2c_writeblock\n", 0, 0, 0, 0, 0, 0));

	if (pi2c->sr & I2C_STA_AL) {
		/* Check if arbitration lost */
		I2CCDBG (L2, ("Arbitration lost\n", 0, 0, 0, 0, 0, 0));
		pi2c->sr &= ~I2C_STA_AL;	/* Clear the condition	*/
		return ERROR;
	}

	pi2c->cr |= I2C_CTL_TX; /* Enable the I2c for TX, Ack	*/

	/* Do the not even offset first */
	if ((ByteOffset % 8) != 0) {
		int remain;

		if (Length > 8) {
			remain = 8 - (ByteOffset % 8);
			Length -= remain;

			pi2cSet->str_adr = ByteOffset;

			if (i2c_start (pi2c, pi2cSet) == ERROR)
				return ERROR;

			for (j = ByteOffset; j < remain; j++) {
				if (i2c_writebyte (pi2c, Data++) != OK)
					return ERROR;
			}

			if (i2c_stop (pi2c) == ERROR)
				return ERROR;

			sysMsDelay (32);

			/* Update the new ByteOffset */
			ByteOffset += remain;
		}
	}

	for (j = ByteOffset, k = 0; j < (Length + ByteOffset); j++) {
		if ((j % 8) == 0) {
			pi2cSet->str_adr = j;
			if (i2c_start (pi2c, pi2cSet) == ERROR)
				return ERROR;
		}

		k++;

		if (i2c_writebyte (pi2c, Data++) != OK)
			return ERROR;

		if ((j == (Length - 1)) || ((k % 8) == 0)) {
			if (i2c_stop (pi2c) == ERROR)
				return ERROR;

			sysMsDelay (50);
		}

	}

	return k;
}

STATUS i2c_readbyte (SI2C * pi2c, UINT8 * readb, int *index)
{
	pi2c->sr &= ~I2C_STA_IF;	/* Clear Interrupt Bit	*/
	*readb = pi2c->dr;		/* Read a byte		*/

	/*
	   Set I2C_CTRL_TXAK will cause Transfer pending and
	   set I2C_CTRL_STA will cause Interrupt pending
	 */
	if (*index != 2) {
		if (chk_status (pi2c, I2C_STA_CF, 1) != OK)	/* Transfer not complete?	*/
			return ERROR;
	}

	if (*index != 1) {
		if (chk_status (pi2c, I2C_STA_IF, 1) != OK)
			return ERROR;
	}

	return (OK);
}


STATUS i2c_writebyte (SI2C * pi2c, UINT8 * writeb)
{
	pi2c->sr &= ~I2C_STA_IF;	/* Clear Interrupt	*/
	pi2c->dr = *writeb;		/* Write a byte		*/

	if (chk_status (pi2c, I2C_STA_CF, 1) != OK)	/* Transfer not complete?	*/
		return ERROR;

	if (chk_status (pi2c, I2C_STA_IF, 1) != OK)
		return ERROR;

	return OK;
}

STATUS i2c_write2byte (SI2C * pi2c, UINT16 * writeb)
{
	UINT8 data;

	data = (UINT8) ((*writeb >> 8) & 0xff);
	if (i2c_writebyte (pi2c, &data) != OK)
		return ERROR;
	data = (UINT8) (*writeb & 0xff);
	if (i2c_writebyte (pi2c, &data) != OK)
		return ERROR;
	return OK;
}

/* FDR table base on 33Mhz - more detail please refer to Odini2c_dividers.xls
FDR FDR scl sda scl2tap2
510 432 tap tap tap tap scl_per	    sda_hold	I2C Freq    0	1   2	3   4	5
000 000 9   3	4   1	28 Clocks   9 Clocks	1190 KHz    0	0   0	0   0	0
000 001 9   3	4   2	44 Clocks   11 Clocks	758 KHz	    0	0   1	0   0	0
000 010 9   3	6   4	80 Clocks   17 Clocks	417 KHz	    0	0   0	1   0	0
000 011 9   3	6   8	144 Clocks  25 Clocks	231 KHz	    0	0   1	1   0	0
000 100 9   3	14  16	288 Clocks  49 Clocks	116 KHz	    0	0   0	0   1	0
000 101 9   3	30  32	576 Clocks  97 Clocks	58 KHz	    0	0   1	0   1	0
000 110 9   3	62  64	1152 Clocks 193 Clocks	29 KHz	    0	0   0	1   1	0
000 111 9   3	126 128 2304 Clocks 385 Clocks	14 KHz	    0	0   1	1   1	0
001 000 10  3	4   1	30 Clocks   9 Clocks	1111 KHz1   0	0   0	0   0
001 001 10  3	4   2	48 Clocks   11 Clocks	694 KHz	    1	0   1	0   0	0
001 010 10  3	6   4	88 Clocks   17 Clocks	379 KHz	    1	0   0	1   0	0
001 011 10  3	6   8	160 Clocks  25 Clocks	208 KHz	    1	0   1	1   0	0
001 100 10  3	14  16	320 Clocks  49 Clocks	104 KHz	    1	0   0	0   1	0
001 101 10  3	30  32	640 Clocks  97 Clocks	52 KHz	    1	0   1	0   1	0
001 110 10  3	62  64	1280 Clocks 193 Clocks	26 KHz	    1	0   0	1   1	0
001 111 10  3	126 128 2560 Clocks 385 Clocks	13 KHz	    1	0   1	1   1	0
010 000 12  4	4   1	34 Clocks   10 Clocks	980 KHz	    0	1   0	0   0	0
010 001 12  4	4   2	56 Clocks   13 Clocks	595 KHz	    0	1   1	0   0	0
010 010 12  4	6   4	104 Clocks  21 Clocks	321 KHz	    0	1   0	1   0	0
010 011 12  4	6   8	192 Clocks  33 Clocks	174 KHz	    0	1   1	1   0	0
010 100 12  4	14  16	384 Clocks  65 Clocks	87 KHz	    0	1   0	0   1	0
010 101 12  4	30  32	768 Clocks  129 Clocks	43 KHz	    0	1   1	0   1	0
010 110 12  4	62  64	1536 Clocks 257 Clocks	22 KHz	    0	1   0	1   1	0
010 111 12  4	126 128 3072 Clocks 513 Clocks	11 KHz	    0	1   1	1   1	0
011 000 15  4	4   1	40 Clocks   10 Clocks	833 KHz	    1	1   0	0   0	0
011 001 15  4	4   2	68 Clocks   13 Clocks	490 KHz	    1	1   1	0   0	0
011 010 15  4	6   4	128 Clocks  21 Clocks	260 KHz	    1	1   0	1   0	0
011 011 15  4	6   8	240 Clocks  33 Clocks	139 KHz	    1	1   1	1   0	0
011 100 15  4	14  16	480 Clocks  65 Clocks	69 KHz	    1	1   0	0   1	0
011 101 15  4	30  32	960 Clocks  129 Clocks	35 KHz	    1	1   1	0   1	0
011 110 15  4	62  64	1920 Clocks 257 Clocks	17 KHz	    1	1   0	1   1	0
011 111 15  4	126 128 3840 Clocks 513 Clocks	9 KHz	    1	1   1	1   1	0
100 000 5   1	4   1	20 Clocks   7 Clocks	1667 KHz    0	0   0	0   0	1
100 001 5   1	4   2	28 Clocks   7 Clocks	1190 KHz    0	0   1	0   0	1
100 010 5   1	6   4	48 Clocks   9 Clocks	694 KHz	    0	0   0	1   0	1
100 011 5   1	6   8	80 Clocks   9 Clocks	417 KHz	    0	0   1	1   0	1
100 100 5   1	14  16	160 Clocks  17 Clocks	208 KHz	    0	0   0	0   1	1
100 101 5   1	30  32	320 Clocks  33 Clocks	104 KHz	    0	0   1	0   1	1
100 110 5   1	62  64	640 Clocks  65 Clocks	52 KHz	    0	0   0	1   1	1
100 111 5   1	126 128 1280 Clocks 129 Clocks	26 KHz	    0	0   1	1   1	1
101 000 6   1	4   1	22 Clocks   7 Clocks	1515 KHz    1	0   0	0   0	1
101 001 6   1	4   2	32 Clocks   7 Clocks	1042 KHz    1	0   1	0   0	1
101 010 6   1	6   4	56 Clocks   9 Clocks	595 KHz	    1	0   0	1   0	1
101 011 6   1	6   8	96 Clocks   9 Clocks	347 KHz	    1	0   1	1   0	1
101 100 6   1	14  16	192 Clocks  17 Clocks	174 KHz	    1	0   0	0   1	1
101 101 6   1	30  32	384 Clocks  33 Clocks	87 KHz	    1	0   1	0   1	1
101 110 6   1	62  64	768 Clocks  65 Clocks	43 KHz	    1	0   0	1   1	1
101 111 6   1	126 128 1536 Clocks 129 Clocks	22 KHz	    1	0   1	1   1	1
110 000 7   2	4   1	24 Clocks   8 Clocks	1389 KHz    0	1   0	0   0	1
110 001 7   2	4   2	36 Clocks   9 Clocks	926 KHz	    0	1   1	0   0	1
110 010 7   2	6   4	64 Clocks   13 Clocks	521 KHz	    0	1   0	1   0	1
110 011 7   2	6   8	112 Clocks  17 Clocks	298 KHz	    0	1   1	1   0	1
110 100 7   2	14  16	224 Clocks  33 Clocks	149 KHz	    0	1   0	0   1	1
110 101 7   2	30  32	448 Clocks  65 Clocks	74 KHz	    0	1   1	0   1	1
110 110 7   2	62  64	896 Clocks  129 Clocks	37 KHz	    0	1   0	1   1	1
110 111 7   2	126 128 1792 Clocks 257 Clocks	19 KHz	    0	1   1	1   1	1
111 000 8   2	4   1	26 Clocks   8 Clocks	1282 KHz    1	1   0	0   0	1
111 001 8   2	4   2	40 Clocks   9 Clocks	833 KHz	    1	1   1	0   0	1
111 010 8   2	6   4	72 Clocks   13 Clocks	463 KHz	    1	1   0	1   0	1
111 011 8   2	6   8	128 Clocks  17 Clocks	260 KHz	    1	1   1	1   0	1
111 100 8   2	14  16	256 Clocks  33 Clocks	130 KHz	    1	1   0	0   1	1
111 101 8   2	30  32	512 Clocks  65 Clocks	65 KHz	    1	1   1	0   1	1
111 110 8   2	62  64	1024 Clocks 129 Clocks	33 KHz	    1	1   0	1   1	1
111 111 8   2	126 128 2048 Clocks 257 Clocks	16 KHz	    1	1   1	1   1	1
*/
STATUS SetI2cFDR (PSI2C pi2cRegs, int bitrate)
{
/* Constants */
	const UINT8 div_hold[8][3] = { {9, 3}, {10, 3},
	{12, 4}, {15, 4},
	{5, 1}, {6, 1},
	{7, 2}, {8, 2}
	};

	const UINT8 scl_tap[8][2] = { {4, 1}, {4, 2},
	{6, 4}, {6, 8},
	{14, 16}, {30, 32},
	{62, 64}, {126, 128}
	};

	UINT8 mfdr_bits;

	int i = 0;
	int j = 0;

	int Diff, min;
	int WhichFreq, iRec, jRec;
	int SCL_Period;
	int SCL_Hold;
	int I2C_Freq;

	I2CCDBG (L2, ("Entering getBitRate: bitrate %d pi2cRegs 0x%08x\n",
		      bitrate, (int) pi2cRegs, 0, 0, 0, 0));

	if (bitrate < 0) {
		I2CCDBG (NO, ("Invalid bitrate\n", 0, 0, 0, 0, 0, 0));
		return ERROR;
	}

	/* Initialize */
	mfdr_bits = 0;
	min = 0x7fffffff;
	WhichFreq = iRec = jRec = 0;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			/* SCL Period = 2 * (scl2tap + [(SCL_Tap - 1) * tap2tap] + 2)
			 * SCL Hold   = scl2tap + ((SDA_Tap - 1) * tap2tap) + 3
			 * Bit Rate (I2C Freq) = System Freq / SCL Period
			 */
			SCL_Period =
				2 * (scl_tap[i][0] +
				     ((div_hold[j][0] - 1) * scl_tap[i][1]) +
				     2);

			/* Now get the I2C Freq */
			I2C_Freq = DEV_CLOCK_FREQ / SCL_Period;

			/* Take equal or slower */
			if (I2C_Freq > bitrate)
				continue;

			/* Take the differences */
			Diff = I2C_Freq - bitrate;

			Diff = ABS (Diff);

			/* Find the closer value */
			if (Diff < min) {
				min = Diff;
				WhichFreq = I2C_Freq;
				iRec = i;
				jRec = j;
			}

			I2CCDBG (L2,
				 ("--- (%d,%d) I2C_Freq %d minDiff %d min %d\n",
				  i, j, I2C_Freq, Diff, min, 0));
		}
	}

	SCL_Period =
		2 * (scl_tap[iRec][0] +
		     ((div_hold[jRec][0] - 1) * scl_tap[iRec][1]) + 2);

	I2CCDBG (L2, ("\nmin %d WhichFreq %d iRec %d jRec %d\n",
		      min, WhichFreq, iRec, jRec, 0, 0));
	I2CCDBG (L2, ("--- scl2tap %d SCL_Tap %d tap2tap %d\n",
		      scl_tap[iRec][0], div_hold[jRec][0], scl_tap[iRec][1],
		      0, 0, 0));

	/* This may no require */
	SCL_Hold =
		scl_tap[iRec][0] +
		((div_hold[jRec][1] - 1) * scl_tap[iRec][1]) + 3;
	I2CCDBG (L2,
		 ("--- SCL_Period %d SCL_Hold %d\n", SCL_Period, SCL_Hold, 0,
		  0, 0, 0));

	I2CCDBG (L2, ("--- mfdr_bits %x\n", mfdr_bits, 0, 0, 0, 0, 0));

	/* FDR 4,3,2 */
	if ((iRec & 1) == 1)
		mfdr_bits |= 0x04;	/* FDR 2 */
	if ((iRec & 2) == 2)
		mfdr_bits |= 0x08;	/* FDR 3 */
	if ((iRec & 4) == 4)
		mfdr_bits |= 0x10;	/* FDR 4 */
	/* FDR 5,1,0 */
	if ((jRec & 1) == 1)
		mfdr_bits |= 0x01;	/* FDR 0 */
	if ((jRec & 2) == 2)
		mfdr_bits |= 0x02;	/* FDR 1 */
	if ((jRec & 4) == 4)
		mfdr_bits |= 0x20;	/* FDR 5 */

	I2CCDBG (L2, ("--- mfdr_bits %x\n", mfdr_bits, 0, 0, 0, 0, 0));

	pi2cRegs->fdr = mfdr_bits;

	return OK;
}
