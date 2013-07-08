/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

/*
 * Digital Thermometers and Thermostats.
 */
#ifndef _DTT_H_
#define _DTT_H_

#if defined(CONFIG_DTT_ADM1021)	|| \
    defined(CONFIG_DTT_ADT7460)	|| \
    defined(CONFIG_DTT_DS1621)	|| \
    defined(CONFIG_DTT_DS1775)	|| \
    defined(CONFIG_DTT_LM63)	|| \
    defined(CONFIG_DTT_LM73)	|| \
    defined(CONFIG_DTT_LM75)	|| \
    defined(CONFIG_DTT_LM81)

#define CONFIG_DTT				/* We have a DTT */

#ifndef CONFIG_DTT_ADM1021
#define DTT_COMMERCIAL_MAX_TEMP	70		/* 0 - +70 C */
#define DTT_INDUSTRIAL_MAX_TEMP	85		/* -40 - +85 C */
#define DTT_AUTOMOTIVE_MAX_TEMP	105		/* -40 - +105 C */

#ifndef CONFIG_SYS_DTT_MAX_TEMP
#define CONFIG_SYS_DTT_MAX_TEMP DTT_COMMERCIAL_MAX_TEMP
#endif

#ifndef CONFIG_SYS_DTT_HYSTERESIS
#define CONFIG_SYS_DTT_HYSTERESIS	5		/* 5 C */
#endif
#endif /* CONFIG_DTT_ADM1021 */

extern void dtt_init(void);
extern int dtt_init_one(int);
extern int dtt_read(int sensor, int reg);
extern int dtt_write(int sensor, int reg, int val);
extern int dtt_get_temp(int sensor);
#endif

#endif /* _DTT_H_ */
