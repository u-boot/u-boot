/******************************************************************************
 *
 * Name:	ski2c.c
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.57 $
 * Date:	$Date: 2003/01/28 09:17:38 $
 * Purpose:	Functions to access Voltage and Temperature Sensor
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2003 SysKonnect GmbH.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * History:
 *
 *	$Log: ski2c.c,v $
 *	Revision 1.57  2003/01/28 09:17:38  rschmidt
 *	Fixed handling for sensors on YUKON Fiber.
 *	Editorial changes.
 *	
 *	Revision 1.56  2002/12/19 14:20:41  rschmidt
 *	Added debugging code in SkI2cWait().
 *	Replaced all I2C-write operations with function SkI2cWrite().
 *	Fixed compiler warning because of uninitialized 'Time' in SkI2cEvent().
 *	Editorial changes.
 *	
 *	Revision 1.55  2002/10/15 07:23:55  rschmidt
 *	Added setting of the GIYukon32Bit bool variable to distinguish
 *	32-bit adapters.
 *	Editorial changes (TWSI).
 *	
 *	Revision 1.54  2002/08/13 09:05:06  rschmidt
 *	Added new thresholds if VAUX is not available (GIVauxAvail).
 *	Merged defines for PHY PLL 3V3 voltage (A and B).
 *	Editorial changes.
 *	
 *	Revision 1.53  2002/08/08 11:04:53  rwahl
 *	Added missing comment for revision 1.51
 *	
 *	Revision 1.52  2002/08/08 10:09:02  jschmalz
 *	Sensor init state caused wrong error log entry
 *	
 *	Revision 1.51  2002/08/06 09:43:03  jschmalz
 *	Extensions and changes for Yukon
 *	
 *	Revision 1.50  2002/08/02 12:09:22  rschmidt
 *	Added support for YUKON sensors.
 *	Editorial changes.
 *	
 *	Revision 1.49  2002/07/30 11:07:52  rschmidt
 *	Replaced MaxSens init by update for Copper in SkI2cInit1(),
 *	because it was already initialized in SkI2cInit0().
 *	Editorial changes.
 *	
 *	Revision 1.48  2001/08/16 12:44:33  afischer
 *	LM80 sensor init values corrected
 *	
 *	Revision 1.47  2001/04/05 11:38:09  rassmann
 *	Set SenState to idle in SkI2cWaitIrq().
 *	Changed error message in SkI2cWaitIrq().
 *	
 *	Revision 1.46  2001/04/02 14:03:35  rassmann
 *	Changed pAC to IoC in SK_IN32().
 *	
 *	Revision 1.45  2001/03/21 12:12:49  rassmann
 *	Resetting I2C_READY interrupt in SkI2cInit1().
 *	
 *	Revision 1.44  2000/08/07 15:49:03  gklug
 *	Fix: SK_INFAST only in NetWare driver.
 *	
 *	Revision 1.43  2000/08/03 14:28:17  rassmann
 *	Added function to wait for I2C being ready before resetting the board.
 *	Replaced one duplicate "out of range" message with correct one.
 *	
 *	Revision 1.42  1999/11/22 13:35:12  cgoos
 *	Changed license header to GPL.
 *	
 *	Revision 1.41  1999/09/14 14:11:30  malthoff
 *	The 1000BT Dual Link adapter has got only one Fan.
 *	The second Fan has been removed.
 *	
 *	Revision 1.40  1999/05/27 13:37:27  malthoff
 *	Set divisor of 1 for fan count calculation.
 *	
 *	Revision 1.39  1999/05/20 14:54:43  malthoff
 *	I2c.DummyReads is not used in Diagnostics.
 *	
 *	Revision 1.38  1999/05/20 09:20:56  cgoos
 *	Changes for 1000Base-T (up to 9 sensors and fans).
 *	
 *	Revision 1.37  1999/03/25 15:11:36  gklug
 *	fix: reset error flag if sensor reads correct value
 *	
 *	Revision 1.36  1999/01/07 14:11:16  gklug
 *	fix: break added
 *	
 *	Revision 1.35  1999/01/05 15:31:49  gklug
 *	fix: CLEAR STAT command is now added correctly
 *	
 *	Revision 1.34  1998/12/01 13:45:16  gklug
 *	fix: introduced Init level, because we don't need reinits
 *	
 *	Revision 1.33  1998/11/09 14:54:25  malthoff
 *	Modify I2C Transfer Timeout handling for Diagnostics.
 *	
 *	Revision 1.32  1998/11/03 06:54:35  gklug
 *	fix: Need dummy reads at the beginning to init sensors
 *
 *	Revision 1.31  1998/11/03 06:42:42  gklug
 *	fix: select correctVIO range only if between warning levels
 *	
 *	Revision 1.30  1998/11/02 07:36:53  gklug
 *	fix: Error should not include WARNING message
 *	
 *	Revision 1.29  1998/10/30 15:07:43  malthoff
 *	Disable 'I2C does not compelete' error log for diagnostics.
 *	
 *	Revision 1.28  1998/10/22 09:48:11  gklug
 *	fix: SysKonnectFileId typo
 *	
 *	Revision 1.27  1998/10/20 09:59:46  gklug
 *	add: parameter to SkOsGetTime
 *	
 *	Revision 1.26  1998/10/09 06:10:59  malthoff
 *	Remove ID_sccs by SysKonnectFileId.
 *	
 *	Revision 1.25  1998/09/08 12:40:26  gklug
 *	fix: syntax error in if clause
 *	
 *	Revision 1.24  1998/09/08 12:19:42  gklug
 *	chg: INIT Level checking
 *	
 *	Revision 1.23  1998/09/08 07:37:20  gklug
 *	fix: log error if PCI_IO voltage sensor could not be initialized
 *	
 *	Revision 1.22  1998/09/04 08:30:03  malthoff
 *	Bugfixes during SK_DIAG testing:
 *	- correct NS2BCLK() macro
 *	- correct SkI2cSndDev()
 *	- correct SkI2cWait() loop waiting for an event
 *	
 *	Revision 1.21  1998/08/27 14:46:01  gklug
 *	chg: if-then-else replaced by switch
 *
 *	Revision 1.20  1998/08/27 14:40:07  gklug
 *	test: integral types
 *	
 *	Revision 1.19  1998/08/25 07:51:54  gklug
 *	fix: typos for compiling
 *	
 *	Revision 1.18  1998/08/25 06:12:24  gklug
 *	add: count errors and warnings
 *	fix: check not the sensor state but the ErrFlag!
 *	
 *	Revision 1.17  1998/08/25 05:56:48  gklug
 *	add: CheckSensor function
 *	
 *	Revision 1.16  1998/08/20 11:41:10  gklug
 *	chg: omit STRCPY macro by using char * as Sensor Description
 *	
 *	Revision 1.15  1998/08/20 11:37:35  gklug
 *	chg: change Ioc to IoC
 *	
 *	Revision 1.14  1998/08/20 11:32:52  gklug
 *	fix: Para compile error
 *	
 *	Revision 1.13  1998/08/20 11:27:41  gklug
 *	fix: Compile bugs with new awrning constants
 *	
 *	Revision 1.12  1998/08/20 08:53:05  gklug
 *	fix: compiler errors
 *	add: Threshold values
 *	
 *	Revision 1.11  1998/08/19 12:39:22  malthoff
 *	Compiler Fix: Some names have changed.
 *	
 *	Revision 1.10  1998/08/19 12:20:56  gklug
 *	fix: remove struct from C files (see CCC)
 *	
 *	Revision 1.9  1998/08/19 06:28:46  malthoff
 *	SkOsGetTime returns SK_U64 now.
 *	
 *	Revision 1.8  1998/08/17 13:53:33  gklug
 *	fix: Parameter of event function and its result
 *	
 *	Revision 1.7  1998/08/17 07:02:15  malthoff
 *	Modify the functions for accessing the I2C SW Registers.
 *	Modify SkI2cWait().
 *	Put Lm80RcvReg into sklm80.c
 *	Remove Compiler Errors.
 *	
 *	Revision 1.6  1998/08/14 07:13:20  malthoff
 *	remove pAc with pAC
 *	remove smc with pAC
 *	change names to new convention
 *
 *	Revision 1.5  1998/08/14 06:24:49  gklug
 *	add: init level 1 and 2
 *
 *	Revision 1.4  1998/08/12 14:31:12  gklug
 *	add: error log for unknown event
 *
 *	Revision 1.3  1998/08/12 13:37:04  gklug
 *	add: Init 0 function
 *
 *	Revision 1.2  1998/08/11 07:27:15  gklug
 *	add: functions of the interface
 *	adapt rest of source to C coding Conventions
 *	rmv: unnecessary code taken from Mona Lisa
 *
 *	Revision 1.1  1998/06/19 14:28:43  malthoff
 *	Created. Sources taken from ML Projekt.
 *	Sources have to be reworked for GE.
 *
 *
 ******************************************************************************/


