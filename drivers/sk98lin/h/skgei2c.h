/******************************************************************************
 *
 * Name:	skgei2c.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.23 $
 * Date:	$Date: 2002/12/19 14:34:27 $
 * Purpose:	Special GEnesis defines for TWSI
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
 *	$Log: skgei2c.h,v $
 *	Revision 1.23  2002/12/19 14:34:27  rschmidt
 *	Added cast in macros SK_I2C_SET_BIT() and SK_I2C_CLR_BIT()
 *	Editorial changes (TWSI)
 *
 *	Revision 1.22  2002/10/14 16:45:56  rschmidt
 *	Editorial changes (TWSI)
 *
 *	Revision 1.21  2002/08/13 08:42:24  rschmidt
 *	Changed define for SK_MIN_SENSORS back to 5
 *	Merged defines for PHY PLL 3V3 voltage (A and B)
 *	Editorial changes
 *
 *	Revision 1.20  2002/08/06 09:43:56  jschmalz
 *	Extensions and changes for Yukon
 *
 *	Revision 1.19  2002/08/02 12:00:08  rschmidt
 *	Added defines for YUKON sensors
 *	Editorial changes
 *
 *	Revision 1.18  2001/08/16 12:44:33  afischer
 *	LM80 sensor init values corrected
 *
 *	Revision 1.17  1999/11/22 13:55:25  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.16  1999/11/12 08:24:10  malthoff
 *	Change voltage warning and error limits
 *	(warning +-5%, error +-10%).
 *
 *	Revision 1.15  1999/09/14 14:14:43  malthoff
 *	The 1000BT Dual Link adapter has got only one Fan.
 *	The second Fan has been removed.
 *
 *	Revision 1.14  1999/05/27 13:40:50  malthoff
 *	Fan Divisor = 1. Assuming fan with 6500 rpm.
 *
 *	Revision 1.13  1999/05/20 14:56:55  malthoff
 *	Bug Fix: Missing brace in SK_LM80_FAN_FAKTOR.
 *
 *	Revision 1.12  1999/05/20 09:22:00  cgoos
 *	Changes for 1000Base-T (Fan sensors).
 *
 *	Revision 1.11  1998/10/14 05:57:22  cgoos
 *	Fixed compilation warnings.
 *
 *	Revision 1.10  1998/09/04 08:37:00  malthoff
 *	bugfix: correct the SK_I2C_GET_CTL() macro.
 *
 *	Revision 1.9  1998/08/25 06:10:03  gklug
 *	add: thresholds for all sensors
 *
 *	Revision 1.8  1998/08/20 11:37:42  gklug
 *	chg: change Ioc to IoC
 *
 *	Revision 1.7  1998/08/20 08:53:11  gklug
 *	fix: compiler errors
 *	add: Threshold values
 *
 *	Revision 1.6  1998/08/17 11:37:09  malthoff
 *	Bugfix in SK_I2C_CTL macro. The parameter 'dev'
 *	has to be shifted 9 bits.
 *
 *	Revision 1.5  1998/08/17 06:52:21  malthoff
 *	Remove unrequired macros.
 *	Add macros for accessing TWSI SW register.
 *
 *	Revision 1.4  1998/08/13 08:30:18  gklug
 *	add: conversion factors for read values
 *	add: new state SEN_VALEXT to read extension value of temperature sensor
 *
 *	Revision 1.3  1998/08/12 13:37:56  gklug
 *	rmv: error numbers and messages
 *
 *	Revision 1.2  1998/08/11 07:54:38  gklug
 *	add: sensor states for GE sensors
 *	add: Macro to access TWSI hardware register
 *	chg: Error messages for TWSI errors
 *
 *	Revision 1.1  1998/07/17 11:27:56  gklug
 *	Created.
 *
 *
 *
 ******************************************************************************/

/*
 * SKGEI2C.H	contains all SK-98xx specific defines for the TWSI handling
 */

#ifndef _INC_SKGEI2C_H_
#define _INC_SKGEI2C_H_

/*
 * Macros to access the B2_I2C_CTRL
 */
#define SK_I2C_CTL(IoC, flag, dev, reg, burst) \
	SK_OUT32(IoC, B2_I2C_CTRL,\
		(flag ? 0x80000000UL : 0x0L) | \
		(((SK_U32) reg << 16) & I2C_ADDR) | \
		(((SK_U32) dev << 9) & I2C_DEV_SEL) | \
		(( burst << 4) & I2C_BURST_LEN))

#define SK_I2C_STOP(IoC) {				\
	SK_U32	I2cCtrl;				\
	SK_IN32(IoC, B2_I2C_CTRL, &I2cCtrl);		\
	SK_OUT32(IoC, B2_I2C_CTRL, I2cCtrl | I2C_STOP);	\
}

