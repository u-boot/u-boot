/******************************************************************************
 *
 * Name:	ski2c.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.34 $
 * Date:	$Date: 2003/01/28 09:11:21 $
 * Purpose:	Defines to access Voltage and Temperature Sensor
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
 *	$Log: ski2c.h,v $
 *	Revision 1.34  2003/01/28 09:11:21  rschmidt
 *	Editorial changes
 *	
 *	Revision 1.33  2002/10/14 16:40:50  rschmidt
 *	Editorial changes (TWSI)
 *	
 *	Revision 1.32  2002/08/13 08:55:07  rschmidt
 *	Editorial changes
 *	
 *	Revision 1.31  2002/08/06 09:44:22  jschmalz
 *	Extensions and changes for Yukon
 *	
 *	Revision 1.30  2001/04/05 11:38:09  rassmann
 *	Set SenState to idle in SkI2cWaitIrq().
 *	Changed error message in SkI2cWaitIrq().
 *	
 *	Revision 1.29  2000/08/03 14:28:17  rassmann
 *	- Added function to wait for I2C being ready before resetting the board.
 *	- Replaced one duplicate "out of range" message with correct one.
 *	
 *	Revision 1.28  1999/11/22 13:55:46  cgoos
 *	Changed license header to GPL.
 *	
 *	Revision 1.27  1999/05/20 09:23:10  cgoos
 *	Changes for 1000Base-T (Fan sensors).
 *	
 *	Revision 1.26  1998/12/01 13:45:47  gklug
 *	add: InitLevel to I2c struct
 *	
 *	Revision 1.25  1998/11/03 06:55:16  gklug
 *	add: Dummy Reads to I2c struct
 *	
 *	Revision 1.24  1998/10/02 14:28:59  cgoos
 *	Added prototype for SkI2cIsr.
 *	
 *	Revision 1.23  1998/09/08 12:20:11  gklug
 *	add: prototypes for init and read functions
 *	
 *	Revision 1.22  1998/09/08 07:37:56  gklug
 *	add: log error if PCI_IO voltage sensor could not be initialized
 *	
 *	Revision 1.21  1998/09/04 08:38:05  malthoff
 *	Change the values for I2C_READ and I2C_WRITE
 *	
 *	Revision 1.20  1998/08/25 07:52:22  gklug
 *	chg: Timestamps (last) added for logging
 *
 *	Revision 1.19  1998/08/25 06:09:00  gklug
 *	rmv: warning and error levels of the individual sensors.
 *	add: timing definitions for sending traps and logging errors
 *	
 *	Revision 1.18  1998/08/20 11:41:15  gklug
 *	chg: omit STRCPY macro by using char * as Sensor Description
 *	
 *	Revision 1.17  1998/08/20 11:37:43  gklug
 *	chg: change Ioc to IoC
 *	
 *	Revision 1.16  1998/08/20 11:30:38  gklug
 *	fix: SenRead declaration
 *	
 *	Revision 1.15  1998/08/20 11:27:53  gklug
 *	fix: Compile bugs with new awrning constants
 *	
 *	Revision 1.14  1998/08/20 08:53:12  gklug
 *	fix: compiler errors
 *	add: Threshold values
 *	
 *	Revision 1.13  1998/08/19 12:21:16  gklug
 *	fix: remove struct from C files (see CCC)
 *	add: typedefs for all structs
 *	
 *	Revision 1.12  1998/08/19 10:57:41  gklug
 *	add: Warning levels
 *	
 *	Revision 1.11  1998/08/18 08:37:02  malthoff
 *	Prototypes not required for SK_DIAG.
 *	
 *	Revision 1.10  1998/08/17 13:54:00  gklug
 *	fix: declaration of event function
 *	
 *	Revision 1.9  1998/08/17 06:48:39  malthoff
 *	Remove some unrequired macros.
 *	Fix the compiler errors.
 *	
 *	Revision 1.8  1998/08/14 06:47:19  gklug
 *	fix: Values are intergers
 *
 *	Revision 1.7  1998/08/14 06:26:05  gklug
 *	add: Init error message
 *
 *	Revision 1.6  1998/08/13 08:31:08  gklug
 *	add: Error message
 *
 *	Revision 1.5  1998/08/12 14:32:04  gklug
 *	add: new error code/message
 *
 *	Revision 1.4  1998/08/12 13:39:08  gklug
 *	chg: names of error messages
 *	add: defines for Sensor type and thresholds
 *
 *	Revision 1.3  1998/08/11 07:57:16  gklug
 *	add: sensor struct
 *	add: Timeout defines
 *	add: I2C control struct for pAC
 *
 *	Revision 1.2  1998/07/17 11:29:02  gklug
 *	rmv: Microwire and SMTPANIC
 *
 *	Revision 1.1  1998/06/19 14:30:10  malthoff
 *	Created. Sources taken from ML Project.
 *
 *
 ******************************************************************************/