/*
 *	I2C Protocol
 */
static const char SysKonnectFileId[] =
	"$Id: ski2c.c,v 1.57 2003/01/28 09:17:38 rschmidt Exp $";

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/lm80.h"
#include "h/skdrv2nd.h"		/* Adapter Control- and Driver specific Def. */

#ifdef __C2MAN__
/*
	I2C protocol implementation.

	General Description:

	The I2C protocol is used for the temperature sensors and for
	the serial EEPROM which hold the configuration.

	This file covers functions that allow to read write and do
	some bulk requests a specified I2C address.

	The Genesis has 2 I2C buses. One for the EEPROM which holds
	the VPD Data and one for temperature and voltage sensor.
	The following picture shows the I2C buses, I2C devices and
	their control registers.

	Note: The VPD functions are in skvpd.c
.
.	PCI Config I2C Bus for VPD Data:
.
.		      +------------+
.		      | VPD EEPROM |
.		      +------------+
.			     |
.			     | <-- I2C
.			     |
.		 +-----------+-----------+
.		 |			 |
.	+-----------------+	+-----------------+
.	| PCI_VPD_ADR_REG |	| PCI_VPD_DAT_REG |
.	+-----------------+	+-----------------+
.
.
.	I2C Bus for LM80 sensor:
.
.			+-----------------+
.			| Temperature and |
.			| Voltage Sensor  |
.			| 	LM80	  |
.			+-----------------+
.				|
.				|
.			I2C --> |
.				|
.			     +----+
.	     +-------------->| OR |<--+
.	     |		     +----+   |
.     +------+------+		      |
.     |		    |		      |
. +--------+	+--------+	+----------+
. | B2_I2C |	| B2_I2C |	|  B2_I2C  |
. | _CTRL  |	| _DATA  |	|   _SW    |
. +--------+	+--------+	+----------+
.
	The I2C bus may be driven by the B2_I2C_SW or by the B2_I2C_CTRL
	and B2_I2C_DATA registers.
	For driver software it is recommended to use the I2C control and
	data register, because I2C bus timing is done by the ASIC and
	an interrupt may be received when the I2C request is completed.

	Clock Rate Timing:			MIN	MAX	generated by
		VPD EEPROM:			50 kHz	100 kHz		HW
		LM80 over I2C Ctrl/Data reg.	50 kHz	100 kHz		HW
		LM80 over B2_I2C_SW register	0	400 kHz		SW

	Note:	The clock generated by the hardware is dependend on the
		PCI clock. If the PCI bus clock is 33 MHz, the I2C/VPD
		clock is 50 kHz.
 */
intro()
{}
#endif

#ifdef	SK_DIAG
/*
 * I2C Fast Mode timing values used by the LM80.
 * If new devices are added to the I2C bus the timing values have to be checked.
 */
#ifndef I2C_SLOW_TIMING
#define	T_CLK_LOW			1300L	/* clock low time in ns */
#define	T_CLK_HIGH		 	 600L	/* clock high time in ns */
#define T_DATA_IN_SETUP		 100L	/* data in Set-up Time */
#define T_START_HOLD		 600L	/* start condition hold time */
#define T_START_SETUP		 600L	/* start condition Set-up time */
#define	T_STOP_SETUP		 600L	/* stop condition Set-up time */
#define T_BUS_IDLE			1300L	/* time the bus must free after Tx */
#define	T_CLK_2_DATA_OUT	 900L	/* max. clock low to data output valid */
#else	/* I2C_SLOW_TIMING */
/* I2C Standard Mode Timing */
#define	T_CLK_LOW			4700L	/* clock low time in ns */
#define	T_CLK_HIGH			4000L	/* clock high time in ns */
#define T_DATA_IN_SETUP		 250L	/* data in Set-up Time */
#define T_START_HOLD		4000L	/* start condition hold time */
#define T_START_SETUP		4700L	/* start condition Set-up time */
#define	T_STOP_SETUP		4000L	/* stop condition Set-up time */
#define T_BUS_IDLE			4700L	/* time the bus must free after Tx */
#endif	/* !I2C_SLOW_TIMING */

#define NS2BCLK(x)	(((x)*125)/10000)

/*
 * I2C Wire Operations
 *
 * About I2C_CLK_LOW():
 *
 * The Data Direction bit (I2C_DATA_DIR) has to be set to input when setting
 * clock to low, to prevent the ASIC and the I2C data client from driving the
 * serial data line simultaneously (ASIC: last bit of a byte = '1', I2C client
 * send an 'ACK'). See also Concentrator Bugreport No. 10192.
 */
