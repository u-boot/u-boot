/* SPDX-License-Identifier: eCos-2.0 */
/*
 *==========================================================================
 *
 *      crc.h
 *
 *      Interface for the CRC algorithms.
 *
 *==========================================================================
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

/* 16 bit CRC with polynomial x^16+x^12+x^5+1 (CRC-CCITT) */

uint16_t crc16_ccitt(uint16_t crc_start, unsigned char *s, int len);

#endif /* _SERVICES_CRC_CRC_H_ */