/*
 * SKI2C.H	contains all I2C specific defines
 */

#ifndef _SKI2C_H_
#define _SKI2C_H_

typedef struct  s_Sensor SK_SENSOR;

#include "h/skgei2c.h"

/*
 * Define the I2C events.
 */
#define SK_I2CEV_IRQ	1	/* IRQ happened Event */
#define SK_I2CEV_TIM	2	/* Timeout event */
#define SK_I2CEV_CLEAR	3	/* Clear MIB Values */

/*
 * Define READ and WRITE Constants.
 */
#define I2C_READ	0
#define I2C_WRITE	1
#define I2C_BURST	1
#define I2C_SINGLE	0

#define SKERR_I2C_E001		(SK_ERRBASE_I2C+0)
#define SKERR_I2C_E001MSG	"Sensor index unknown"
#define SKERR_I2C_E002		(SKERR_I2C_E001+1)
#define SKERR_I2C_E002MSG	"TWSI: transfer does not complete"
#define SKERR_I2C_E003		(SKERR_I2C_E002+1)
#define SKERR_I2C_E003MSG	"LM80: NAK on device send"
#define SKERR_I2C_E004		(SKERR_I2C_E003+1)
#define SKERR_I2C_E004MSG	"LM80: NAK on register send"
#define SKERR_I2C_E005		(SKERR_I2C_E004+1)
#define SKERR_I2C_E005MSG	"LM80: NAK on device (2) send"
#define SKERR_I2C_E006		(SKERR_I2C_E005+1)
#define SKERR_I2C_E006MSG	"Unknown event"
#define SKERR_I2C_E007		(SKERR_I2C_E006+1)
#define SKERR_I2C_E007MSG	"LM80 read out of state"
#define SKERR_I2C_E008		(SKERR_I2C_E007+1)
#define SKERR_I2C_E008MSG	"Unexpected sensor read completed"
#define SKERR_I2C_E009		(SKERR_I2C_E008+1)
#define SKERR_I2C_E009MSG	"WARNING: temperature sensor out of range"
#define SKERR_I2C_E010		(SKERR_I2C_E009+1)
#define SKERR_I2C_E010MSG	"WARNING: voltage sensor out of range"
#define SKERR_I2C_E011		(SKERR_I2C_E010+1)
#define SKERR_I2C_E011MSG	"ERROR: temperature sensor out of range"
#define SKERR_I2C_E012		(SKERR_I2C_E011+1)
#define SKERR_I2C_E012MSG	"ERROR: voltage sensor out of range"
#define SKERR_I2C_E013		(SKERR_I2C_E012+1)
#define SKERR_I2C_E013MSG	"ERROR: couldn't init sensor"
#define SKERR_I2C_E014		(SKERR_I2C_E013+1)
#define SKERR_I2C_E014MSG	"WARNING: fan sensor out of range"
#define SKERR_I2C_E015		(SKERR_I2C_E014+1)
#define SKERR_I2C_E015MSG	"ERROR: fan sensor out of range"
#define SKERR_I2C_E016		(SKERR_I2C_E015+1)
#define SKERR_I2C_E016MSG	"TWSI: active transfer does not complete"

/*
 * Define Timeout values
 */
#define SK_I2C_TIM_LONG		2000000L	/* 2 seconds */
#define SK_I2C_TIM_SHORT	 100000L	/* 100 milliseconds */
#define SK_I2C_TIM_WATCH	1000000L	/* 1 second */

/*
 * Define trap and error log hold times
 */
