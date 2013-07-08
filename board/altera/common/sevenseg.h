/*
 * (C) Copyright 2003, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * common/sevenseg.h
 *
 * NIOS PIO based seven segment led support functions
 */

#ifndef __DK1S10_SEVENSEG_H__
#define __DK1S10_SEVENSEG_H__

#ifdef	CONFIG_SEVENSEG

/*
 *  15                    8 7      0
 * |-----------------------|--------|
 * |   controll value      |  value |
 * ----------------------------------
 */
#define	SEVENSEG_RAW		(int)(0)	/* write out byte value (hex) */
#define	SEVENSEG_OFF		(int)( 1 << 8)	/* display switch off */
#define	SEVENSEG_SET_DPL	(int)( 2 << 8)	/* set dp low  nibble */
#define	SEVENSEG_SET_DPH	(int)( 3 << 8)	/* set dp high nibble */
#define	SEVENSEG_RES_DPL	(int)( 4 << 8)	/* reset dp low  nibble */
#define	SEVENSEG_RES_DPH	(int)( 5 << 8)	/* reset dp high nibble */
#define	SEVENSEG_TOG_DPL	(int)( 6 << 8)	/* toggle dp low  nibble */
#define	SEVENSEG_TOG_DPH	(int)( 7 << 8)	/* toggle dp high nibble */
#define	SEVENSEG_LO		(int)( 8 << 8)	/* write out low nibble only */
#define	SEVENSEG_HI		(int)( 9 << 8)	/* write out high nibble only */
#define	SEVENSEG_STR		(int)(10 << 8)	/* write out a string */

#define	SEVENSEG_MASK_VAL	(0xff)		/* only used by SEVENSEG_RAW */
#define	SEVENSEG_MASK_CTRL	(~SEVENSEG_MASK_VAL)

#ifdef	SEVENSEG_DIGIT_HI_LO_EQUAL

#define	SEVENSEG_DIGITS_0	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_F	)
#define	SEVENSEG_DIGITS_1	(	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	)
#define	SEVENSEG_DIGITS_2	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_3	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_4	(	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_5	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_6	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_7	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	)
#define	SEVENSEG_DIGITS_8	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_9	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_A	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_B	(	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_C	(	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_D	(	SEVENSEG_DIGIT_B	\
				|	SEVENSEG_DIGIT_C	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_E	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_D	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)
#define	SEVENSEG_DIGITS_F	(	SEVENSEG_DIGIT_A	\
				|	SEVENSEG_DIGIT_E	\
				|	SEVENSEG_DIGIT_F	\
				|	SEVENSEG_DIGIT_G	)

#else	/* !SEVENSEG_DIGIT_HI_LO_EQUAL */
#error SEVENSEG: different pin asssignments not supported
#endif

void sevenseg_set(int value);

#endif	/* CONFIG_SEVENSEG */

#endif	/* __DK1S10_SEVENSEG_H__ */