#define SK_I2C_GET_CTL(IoC, pI2cCtrl)	SK_IN32(IoC, B2_I2C_CTRL, pI2cCtrl)

/*
 * Macros to access the TWSI SW Registers
 */
#define SK_I2C_SET_BIT(IoC, SetBits) {			\
	SK_U8	OrgBits;				\
	SK_IN8(IoC, B2_I2C_SW, &OrgBits);		\
	SK_OUT8(IoC, B2_I2C_SW, OrgBits | (SK_U8)(SetBits));	\
}

#define SK_I2C_CLR_BIT(IoC, ClrBits) {			\
	SK_U8	OrgBits;				\
	SK_IN8(IoC, B2_I2C_SW, &OrgBits);		\
	SK_OUT8(IoC, B2_I2C_SW, OrgBits & ~((SK_U8)(ClrBits)));	\
}

#define SK_I2C_GET_SW(IoC, pI2cSw)	SK_IN8(IoC, B2_I2C_SW, pI2cSw)

/*
 * define the possible sensor states
 */
#define	SK_SEN_IDLE		0	/* Idle: sensor not read */
#define	SK_SEN_VALUE	1	/* Value Read cycle */
#define	SK_SEN_VALEXT	2	/* Extended Value Read cycle */

/*
 * Conversion factor to convert read Voltage sensor to milli Volt
 * Conversion factor to convert read Temperature sensor to 10th degree Celsius
 */
#define	SK_LM80_VT_LSB		22	/* 22mV LSB resolution */
#define	SK_LM80_TEMP_LSB	10	/* 1 degree LSB resolution */
#define	SK_LM80_TEMPEXT_LSB	5	/* 0.5 degree LSB resolution for the
					 * extension value
					 */
#define SK_LM80_FAN_FAKTOR	((22500L*60)/(1*2))
/* formula: counter = (22500*60)/(rpm * divisor * pulses/2)
 * assuming: 6500rpm, 4 pulses, divisor 1
 */

/*
 * Define sensor management data
 * Maximum is reached on copperfield with dual Broadcom.
 * Board specific maximum is in pAC->I2c.MaxSens
 */
#define	SK_MAX_SENSORS	8	/* maximal no. of installed sensors */
#define	SK_MIN_SENSORS	5	/* minimal no. of installed sensors */

/*
 * To watch the statemachine (JS) use the timer in two ways instead of one as hitherto
 */
#define	SK_TIMER_WATCH_STATEMACHINE	0	/* Watch the statemachine to finish in a specific time */
#define	SK_TIMER_NEW_GAUGING    	1	/* Start a new gauging when timer expires */


/*
 * Defines for the individual Thresholds
 */

/* Temperature sensor */
#define	SK_SEN_TEMP_HIGH_ERR    800	/* Temperature High Err  Threshold */
#define	SK_SEN_TEMP_HIGH_WARN	700	/* Temperature High Warn Threshold */
#define	SK_SEN_TEMP_LOW_WARN	100	/* Temperature Low  Warn Threshold */
#define	SK_SEN_TEMP_LOW_ERR       0	/* Temperature Low  Err  Threshold */

/* VCC which should be 5 V */
#define	SK_SEN_PCI_5V_HIGH_ERR  	5588	/* Voltage PCI High Err  Threshold */
#define	SK_SEN_PCI_5V_HIGH_WARN     5346	/* Voltage PCI High Warn Threshold */
#define	SK_SEN_PCI_5V_LOW_WARN		4664	/* Voltage PCI Low  Warn Threshold */
#define	SK_SEN_PCI_5V_LOW_ERR		4422	/* Voltage PCI Low  Err  Threshold */

/*
 * VIO may be 5 V or 3.3 V. Initialization takes two parts:
 * 1. Initialize lowest lower limit and highest higher limit.
 * 2. After the first value is read correct the upper or the lower limit to
 *    the appropriate C constant.
 *
 * Warning limits are +-5% of the exepected voltage.
 * Error limits are +-10% of the expected voltage.
 */

/* Bug fix AF: 16.Aug.2001: Correct the init base of LM80 sensor */

#define	SK_SEN_PCI_IO_5V_HIGH_ERR	5566	/* + 10% V PCI-IO High Err Threshold */
#define	SK_SEN_PCI_IO_5V_HIGH_WARN	5324	/* +  5% V PCI-IO High Warn Threshold */
					/*		5000	mVolt */
#define	SK_SEN_PCI_IO_5V_LOW_WARN	4686	/* -  5% V PCI-IO Low Warn Threshold */
#define	SK_SEN_PCI_IO_5V_LOW_ERR	4444	/* - 10% V PCI-IO Low Err Threshold */

#define	SK_SEN_PCI_IO_RANGE_LIMITER	4000	/* 4000 mV range delimiter */