#define I2C_DATA_HIGH(IoC)	SK_I2C_SET_BIT(IoC, I2C_DATA)
#define	I2C_DATA_LOW(IoC)	SK_I2C_CLR_BIT(IoC, I2C_DATA)
#define	I2C_DATA_OUT(IoC)	SK_I2C_SET_BIT(IoC, I2C_DATA_DIR)
#define	I2C_DATA_IN(IoC)	SK_I2C_CLR_BIT(IoC, I2C_DATA_DIR | I2C_DATA)
#define	I2C_CLK_HIGH(IoC)	SK_I2C_SET_BIT(IoC, I2C_CLK)
#define	I2C_CLK_LOW(IoC)	SK_I2C_CLR_BIT(IoC, I2C_CLK | I2C_DATA_DIR)
#define	I2C_START_COND(IoC)	SK_I2C_CLR_BIT(IoC, I2C_CLK)

#define NS2CLKT(x)	((x*125L)/10000)

/*--------------- I2C Interface Register Functions --------------- */

/*
 * sending one bit
 */
void SkI2cSndBit(
SK_IOC	IoC,	/* I/O Context */
SK_U8	Bit)	/* Bit to send */
{
	I2C_DATA_OUT(IoC);
	if (Bit) {
		I2C_DATA_HIGH(IoC);
	}
	else {
		I2C_DATA_LOW(IoC);
	}
	SkDgWaitTime(IoC, NS2BCLK(T_DATA_IN_SETUP));
	I2C_CLK_HIGH(IoC);
	SkDgWaitTime(IoC, NS2BCLK(T_CLK_HIGH));
	I2C_CLK_LOW(IoC);
}	/* SkI2cSndBit*/


/*
 * Signal a start to the I2C Bus.
 *
 * A start is signaled when data goes to low in a high clock cycle.
 *
 * Ends with Clock Low.
 *
 * Status: not tested
 */
void SkI2cStart(
SK_IOC	IoC)	/* I/O Context */
{
	/* Init data and Clock to output lines */
	/* Set Data high */
	I2C_DATA_OUT(IoC);
	I2C_DATA_HIGH(IoC);
	/* Set Clock high */
	I2C_CLK_HIGH(IoC);

	SkDgWaitTime(IoC, NS2BCLK(T_START_SETUP));

	/* Set Data Low */
	I2C_DATA_LOW(IoC);

	SkDgWaitTime(IoC, NS2BCLK(T_START_HOLD));

	/* Clock low without Data to Input */
	I2C_START_COND(IoC);

	SkDgWaitTime(IoC, NS2BCLK(T_CLK_LOW));
}	/* SkI2cStart */


void SkI2cStop(
SK_IOC	IoC)	/* I/O Context */
{
	/* Init data and Clock to output lines */
	/* Set Data low */
	I2C_DATA_OUT(IoC);
	I2C_DATA_LOW(IoC);

	SkDgWaitTime(IoC, NS2BCLK(T_CLK_2_DATA_OUT));

	/* Set Clock high */
	I2C_CLK_HIGH(IoC);

	SkDgWaitTime(IoC, NS2BCLK(T_STOP_SETUP));

	/*
	 * Set Data High:	Do it by setting the Data Line to Input.
	 *			Because of a pull up resistor the Data Line
	 *			floods to high.
	 */
	I2C_DATA_IN(IoC);

	/*
	 *	When I2C activity is stopped
	 *	 o	DATA should be set to input and
	 *	 o	CLOCK should be set to high!
	 */
	SkDgWaitTime(IoC, NS2BCLK(T_BUS_IDLE));
}	/* SkI2cStop */


/*
 * Receive just one bit via the I2C bus.
 *
 * Note:	Clock must be set to LOW before calling this function.
 *
 * Returns The received bit.
 */
int SkI2cRcvBit(
SK_IOC	IoC)	/* I/O Context */
{
	int	Bit;
	SK_U8	I2cSwCtrl;

	/* Init data as input line */
	I2C_DATA_IN(IoC);

	SkDgWaitTime(IoC, NS2BCLK(T_CLK_2_DATA_OUT));

	I2C_CLK_HIGH(IoC);

	SkDgWaitTime(IoC, NS2BCLK(T_CLK_HIGH));

	SK_I2C_GET_SW(IoC, &I2cSwCtrl);
	
	Bit = (I2cSwCtrl & I2C_DATA) ? 1 : 0;

	I2C_CLK_LOW(IoC);
	SkDgWaitTime(IoC, NS2BCLK(T_CLK_LOW-T_CLK_2_DATA_OUT));

	return(Bit);
}	/* SkI2cRcvBit */


/*
 * Receive an ACK.
 *
 * returns	0 If acknowledged
 *		1 in case of an error
 */
int SkI2cRcvAck(
SK_IOC	IoC)	/* I/O Context */
{
	/*
	 * Received bit must be zero.
	 */
	return(SkI2cRcvBit(IoC) != 0);
}	/* SkI2cRcvAck */


/*
 * Send an NACK.
 */
void SkI2cSndNAck(
SK_IOC	IoC)	/* I/O Context */
{
	/*
	 * Received bit must be zero.
	 */
	SkI2cSndBit(IoC, 1);
}	/* SkI2cSndNAck */


/*
 * Send an ACK.
 */
void SkI2cSndAck(
SK_IOC IoC)	/* I/O Context */
{
	/*
	 * Received bit must be zero.
	 *
	 */
	SkI2cSndBit(IoC, 0);
}	/* SkI2cSndAck */


/*
 * Send one byte to the I2C device and wait for ACK.
 *
 * Return acknowleged status.
 */
int SkI2cSndByte(
SK_IOC	IoC,	/* I/O Context */
int		Byte)	/* byte to send */
{
	int	i;

	for (i = 0; i < 8; i++) {
		if (Byte & (1<<(7-i))) {
			SkI2cSndBit(IoC, 1);
		}
		else {
			SkI2cSndBit(IoC, 0);
		}
	}

	return(SkI2cRcvAck(IoC));
}	/* SkI2cSndByte */


/*
 * Receive one byte and ack it.
 *
 * Return byte.
 */
int SkI2cRcvByte(
SK_IOC	IoC,	/* I/O Context */
int		Last)	/* Last Byte Flag */
{
	int	i;
	int	Byte = 0;

	for (i = 0; i < 8; i++) {
		Byte <<= 1;
		Byte |= SkI2cRcvBit(IoC);
	}

	if (Last) {
		SkI2cSndNAck(IoC);
	}
	else {
		SkI2cSndAck(IoC);
	}

	return(Byte);
}	/* SkI2cRcvByte */


/*
 * Start dialog and send device address
 *
 * Return 0 if acknowleged, 1 in case of an error
 */
