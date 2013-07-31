/*
 *==========================================================================
 *
 *      crc.h
 *
 *      Interface for the CRC algorithms.
 *
 *==========================================================================
 * SPDX-License-Identifier:	eCos-2.0
 *==========================================================================
 *#####DESCRIPTIONBEGIN####
 *
 * Author(s):    Andrew Lunn
 * Contributors: Andrew Lunn
 * Date:         2002-08-06
 * Purpose:
 * Description:
 *
 * This code is part of eCos (tm).
 *
 *####DESCRIPTIONEND####
 *
 *==========================================================================
 */

#ifndef _SERVICES_CRC_CRC_H_
#define _SERVICES_CRC_CRC_H_

#include <linux/types.h>

#ifndef __externC
# ifdef __cplusplus
#  define __externC extern "C"
# else
#  define __externC extern
# endif
#endif

/* Compute a CRC, using the POSIX 1003 definition */
extern uint32_t
cyg_posix_crc32(unsigned char *s, int len);

/* Gary S. Brown's 32 bit CRC */

extern uint32_t
cyg_crc32(unsigned char *s, int len);

/* Gary S. Brown's 32 bit CRC, but accumulate the result from a */
/* previous CRC calculation */

extern uint32_t
cyg_crc32_accumulate(uint32_t crc, unsigned char *s, int len);

/* Ethernet FCS Algorithm */

extern uint32_t
cyg_ether_crc32(unsigned char *s, int len);

/* Ethernet FCS algorithm, but accumulate the result from a previous */
/* CRC calculation. */

extern uint32_t
cyg_ether_crc32_accumulate(uint32_t crc, unsigned char *s, int len);

/* 16 bit CRC with polynomial x^16+x^12+x^5+1 */

extern uint16_t cyg_crc16(unsigned char *s, int len);

#endif /* _SERVICES_CRC_CRC_H_ */