/* correction values for the second pass */
#define	SK_SEN_PCI_IO_3V3_HIGH_ERR	3850	/* + 15% V PCI-IO High Err Threshold */
#define	SK_SEN_PCI_IO_3V3_HIGH_WARN	3674	/* + 10% V PCI-IO High Warn Threshold */
					/*		3300	mVolt */
#define	SK_SEN_PCI_IO_3V3_LOW_WARN  2926	/* - 10% V PCI-IO Low Warn Threshold */
#define	SK_SEN_PCI_IO_3V3_LOW_ERR   2772	/* - 15% V PCI-IO Low Err  Threshold */


/*
 * VDD voltage
 */
#define	SK_SEN_VDD_HIGH_ERR	    3630	/* Voltage ASIC High Err  Threshold */
#define	SK_SEN_VDD_HIGH_WARN    3476	/* Voltage ASIC High Warn Threshold */
#define	SK_SEN_VDD_LOW_WARN     3146	/* Voltage ASIC Low  Warn Threshold */
#define	SK_SEN_VDD_LOW_ERR      2970	/* Voltage ASIC Low  Err  Threshold */

/*
 * PHY PLL 3V3 voltage
 */
#define	SK_SEN_PLL_3V3_HIGH_ERR		3630	/* Voltage PMA High Err  Threshold */
#define	SK_SEN_PLL_3V3_HIGH_WARN	3476	/* Voltage PMA High Warn Threshold */
#define	SK_SEN_PLL_3V3_LOW_WARN		3146	/* Voltage PMA Low  Warn Threshold */
#define	SK_SEN_PLL_3V3_LOW_ERR		2970	/* Voltage PMA Low  Err  Threshold */

/*
 * VAUX (YUKON only)
 */
#define	SK_SEN_VAUX_3V3_HIGH_ERR	3630	/* Voltage VAUX High Err Threshold */
#define	SK_SEN_VAUX_3V3_HIGH_WARN	3476	/* Voltage VAUX High Warn Threshold */
#define	SK_SEN_VAUX_3V3_LOW_WARN	3146	/* Voltage VAUX Low Warn Threshold */
#define	SK_SEN_VAUX_3V3_LOW_ERR	    2970	/* Voltage VAUX Low Err Threshold */
#define	SK_SEN_VAUX_0V_WARN_ERR	       0	/* if VAUX not present */
#define	SK_SEN_VAUX_RANGE_LIMITER	1000	/* 1000 mV range delimiter */

/*
 * PHY 2V5 voltage
 */
#define	SK_SEN_PHY_2V5_HIGH_ERR		2750	/* Voltage PHY High Err Threshold */
#define	SK_SEN_PHY_2V5_HIGH_WARN	2640	/* Voltage PHY High Warn Threshold */
#define	SK_SEN_PHY_2V5_LOW_WARN		2376	/* Voltage PHY Low Warn Threshold */
#define	SK_SEN_PHY_2V5_LOW_ERR		2222	/* Voltage PHY Low Err Threshold */

/*
 * ASIC Core 1V5 voltage (YUKON only)
 */
#define	SK_SEN_CORE_1V5_HIGH_ERR    1650	/* Voltage ASIC Core High Err Threshold */
#define	SK_SEN_CORE_1V5_HIGH_WARN	1575	/* Voltage ASIC Core High Warn Threshold */
#define	SK_SEN_CORE_1V5_LOW_WARN	1425	/* Voltage ASIC Core Low Warn Threshold */
#define	SK_SEN_CORE_1V5_LOW_ERR 	1350	/* Voltage ASIC Core Low Err Threshold */

/*
 * FAN 1 speed
 */
/* assuming: 6500rpm +-15%, 4 pulses,
 * warning at:	80 %
 * error at:	70 %
 * no upper limit
 */
#define	SK_SEN_FAN_HIGH_ERR		20000	/* FAN Speed High Err Threshold */
#define	SK_SEN_FAN_HIGH_WARN	20000	/* FAN Speed High Warn Threshold */
#define	SK_SEN_FAN_LOW_WARN 	5200	/* FAN Speed Low Warn Threshold */
#define	SK_SEN_FAN_LOW_ERR		4550	/* FAN Speed Low Err Threshold */

/*
 * Some Voltages need dynamic thresholds
 */
#define	SK_SEN_DYN_INIT_NONE		 0  /* No dynamic init of thresholds */
#define	SK_SEN_DYN_INIT_PCI_IO		10  /* Init PCI-IO with new thresholds */
#define	SK_SEN_DYN_INIT_VAUX		11  /* Init VAUX with new thresholds */

extern	int SkLm80ReadSensor(SK_AC *pAC, SK_IOC IoC, SK_SENSOR *pSen);
#endif	/* n_INC_SKGEI2C_H */