int	SkI2cSndDev(
SK_IOC	IoC,	/* I/O Context */
int		Addr,	/* Device Address */
int		Rw)		/* Read / Write Flag */
{
	SkI2cStart(IoC);
	Rw = ~Rw;
	Rw &= I2C_WRITE;
	return(SkI2cSndByte(IoC, (Addr<<1) | Rw));
}	/* SkI2cSndDev */

#endif	/* SK_DIAG */

/*----------------- I2C CTRL Register Functions ----------*/

/*
 * waits for a completion of an I2C transfer
 *
 * returns	0:	success, transfer completes
 *			1:	error,	 transfer does not complete, I2C transfer
 *						 killed, wait loop terminated.
 */
int	SkI2cWait(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Event)	/* complete event to wait for (I2C_READ or I2C_WRITE) */
{
	SK_U64	StartTime;
	SK_U64	CurrentTime;
	SK_U32	I2cCtrl;

	StartTime = SkOsGetTime(pAC);
	
	do {
		CurrentTime = SkOsGetTime(pAC);

		if (CurrentTime - StartTime > SK_TICKS_PER_SEC / 8) {
			
			SK_I2C_STOP(IoC);
#ifndef SK_DIAG
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_I2C_E002, SKERR_I2C_E002MSG);
#endif	/* !SK_DIAG */
			return(1);
		}
		
		SK_I2C_GET_CTL(IoC, &I2cCtrl);

#ifdef xYUKON_DBG
		printf("StartTime=%lu, CurrentTime=%lu\n",
			StartTime, CurrentTime);
		if (kbhit()) {
			return(1);
		}
#endif /* YUKON_DBG */
	
	} while ((I2cCtrl & I2C_FLAG) == (SK_U32)Event << 31);

	return(0);
}	/* SkI2cWait */


/*
 * waits for a completion of an I2C transfer
 *
 * Returns
 *	Nothing
 */
void SkI2cWaitIrq(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	SK_SENSOR	*pSen;
	SK_U64		StartTime;
	SK_U32		IrqSrc;

	pSen = &pAC->I2c.SenTable[pAC->I2c.CurrSens];

	if (pSen->SenState == SK_SEN_IDLE) {
		return;
	}

	StartTime = SkOsGetTime(pAC);
	do {
		if (SkOsGetTime(pAC) - StartTime > SK_TICKS_PER_SEC / 8) {
			SK_I2C_STOP(IoC);
#ifndef SK_DIAG
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_I2C_E016, SKERR_I2C_E016MSG);
#endif	/* !SK_DIAG */
			return;
		}
		SK_IN32(IoC, B0_ISRC, &IrqSrc);
	} while ((IrqSrc & IS_I2C_READY) == 0);

	pSen->SenState = SK_SEN_IDLE;
	return;
}	/* SkI2cWaitIrq */

/*
 * writes a single byte or 4 bytes into the I2C device
 *
 * returns	0:	success
 *			1:	error
 */
int SkI2cWrite(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	I2cData,	/* I2C Data to write */
int		I2cDev,		/* I2C Device Address */
int		I2cReg,		/* I2C Device Register Address */
int		I2cBurst)	/* I2C Burst Flag */
{
	SK_OUT32(IoC, B2_I2C_DATA, I2cData);
	SK_I2C_CTL(IoC, I2C_WRITE, I2cDev, I2cReg, I2cBurst);
	
	return(SkI2cWait(pAC, IoC, I2C_WRITE));
}	/* SkI2cWrite*/


#ifdef	SK_DIAG

/*
 * reads a single byte or 4 bytes from the I2C device
 *
 * returns	the word read
 */
SK_U32 SkI2cRead(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		I2cDev,		/* I2C Device Address */
int		I2cReg,		/* I2C Device Register Address */
int		I2cBurst)	/* I2C Burst Flag */
{
	SK_U32	Data;

	SK_OUT32(IoC, B2_I2C_DATA, 0);
	SK_I2C_CTL(IoC, I2C_READ, I2cDev, I2cReg, I2cBurst);
	
	if (SkI2cWait(pAC, IoC, I2C_READ) != 0) {
		w_print("%s\n", SKERR_I2C_E002MSG);
	}
	
	SK_IN32(IoC, B2_I2C_DATA, &Data);
	return(Data);
}	/* SkI2cRead */

#endif	/* SK_DIAG */


/*
 * read a sensor's value
 *
 * This function reads a sensor's value from the I2C sensor chip. The sensor
 * is defined by its index into the sensors database in the struct pAC points
 * to.
 * Returns
 *		1 if the read is completed
 *		0 if the read must be continued (I2C Bus still allocated)
 */
int	SkI2cReadSensor(
SK_AC		*pAC,	/* Adapter Context */
SK_IOC		IoC,	/* I/O Context */
SK_SENSOR	*pSen)	/* Sensor to be read */
{
    if (pSen->SenRead != NULL) {
        return((*pSen->SenRead)(pAC, IoC, pSen));
    }
    else
        return(0); /* no success */
}	/* SkI2cReadSensor*/

/*
 * Do the Init state 0 initialization
 */
static int SkI2cInit0(
SK_AC	*pAC)	/* Adapter Context */
{
	int	i;

	/* Begin with first sensor */
	pAC->I2c.CurrSens = 0;
	
	/* Begin with timeout control for state machine */
	pAC->I2c.TimerMode = SK_TIMER_WATCH_STATEMACHINE;
	
	/* Set sensor number to zero */
	pAC->I2c.MaxSens = 0;

#ifndef	SK_DIAG
	/* Initialize Number of Dummy Reads */
	pAC->I2c.DummyReads = SK_MAX_SENSORS;
#endif

	for (i = 0; i < SK_MAX_SENSORS; i++) {
		pAC->I2c.SenTable[i].SenDesc = "unknown";
		pAC->I2c.SenTable[i].SenType = SK_SEN_UNKNOWN;
		pAC->I2c.SenTable[i].SenThreErrHigh = 0;
		pAC->I2c.SenTable[i].SenThreErrLow = 0;
		pAC->I2c.SenTable[i].SenThreWarnHigh = 0;
		pAC->I2c.SenTable[i].SenThreWarnLow = 0;
		pAC->I2c.SenTable[i].SenReg = LM80_FAN2_IN;
		pAC->I2c.SenTable[i].SenInit = SK_SEN_DYN_INIT_NONE;
		pAC->I2c.SenTable[i].SenValue = 0;
		pAC->I2c.SenTable[i].SenErrFlag = SK_SEN_ERR_NOT_PRESENT;
		pAC->I2c.SenTable[i].SenErrCts = 0;
		pAC->I2c.SenTable[i].SenBegErrTS = 0;
		pAC->I2c.SenTable[i].SenState = SK_SEN_IDLE;
		pAC->I2c.SenTable[i].SenRead = NULL;
		pAC->I2c.SenTable[i].SenDev = 0;
	}

	/* Now we are "INIT data"ed */
	pAC->I2c.InitLevel = SK_INIT_DATA;
	return(0);
}	/* SkI2cInit0*/


