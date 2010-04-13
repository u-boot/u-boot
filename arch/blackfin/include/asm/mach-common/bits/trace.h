/*
 * Trace Unit Masks
 */

#ifndef __BFIN_PERIPHERAL_TRACE__
#define __BFIN_PERIPHERAL_TRACE__

/* Trace Buffer Control (TBUFCTL) Register Masks */
#define TBUFPWR       0x00000001
#define TBUFEN        0x00000002
#define TBUFOVF       0x00000004
#define CMPLB_SINGLE  0x00000008
#define CMPLP_DOUBLE  0x00000010
#define CMPLB         (CMPLB_SINGLE | CMPLP_DOUBLE)

/* Trace Buffer Status (TBUFSTAT) Register Masks */
#define TBUFCNT       0x0000001F

#endif
