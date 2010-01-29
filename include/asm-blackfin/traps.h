/*
 *  Copyright 2004-2009 Analog Devices Inc.
 *                 2001 Lineo, Inc
 *                        Tony Kou
 *                 1993 Hamish Macdonald
 *
 * Licensed under the GPL-2
 */

#ifndef _BFIN_TRAPS_H
#define _BFIN_TRAPS_H

#define VEC_SYS		(0)
#define VEC_EXCPT01	(1)
#define VEC_EXCPT02	(2)
#define VEC_EXCPT03	(3)
#define VEC_EXCPT04	(4)
#define VEC_EXCPT05	(5)
#define VEC_EXCPT06	(6)
#define VEC_EXCPT07	(7)
#define VEC_EXCPT08	(8)
#define VEC_EXCPT09	(9)
#define VEC_EXCPT10	(10)
#define VEC_EXCPT11	(11)
#define VEC_EXCPT12	(12)
#define VEC_EXCPT13	(13)
#define VEC_EXCPT14	(14)
#define VEC_EXCPT15	(15)
#define VEC_STEP	(16)
#define VEC_OVFLOW	(17)
#define VEC_UNDEF_I	(33)
#define VEC_ILGAL_I	(34)
#define VEC_CPLB_VL	(35)
#define VEC_MISALI_D	(36)
#define VEC_UNCOV	(37)
#define VEC_CPLB_M	(38)
#define VEC_CPLB_MHIT	(39)
#define VEC_WATCH	(40)
#define VEC_ISTRU_VL	(41)	/*ADSP-BF535 only (MH) */
#define VEC_MISALI_I	(42)
#define VEC_CPLB_I_VL	(43)
#define VEC_CPLB_I_M	(44)
#define VEC_CPLB_I_MHIT	(45)
#define VEC_ILL_RES	(46)	/* including unvalid supervisor mode insn */
/* The hardware reserves (63) for future use - we use it to tell our
 * normal exception handling code we have a hardware error
 */
#define VEC_HWERR	(63)

#endif				/* _BFIN_TRAPS_H */