/*
 * Do the init state 1 initialization
 *
 * initialize the following register of the LM80:
 * Configuration register:
 * - START, noINT, activeLOW, noINT#Clear, noRESET, noCI, noGPO#, noINIT
 *
 * Interrupt Mask Register 1:
 * - all interrupts are Disabled (0xff)
 *
 * Interrupt Mask Register 2:
 * - all interrupts are Disabled (0xff) Interrupt modi doesn't matter.
 *
 * Fan Divisor/RST_OUT register:
 * - Divisors set to 1 (bits 00), all others 0s.
 *
 * OS# Configuration/Temperature resolution Register:
 * - all 0s
 *
 */
static int SkI2cInit1(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
    int i;
    SK_U8 I2cSwCtrl;
	SK_GEPORT *pPrt;	/* GIni Port struct pointer */

	if (pAC->I2c.InitLevel != SK_INIT_DATA) {
		/* ReInit not needed in I2C module */
		return(0);
	}

    /* Set the Direction of I2C-Data Pin to IN */
    SK_I2C_CLR_BIT(IoC, I2C_DATA_DIR | I2C_DATA);
    /* Check for 32-Bit Yukon with Low at I2C-Data Pin */
	SK_I2C_GET_SW(IoC, &I2cSwCtrl);

	if ((I2cSwCtrl & I2C_DATA) == 0) {
		/* this is a 32-Bit board */
		pAC->GIni.GIYukon32Bit = SK_TRUE;
        return(0);
    }

	/* Check for 64 Bit Yukon without sensors */
	if (SkI2cWrite(pAC, IoC, 0, LM80_ADDR, LM80_CFG, 0) != 0) {
        return(0);
    }

	(void)SkI2cWrite(pAC, IoC, 0xff, LM80_ADDR, LM80_IMSK_1, 0);
	
	(void)SkI2cWrite(pAC, IoC, 0xff, LM80_ADDR, LM80_IMSK_2, 0);
	
	(void)SkI2cWrite(pAC, IoC, 0, LM80_ADDR, LM80_FAN_CTRL, 0);
	
	(void)SkI2cWrite(pAC, IoC, 0, LM80_ADDR, LM80_TEMP_CTRL, 0);
	
	(void)SkI2cWrite(pAC, IoC, LM80_CFG_START, LM80_ADDR, LM80_CFG, 0);
	
	/*
	 * MaxSens has to be updated here, because PhyType is not
	 * set when performing Init Level 0
	 */
    pAC->I2c.MaxSens = 5;
	
	pPrt = &pAC->GIni.GP[0];
	
	if (pAC->GIni.GIGenesis) {
		if (pPrt->PhyType == SK_PHY_BCOM) {
			if (pAC->GIni.GIMacsFound == 1) {
				pAC->I2c.MaxSens += 1;
			}
			else {
				pAC->I2c.MaxSens += 3;
			}
		}
	}
	else {
		pAC->I2c.MaxSens += 3;
	}
	
	for (i = 0; i < pAC->I2c.MaxSens; i++) {
		switch (i) {
		case 0:
			pAC->I2c.SenTable[i].SenDesc = "Temperature";
			pAC->I2c.SenTable[i].SenType = SK_SEN_TEMP;
			pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_TEMP_HIGH_ERR;
			pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_TEMP_HIGH_WARN;
			pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_TEMP_LOW_WARN;
			pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_TEMP_LOW_ERR;
			pAC->I2c.SenTable[i].SenReg = LM80_TEMP_IN;
			break;
		case 1:
			pAC->I2c.SenTable[i].SenDesc = "Voltage PCI";
			pAC->I2c.SenTable[i].SenType = SK_SEN_VOLT;
			pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_PCI_5V_HIGH_ERR;
			pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_PCI_5V_HIGH_WARN;
			pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_PCI_5V_LOW_WARN;
			pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_PCI_5V_LOW_ERR;
			pAC->I2c.SenTable[i].SenReg = LM80_VT0_IN;
			break;
		case 2:
			pAC->I2c.SenTable[i].SenDesc = "Voltage PCI-IO";
			pAC->I2c.SenTable[i].SenType = SK_SEN_VOLT;
			pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_PCI_IO_5V_HIGH_ERR;
			pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_PCI_IO_5V_HIGH_WARN;
			pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_PCI_IO_3V3_LOW_WARN;
			pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_PCI_IO_3V3_LOW_ERR;
			pAC->I2c.SenTable[i].SenReg = LM80_VT1_IN;
			pAC->I2c.SenTable[i].SenInit = SK_SEN_DYN_INIT_PCI_IO;
			break;
		case 3:
			pAC->I2c.SenTable[i].SenDesc = "Voltage ASIC";
			pAC->I2c.SenTable[i].SenType = SK_SEN_VOLT;
			pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_VDD_HIGH_ERR;
			pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_VDD_HIGH_WARN;
			pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_VDD_LOW_WARN;
			pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_VDD_LOW_ERR;
			pAC->I2c.SenTable[i].SenReg = LM80_VT2_IN;
			break;
		case 4:
			if (pAC->GIni.GIGenesis) {
				if (pPrt->PhyType == SK_PHY_BCOM) {
					pAC->I2c.SenTable[i].SenDesc = "Voltage PHY A PLL";
					pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_PLL_3V3_HIGH_ERR;
					pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_PLL_3V3_HIGH_WARN;
					pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_PLL_3V3_LOW_WARN;
					pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_PLL_3V3_LOW_ERR;
				}
				else {
					pAC->I2c.SenTable[i].SenDesc = "Voltage PMA";
					pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_PLL_3V3_HIGH_ERR;
					pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_PLL_3V3_HIGH_WARN;
					pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_PLL_3V3_LOW_WARN;
					pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_PLL_3V3_LOW_ERR;
				}
			}
			else {
				pAC->I2c.SenTable[i].SenDesc = "Voltage VAUX";
				pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_VAUX_3V3_HIGH_ERR;
				pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_VAUX_3V3_HIGH_WARN;
				if (pAC->GIni.GIVauxAvail) {
					pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_VAUX_3V3_LOW_WARN;
					pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_VAUX_3V3_LOW_ERR;
				}
				else {
					pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_VAUX_0V_WARN_ERR;
					pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_VAUX_0V_WARN_ERR;
				}
			}
			pAC->I2c.SenTable[i].SenType = SK_SEN_VOLT;
			pAC->I2c.SenTable[i].SenReg = LM80_VT3_IN;
			break;
		case 5:
			if (pAC->GIni.GIGenesis) {
				pAC->I2c.SenTable[i].SenDesc = "Voltage PHY 2V5";
				pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_PHY_2V5_HIGH_ERR;
				pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_PHY_2V5_HIGH_WARN;
				pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_PHY_2V5_LOW_WARN;
				pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_PHY_2V5_LOW_ERR;
			}
			else {
				pAC->I2c.SenTable[i].SenDesc = "Voltage ASIC-Co 1V5";
				pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_CORE_1V5_HIGH_ERR;
				pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_CORE_1V5_HIGH_WARN;
				pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_CORE_1V5_LOW_WARN;
				pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_CORE_1V5_LOW_ERR;
			}
			pAC->I2c.SenTable[i].SenType = SK_SEN_VOLT;
			pAC->I2c.SenTable[i].SenReg = LM80_VT4_IN;
			break;
		case 6:
			if (pAC->GIni.GIGenesis) {
				pAC->I2c.SenTable[i].SenDesc = "Voltage PHY B PLL";
			}
			else {
				pAC->I2c.SenTable[i].SenDesc = "Voltage PHY 3V3";
			}
			pAC->I2c.SenTable[i].SenType = SK_SEN_VOLT;
			pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_PLL_3V3_HIGH_ERR;
			pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_PLL_3V3_HIGH_WARN;
			pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_PLL_3V3_LOW_WARN;
			pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_PLL_3V3_LOW_ERR;
			pAC->I2c.SenTable[i].SenReg = LM80_VT5_IN;
			break;
		case 7:
			if (pAC->GIni.GIGenesis) {
				pAC->I2c.SenTable[i].SenDesc = "Speed Fan";
				pAC->I2c.SenTable[i].SenType = SK_SEN_FAN;
				pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_FAN_HIGH_ERR;
				pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_FAN_HIGH_WARN;
				pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_FAN_LOW_WARN;
				pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_FAN_LOW_ERR;
				pAC->I2c.SenTable[i].SenReg = LM80_FAN2_IN;
			}
			else {
				pAC->I2c.SenTable[i].SenDesc = "Voltage PHY 2V5";
				pAC->I2c.SenTable[i].SenType = SK_SEN_VOLT;
				pAC->I2c.SenTable[i].SenThreErrHigh = SK_SEN_PHY_2V5_HIGH_ERR;
				pAC->I2c.SenTable[i].SenThreWarnHigh = SK_SEN_PHY_2V5_HIGH_WARN;
				pAC->I2c.SenTable[i].SenThreWarnLow = SK_SEN_PHY_2V5_LOW_WARN;
				pAC->I2c.SenTable[i].SenThreErrLow = SK_SEN_PHY_2V5_LOW_ERR;
				pAC->I2c.SenTable[i].SenReg = LM80_VT6_IN;
			}
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_INIT | SK_ERRCL_SW,
				SKERR_I2C_E001, SKERR_I2C_E001MSG);
			break;
		}

		pAC->I2c.SenTable[i].SenValue = 0;
		pAC->I2c.SenTable[i].SenErrFlag = SK_SEN_ERR_OK;
		pAC->I2c.SenTable[i].SenErrCts = 0;
		pAC->I2c.SenTable[i].SenBegErrTS = 0;
		pAC->I2c.SenTable[i].SenState = SK_SEN_IDLE;
		pAC->I2c.SenTable[i].SenRead = SkLm80ReadSensor;
		pAC->I2c.SenTable[i].SenDev = LM80_ADDR;
	}