#ifndef	SK_SEN_ERR_TR_HOLD
#define SK_SEN_ERR_TR_HOLD		(4*SK_TICKS_PER_SEC)
#endif
#ifndef	SK_SEN_ERR_LOG_HOLD
#define SK_SEN_ERR_LOG_HOLD		(60*SK_TICKS_PER_SEC)
#endif
#ifndef	SK_SEN_WARN_TR_HOLD
#define SK_SEN_WARN_TR_HOLD		(15*SK_TICKS_PER_SEC)
#endif
#ifndef	SK_SEN_WARN_LOG_HOLD
#define SK_SEN_WARN_LOG_HOLD	(15*60*SK_TICKS_PER_SEC)
#endif

/*
 * Defines for SenType
 */
#define SK_SEN_UNKNOWN	0
#define SK_SEN_TEMP		1
#define SK_SEN_VOLT		2
#define SK_SEN_FAN		3

/*
 * Define for the SenErrorFlag
 */
#define SK_SEN_ERR_NOT_PRESENT	0	/* Error Flag: Sensor not present */
#define SK_SEN_ERR_OK			1	/* Error Flag: O.K. */
#define SK_SEN_ERR_WARN			2	/* Error Flag: Warning */
#define SK_SEN_ERR_ERR			3	/* Error Flag: Error */
#define SK_SEN_ERR_FAULTY		4	/* Error Flag: Faulty */

/*
 * Define the Sensor struct
 */
struct	s_Sensor {
	char	*SenDesc;			/* Description */
	int		SenType;			/* Voltage or Temperature */
	SK_I32	SenValue;			/* Current value of the sensor */
	SK_I32	SenThreErrHigh;		/* High error Threshhold of this sensor */
	SK_I32	SenThreWarnHigh;	/* High warning Threshhold of this sensor */
	SK_I32	SenThreErrLow;		/* Lower error Threshold of the sensor */
	SK_I32	SenThreWarnLow;		/* Lower warning Threshold of the sensor */
	int		SenErrFlag;			/* Sensor indicated an error */
	SK_BOOL	SenInit;			/* Is sensor initialized ? */
	SK_U64	SenErrCts;			/* Error  trap counter */
	SK_U64	SenWarnCts;			/* Warning trap counter */
	SK_U64	SenBegErrTS;		/* Begin error timestamp */
	SK_U64	SenBegWarnTS;		/* Begin warning timestamp */
	SK_U64	SenLastErrTrapTS;	/* Last error trap timestamp */
	SK_U64	SenLastErrLogTS;	/* Last error log timestamp */
	SK_U64	SenLastWarnTrapTS;	/* Last warning trap timestamp */
	SK_U64	SenLastWarnLogTS;	/* Last warning log timestamp */
	int		SenState;			/* Sensor State (see HW specific include) */
	int		(*SenRead)(SK_AC *pAC, SK_IOC IoC, struct s_Sensor *pSen);
								/* Sensors read function */
	SK_U16	SenReg;				/* Register Address for this sensor */
	SK_U8	SenDev;				/* Device Selection for this sensor */
};

typedef	struct	s_I2c {
	SK_SENSOR	SenTable[SK_MAX_SENSORS];	/* Sensor Table */
	int			CurrSens;	/* Which sensor is currently queried */
	int			MaxSens;	/* Max. number of sensors */
	int			TimerMode;	/* Use the timer also to watch the state machine */
	int			InitLevel;	/* Initialized Level */
#ifndef SK_DIAG
	int			DummyReads;	/* Number of non-checked dummy reads */
	SK_TIMER	SenTimer;	/* Sensors timer */
#endif /* !SK_DIAG */
} SK_I2C;

extern int SkI2cReadSensor(SK_AC *pAC, SK_IOC IoC, SK_SENSOR *pSen);
#ifndef SK_DIAG
extern int SkI2cEvent(SK_AC *pAC, SK_IOC IoC, SK_U32 Event, SK_EVPARA Para);
extern int SkI2cInit(SK_AC *pAC, SK_IOC IoC, int Level);
extern void SkI2cWaitIrq(SK_AC *pAC, SK_IOC IoC);
extern void SkI2cIsr(SK_AC *pAC, SK_IOC IoC);

#endif
#endif /* n_SKI2C_H */

