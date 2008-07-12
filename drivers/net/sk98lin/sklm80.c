/******************************************************************************
 *
 * Name:	sklm80.c
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.20 $
 * Date:	$Date: 2002/08/13 09:16:27 $
 * Purpose:	Funktions to access Voltage and Temperature Sensor (LM80)
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
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
 *	$Log: sklm80.c,v $
 *	Revision 1.20  2002/08/13 09:16:27  rschmidt
 *	Changed return value for SkLm80ReadSensor() back to 'int'
 *	Editorial changes
 *
 *	Revision 1.19  2002/08/06 09:43:31  jschmalz
 *	Extensions and changes for Yukon
 *
 *	Revision 1.18  2002/08/02 12:26:57  rschmidt
 *	Editorial changes
 *
 *	Revision 1.17  1999/11/22 13:35:51  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.16  1999/05/27 14:05:47  malthoff
 *	Fans: Set SenVal to 0 if the fan value is 0 or 0xff. Both values
 *	are outside the limits (0: div zero error, 0xff: value not in
 *	range, assume 0).
 *
 *	Revision 1.15  1999/05/27 13:38:51  malthoff
 *	Pervent from Division by zero errors.
 *
 *	Revision 1.14  1999/05/20 09:20:01  cgoos
 *	Changes for 1000Base-T (Fan sensors).
 *
 *	Revision 1.13  1998/10/22 09:48:14  gklug
 *	fix: SysKonnectFileId typo
 *
 *	Revision 1.12  1998/10/09 06:12:06  malthoff
 *	Remove ID_sccs by SysKonnectFileId.
 *
 *	Revision 1.11  1998/09/04 08:33:48  malthoff
 *	bug fix: SenState = SK_SEN_IDLE when
 *	leaving SK_SEN_VALEXT state
 *
 *	Revision 1.10  1998/08/20 12:02:10  gklug
 *	fix: compiler warnings type mismatch
 *
 *	Revision 1.9  1998/08/20 11:37:38  gklug
 *	chg: change Ioc to IoC
 *
 *	Revision 1.8  1998/08/19 12:20:58  gklug
 *	fix: remove struct from C files (see CCC)
 *
 *	Revision 1.7  1998/08/17 07:04:57  malthoff
 *	Take SkLm80RcvReg() function from ski2c.c.
 *	Add IoC parameter to BREAK_OR_WAIT() macro.
 *
 *	Revision 1.6  1998/08/14 07:11:28  malthoff
 *	remove pAc with pAC.
 *
 *	Revision 1.5  1998/08/14 06:46:55  gklug
 *	fix: temperature can get negative
 *
 *	Revision 1.4  1998/08/13 08:27:04  gklug
 *	add: temperature reading now o.k.
 *	fix: pSen declaration, SK_ERR_LOG call, ADDR macro
 *
 *	Revision 1.3  1998/08/13 07:28:21  gklug
 *	fix: pSen was wrong initialized
 *	add: correct conversion for voltage readings
 *
 *	Revision 1.2  1998/08/11 07:52:14  gklug
 *	add: Lm80 read sensor function
 *
 *	Revision 1.1  1998/07/17 09:57:12  gklug
 *	initial version
 *
 *
 *
 ******************************************************************************/


#include <config.h>

/*
	LM80 functions
*/
static const char SysKonnectFileId[] =
	"$Id: sklm80.c,v 1.20 2002/08/13 09:16:27 rschmidt Exp $" ;

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/lm80.h"
#include "h/skdrv2nd.h"		/* Adapter Control- and Driver specific Def. */

#ifdef	SK_DIAG
#define	BREAK_OR_WAIT(pAC,IoC,Event)	SkI2cWait(pAC,IoC,Event)
#else	/* nSK_DIAG */
#define	BREAK_OR_WAIT(pAC,IoC,Event)	break
#endif	/* nSK_DIAG */

#ifdef	SK_DIAG
/*
 * read the register 'Reg' from the device 'Dev'
 *
 * return	read error	-1
 *		success		the read value
 */