#ifndef	SK_DIAG
	pAC->I2c.DummyReads = pAC->I2c.MaxSens;
#endif	/* !SK_DIAG */
	
	/* Clear I2C IRQ */
	SK_OUT32(IoC, B2_I2C_IRQ, I2C_CLR_IRQ);
	
	/* Now we are I/O initialized */
	pAC->I2c.InitLevel = SK_INIT_IO;
	return(0);
}	/* SkI2cInit1 */


/*
 * Init level 2: Start first sensor read.
 */
static int SkI2cInit2(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	int		ReadComplete;
	SK_SENSOR	*pSen;

	if (pAC->I2c.InitLevel != SK_INIT_IO) {
		/* ReInit not needed in I2C module */
		/* Init0 and Init2 not permitted */
		return(0);
	}

	pSen = &pAC->I2c.SenTable[pAC->I2c.CurrSens];
	ReadComplete = SkI2cReadSensor(pAC, IoC, pSen);

	if (ReadComplete) {
		SK_ERR_LOG(pAC, SK_ERRCL_INIT, SKERR_I2C_E008, SKERR_I2C_E008MSG);
	}

	/* Now we are correctly initialized */
	pAC->I2c.InitLevel = SK_INIT_RUN;

	return(0);
}	/* SkI2cInit2*/


/*
 * Initialize I2C devices
 *
 * Get the first voltage value and discard it.
 * Go into temperature read mode. A default pointer is not set.
 *
 * The things to be done depend on the init level in the parameter list:
 * Level 0:
 *	Initialize only the data structures. Do NOT access hardware.
 * Level 1:
 *	Initialize hardware through SK_IN / SK_OUT commands. Do NOT use interrupts.
 * Level 2:
 *	Everything is possible. Interrupts may be used from now on.
 *
 * return:
 *	0 = success
 *	other = error.
 */
int	SkI2cInit(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context needed in levels 1 and 2 */
int		Level)	/* Init Level */
{

	switch (Level) {
	case SK_INIT_DATA:
		return(SkI2cInit0(pAC));
	case SK_INIT_IO:
		return(SkI2cInit1(pAC, IoC));
	case SK_INIT_RUN:
		return(SkI2cInit2(pAC, IoC));
	default:
		break;
	}

	return(0);
}	/* SkI2cInit */


#ifndef SK_DIAG

/*
 * Interrupt service function for the I2C Interface
 *
 * Clears the Interrupt source
 *
 * Reads the register and check it for sending a trap.
 *
 * Starts the timer if necessary.
 */
void SkI2cIsr(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	SK_EVPARA	Para;

	/* Clear I2C IRQ */
	SK_OUT32(IoC, B2_I2C_IRQ, I2C_CLR_IRQ);

	Para.Para64 = 0;
	SkEventQueue(pAC, SKGE_I2C, SK_I2CEV_IRQ, Para);
}	/* SkI2cIsr */


/*
 * Check this sensors Value against the threshold and send events.
 */
static void SkI2cCheckSensor(
SK_AC		*pAC,	/* Adapter Context */
SK_SENSOR	*pSen)
{
	SK_EVPARA	ParaLocal;
	SK_BOOL		TooHigh;	/* Is sensor too high? */
	SK_BOOL		TooLow;		/* Is sensor too low? */
	SK_U64		CurrTime;	/* Current Time */
	SK_BOOL		DoTrapSend;	/* We need to send a trap */
	SK_BOOL		DoErrLog;	/* We need to log the error */
	SK_BOOL		IsError;	/* We need to log the error */

	/* Check Dummy Reads first */
	if (pAC->I2c.DummyReads > 0) {
		pAC->I2c.DummyReads--;
		return;
	}

	/* Get the current time */
	CurrTime = SkOsGetTime(pAC);

	/* Set para to the most useful setting: The current sensor. */
	ParaLocal.Para64 = (SK_U64)pAC->I2c.CurrSens;

	/* Check the Value against the thresholds. First: Error Thresholds */
	TooHigh = (pSen->SenValue > pSen->SenThreErrHigh);
	TooLow = (pSen->SenValue < pSen->SenThreErrLow);
		
	IsError = SK_FALSE;
	if (TooHigh || TooLow) {
		/* Error condition is satisfied */
		DoTrapSend = SK_TRUE;
		DoErrLog = SK_TRUE;

		/* Now error condition is satisfied */
		IsError = SK_TRUE;

		if (pSen->SenErrFlag == SK_SEN_ERR_ERR) {
			/* This state is the former one */

			/* So check first whether we have to send a trap */
			if (pSen->SenLastErrTrapTS + SK_SEN_ERR_TR_HOLD >
			    CurrTime) {
				/*
				 * Do NOT send the Trap. The hold back time
				 * has to run out first.
				 */
				DoTrapSend = SK_FALSE;
			}

			/* Check now whether we have to log an Error */
			if (pSen->SenLastErrLogTS + SK_SEN_ERR_LOG_HOLD >
			    CurrTime) {
				/*
				 * Do NOT log the error. The hold back time
				 * has to run out first.
				 */
				DoErrLog = SK_FALSE;
			}
		}
		else {
			/* We came from a different state -> Set Begin Time Stamp */
			pSen->SenBegErrTS = CurrTime;
			pSen->SenErrFlag = SK_SEN_ERR_ERR;
		}

		if (DoTrapSend) {
			/* Set current Time */
			pSen->SenLastErrTrapTS = CurrTime;
			pSen->SenErrCts++;

			/* Queue PNMI Event */
			SkEventQueue(pAC, SKGE_PNMI, (TooHigh ?
				SK_PNMI_EVT_SEN_ERR_UPP :
				SK_PNMI_EVT_SEN_ERR_LOW),
				ParaLocal);
		}

		if (DoErrLog) {
			/* Set current Time */
			pSen->SenLastErrLogTS = CurrTime;

			if (pSen->SenType == SK_SEN_TEMP) {
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_I2C_E011,
					SKERR_I2C_E011MSG);
			} else if (pSen->SenType == SK_SEN_VOLT) {
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_I2C_E012,
					SKERR_I2C_E012MSG);
			} else
			{
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_I2C_E015,
					SKERR_I2C_E015MSG);
			}
		}
	}

	/* Check the Value against the thresholds */
	/* 2nd: Warning thresholds */
	TooHigh = (pSen->SenValue > pSen->SenThreWarnHigh);
	TooLow = (pSen->SenValue < pSen->SenThreWarnLow);
		
	if (!IsError && (TooHigh || TooLow)) {
		/* Error condition is satisfied */
		DoTrapSend = SK_TRUE;
		DoErrLog = SK_TRUE;

		if (pSen->SenErrFlag == SK_SEN_ERR_WARN) {
			/* This state is the former one */

			/* So check first whether we have to send a trap */
			if (pSen->SenLastWarnTrapTS + SK_SEN_WARN_TR_HOLD >
			    CurrTime) {
				/*
				 * Do NOT send the Trap. The hold back time
				 * has to run out first.
				 */
				DoTrapSend = SK_FALSE;
			}

			/* Check now whether we have to log an Error */
			if (pSen->SenLastWarnLogTS + SK_SEN_WARN_LOG_HOLD >
			    CurrTime) {
				/*
				 * Do NOT log the error. The hold back time
				 * has to run out first.
				 */
				DoErrLog = SK_FALSE;
			}
		}
		else {
			/* We came from a different state -> Set Begin Time Stamp */
			pSen->SenBegWarnTS = CurrTime;
			pSen->SenErrFlag = SK_SEN_ERR_WARN;
		}

		if (DoTrapSend) {
			/* Set current Time */
			pSen->SenLastWarnTrapTS = CurrTime;
			pSen->SenWarnCts++;

			/* Queue PNMI Event */
			SkEventQueue(pAC, SKGE_PNMI, (TooHigh ?
				SK_PNMI_EVT_SEN_WAR_UPP :
				SK_PNMI_EVT_SEN_WAR_LOW),
				ParaLocal);
		}

		if (DoErrLog) {
			/* Set current Time */
			pSen->SenLastWarnLogTS = CurrTime;

			if (pSen->SenType == SK_SEN_TEMP) {
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_I2C_E009,
					SKERR_I2C_E009MSG);
			} else if (pSen->SenType == SK_SEN_VOLT) {
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_I2C_E010,
					SKERR_I2C_E010MSG);
			} else
			{
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_I2C_E014,
					SKERR_I2C_E014MSG);
			}
		}
	}

	/* Check for NO error at all */
	if (!IsError && !TooHigh && !TooLow) {
		/* Set o.k. Status if no error and no warning condition */
		pSen->SenErrFlag = SK_SEN_ERR_OK;
	}

	/* End of check against the thresholds */

	/* Bug fix AF: 16.Aug.2001: Correct the init base
	 * of LM80 sensor.
	 */
	if (pSen->SenInit == SK_SEN_DYN_INIT_PCI_IO) {

        pSen->SenInit = SK_SEN_DYN_INIT_NONE;

		if (pSen->SenValue > SK_SEN_PCI_IO_RANGE_LIMITER) {
			/* 5V PCI-IO Voltage */
			pSen->SenThreWarnLow = SK_SEN_PCI_IO_5V_LOW_WARN;
			pSen->SenThreErrLow = SK_SEN_PCI_IO_5V_LOW_ERR;
		}
		else {
			/* 3.3V PCI-IO Voltage */
			pSen->SenThreWarnHigh = SK_SEN_PCI_IO_3V3_HIGH_WARN;
			pSen->SenThreErrHigh = SK_SEN_PCI_IO_3V3_HIGH_ERR;
		}
	}
	