int	SkLm80RcvReg(
SK_IOC	IoC,		/* Adapter Context */
int		Dev,		/* I2C device address */
int		Reg)		/* register to read */
{
	int	Val = 0;
	int	TempExt;

	/* Signal device number */
	if (SkI2cSndDev(IoC, Dev, I2C_WRITE)) {
		return(-1);
	}

	if (SkI2cSndByte(IoC, Reg)) {
		return(-1);
	}

	/* repeat start */
	if (SkI2cSndDev(IoC, Dev, I2C_READ)) {
		return(-1);
	}

	switch (Reg) {
	case LM80_TEMP_IN:
		Val = (int)SkI2cRcvByte(IoC, 1);

		/* First: correct the value: it might be negative */
		if ((Val & 0x80) != 0) {
			/* Value is negative */
			Val = Val - 256;
		}
		Val = Val * SK_LM80_TEMP_LSB;
		SkI2cStop(IoC);

		TempExt = (int)SkLm80RcvReg(IoC, LM80_ADDR, LM80_TEMP_CTRL);

		if (Val > 0) {
			Val += ((TempExt >> 7) * SK_LM80_TEMPEXT_LSB);
		}
		else {
			Val -= ((TempExt >> 7) * SK_LM80_TEMPEXT_LSB);
		}
		return(Val);
		break;
	case LM80_VT0_IN:
	case LM80_VT1_IN:
	case LM80_VT2_IN:
	case LM80_VT3_IN:
		Val = (int)SkI2cRcvByte(IoC, 1) * SK_LM80_VT_LSB;
		break;

	default:
		Val = (int)SkI2cRcvByte(IoC, 1);
		break;
	}

	SkI2cStop(IoC);
	return(Val);
}
#endif	/* SK_DIAG */

/*
 * read a sensors value (LM80 specific)
 *
 * This function reads a sensors value from the I2C sensor chip LM80.
 * The sensor is defined by its index into the sensors database in the struct
 * pAC points to.
 *
 * Returns	1 if the read is completed
 *		0 if the read must be continued (I2C Bus still allocated)
 */
int SkLm80ReadSensor(
SK_AC		*pAC,	/* Adapter Context */
SK_IOC		IoC,	/* I/O Context needed in level 1 and 2 */
SK_SENSOR	*pSen)	/* Sensor to be read */
{
	SK_I32		Value;

	switch (pSen->SenState) {
	case SK_SEN_IDLE:
		/* Send address to ADDR register */
		SK_I2C_CTL(IoC, I2C_READ, pSen->SenDev, pSen->SenReg, 0);

		pSen->SenState = SK_SEN_VALUE ;
		BREAK_OR_WAIT(pAC, IoC, I2C_READ);

	case SK_SEN_VALUE:
		/* Read value from data register */
		SK_IN32(IoC, B2_I2C_DATA, ((SK_U32 *)&Value));

		Value &= 0xff; /* only least significant byte is valid */

		/* Do NOT check the Value against the thresholds */
		/* Checking is done in the calling instance */

		if (pSen->SenType == SK_SEN_VOLT) {
			/* Voltage sensor */
			pSen->SenValue = Value * SK_LM80_VT_LSB;
			pSen->SenState = SK_SEN_IDLE ;
			return(1);
		}

		if (pSen->SenType == SK_SEN_FAN) {
			if (Value != 0 && Value != 0xff) {
				/* Fan speed counter */
				pSen->SenValue = SK_LM80_FAN_FAKTOR/Value;
			}
			else {
				/* Indicate Fan error */
				pSen->SenValue = 0;
			}
			pSen->SenState = SK_SEN_IDLE ;
			return(1);
		}

		/* First: correct the value: it might be negative */
		if ((Value & 0x80) != 0) {
			/* Value is negative */
			Value = Value - 256;
		}

		/* We have a temperature sensor and need to get the signed extension.
		 * For now we get the extension from the last reading, so in the normal
		 * case we won't see flickering temperatures.
		 */
		pSen->SenValue = (Value * SK_LM80_TEMP_LSB) +
			(pSen->SenValue % SK_LM80_TEMP_LSB);

		/* Send address to ADDR register */
		SK_I2C_CTL(IoC, I2C_READ, pSen->SenDev, LM80_TEMP_CTRL, 0);

		pSen->SenState = SK_SEN_VALEXT ;
		BREAK_OR_WAIT(pAC, IoC, I2C_READ);

	case SK_SEN_VALEXT:
		/* Read value from data register */
		SK_IN32(IoC, B2_I2C_DATA, ((SK_U32 *)&Value));
		Value &= LM80_TEMP_LSB_9; /* only bit 7 is valid */

		/* cut the LSB bit */
		pSen->SenValue = ((pSen->SenValue / SK_LM80_TEMP_LSB) *
			SK_LM80_TEMP_LSB);

		if (pSen->SenValue < 0) {
			/* Value negative: The bit value must be subtracted */
			pSen->SenValue -= ((Value >> 7) * SK_LM80_TEMPEXT_LSB);
		}
		else {
			/* Value positive: The bit value must be added */
			pSen->SenValue += ((Value >> 7) * SK_LM80_TEMPEXT_LSB);
		}

		pSen->SenState = SK_SEN_IDLE ;
		return(1);

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_I2C_E007, SKERR_I2C_E007MSG);
		return(1);
	}

	/* Not completed */
	return(0);
}