#if 0
    /* Dynamic thresholds also for VAUX of LM80 sensor */
	if (pSen->SenInit == SK_SEN_DYN_INIT_VAUX) {

        pSen->SenInit = SK_SEN_DYN_INIT_NONE;

		/* 3.3V VAUX Voltage */
		if (pSen->SenValue > SK_SEN_VAUX_RANGE_LIMITER) {
			pSen->SenThreWarnLow = SK_SEN_VAUX_3V3_LOW_WARN;
			pSen->SenThreErrLow = SK_SEN_VAUX_3V3_LOW_ERR;
		}
		/* 0V VAUX Voltage */
		else {
			pSen->SenThreWarnHigh = SK_SEN_VAUX_0V_WARN_ERR;
			pSen->SenThreErrHigh = SK_SEN_VAUX_0V_WARN_ERR;
		}
	}

	/*
	 * Check initialization state:
	 * The VIO Thresholds need adaption
	 */
	if (!pSen->SenInit && pSen->SenReg == LM80_VT1_IN &&
	     pSen->SenValue > SK_SEN_WARNLOW2C &&
	     pSen->SenValue < SK_SEN_WARNHIGH2) {
		pSen->SenThreErrLow = SK_SEN_ERRLOW2C;
		pSen->SenThreWarnLow = SK_SEN_WARNLOW2C;
		pSen->SenInit = SK_TRUE;
	}

	if (!pSen->SenInit && pSen->SenReg == LM80_VT1_IN &&
	     pSen->SenValue > SK_SEN_WARNLOW2 &&
	     pSen->SenValue < SK_SEN_WARNHIGH2C) {
		pSen->SenThreErrHigh = SK_SEN_ERRHIGH2C;
		pSen->SenThreWarnHigh = SK_SEN_WARNHIGH2C;
		pSen->SenInit = SK_TRUE;
	}
#endif

	if (pSen->SenInit != SK_SEN_DYN_INIT_NONE) {
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_I2C_E013, SKERR_I2C_E013MSG);
	}
}	/* SkI2cCheckSensor*/


/*
 * The only Event to be served is the timeout event
 *
 */
int	SkI2cEvent(
SK_AC		*pAC,	/* Adapter Context */
SK_IOC		IoC,	/* I/O Context */
SK_U32		Event,	/* Module specific Event */
SK_EVPARA	Para)	/* Event specific Parameter */
{
	int			ReadComplete;
	SK_SENSOR	*pSen;
	SK_U32		Time;
	SK_EVPARA	ParaLocal;
	int			i;

	/* New case: no sensors */
	if (pAC->I2c.MaxSens == 0) {
		return(0);
	}

	switch (Event) {
	case SK_I2CEV_IRQ:
		pSen = &pAC->I2c.SenTable[pAC->I2c.CurrSens];
		ReadComplete = SkI2cReadSensor(pAC, IoC, pSen);

		if (ReadComplete) {
			/* Check sensor against defined thresholds */
			SkI2cCheckSensor (pAC, pSen);

			/* Increment Current sensor and set appropriate Timeout */
			pAC->I2c.CurrSens++;
			if (pAC->I2c.CurrSens >= pAC->I2c.MaxSens) {
				pAC->I2c.CurrSens = 0;
				Time = SK_I2C_TIM_LONG;
			}
			else {
				Time = SK_I2C_TIM_SHORT;
			}

			/* Start Timer */
			ParaLocal.Para64 = (SK_U64)0;

			pAC->I2c.TimerMode = SK_TIMER_NEW_GAUGING;
			
			SkTimerStart(pAC, IoC, &pAC->I2c.SenTimer, Time,
				SKGE_I2C, SK_I2CEV_TIM, ParaLocal);
		}
        else {
			/* Start Timer */
			ParaLocal.Para64 = (SK_U64)0;

			pAC->I2c.TimerMode = SK_TIMER_WATCH_STATEMACHINE;

            SkTimerStart(pAC, IoC, &pAC->I2c.SenTimer, SK_I2C_TIM_WATCH,
				SKGE_I2C, SK_I2CEV_TIM, ParaLocal);
		}
		break;
	case SK_I2CEV_TIM:
		if (pAC->I2c.TimerMode == SK_TIMER_NEW_GAUGING) {

			ParaLocal.Para64 = (SK_U64)0;
			SkTimerStop(pAC, IoC, &pAC->I2c.SenTimer);

			pSen = &pAC->I2c.SenTable[pAC->I2c.CurrSens];
			ReadComplete = SkI2cReadSensor(pAC, IoC, pSen);

			if (ReadComplete) {
				/* Check sensor against defined thresholds */
				SkI2cCheckSensor (pAC, pSen);

				/* Increment Current sensor and set appropriate Timeout */
				pAC->I2c.CurrSens++;
				if (pAC->I2c.CurrSens == pAC->I2c.MaxSens) {
					pAC->I2c.CurrSens = 0;
					Time = SK_I2C_TIM_LONG;
				}
				else {
					Time = SK_I2C_TIM_SHORT;
				}

				/* Start Timer */
				ParaLocal.Para64 = (SK_U64)0;

				pAC->I2c.TimerMode = SK_TIMER_NEW_GAUGING;

				SkTimerStart(pAC, IoC, &pAC->I2c.SenTimer, Time,
					SKGE_I2C, SK_I2CEV_TIM, ParaLocal);
			}
		}
		else {
			pSen = &pAC->I2c.SenTable[pAC->I2c.CurrSens];
			pSen->SenErrFlag = SK_SEN_ERR_FAULTY;
			SK_I2C_STOP(IoC);

			/* Increment Current sensor and set appropriate Timeout */
			pAC->I2c.CurrSens++;
			if (pAC->I2c.CurrSens == pAC->I2c.MaxSens) {
				pAC->I2c.CurrSens = 0;
				Time = SK_I2C_TIM_LONG;
			}
			else {
				Time = SK_I2C_TIM_SHORT;
			}

			/* Start Timer */
			ParaLocal.Para64 = (SK_U64)0;

			pAC->I2c.TimerMode = SK_TIMER_NEW_GAUGING;

			SkTimerStart(pAC, IoC, &pAC->I2c.SenTimer, Time,
				SKGE_I2C, SK_I2CEV_TIM, ParaLocal);
		}
		break;
	case SK_I2CEV_CLEAR:
		for (i = 0; i < SK_MAX_SENSORS; i++) {
			pAC->I2c.SenTable[i].SenErrFlag = SK_SEN_ERR_OK;
			pAC->I2c.SenTable[i].SenErrCts = 0;
			pAC->I2c.SenTable[i].SenWarnCts = 0;
			pAC->I2c.SenTable[i].SenBegErrTS = 0;
			pAC->I2c.SenTable[i].SenBegWarnTS = 0;
			pAC->I2c.SenTable[i].SenLastErrTrapTS = (SK_U64)0;
			pAC->I2c.SenTable[i].SenLastErrLogTS = (SK_U64)0;
			pAC->I2c.SenTable[i].SenLastWarnTrapTS = (SK_U64)0;
			pAC->I2c.SenTable[i].SenLastWarnLogTS = (SK_U64)0;
		}
		break;
	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_I2C_E006, SKERR_I2C_E006MSG);
	}

	return(0);
}	/* SkI2cEvent*/

#endif	/* !SK_DIAG */
