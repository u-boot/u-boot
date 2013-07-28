/** 
 * @file IxOsalEndianess.h (Obsolete file) 
 * 
 * @brief Header file for determining system endianess and OS
 * 
 * @par
 * @version $Revision: 1.1
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 */


#ifndef IxOsalEndianess_H
#define IxOsalEndianess_H

#if defined (__vxworks) || defined (__linux)

/* get ntohl/ntohs/htohl/htons macros and CPU definitions for VxWorks */
/* #include <netinet/in.h>  */

#elif defined (__wince)

/* get ntohl/ntohs/htohl/htons macros definitions for WinCE */
#include <Winsock2.h>

#else

#error Unknown OS, please add a section with the include file for htonl/htons/ntohl/ntohs

#endif /* vxworks or linux or wince */

/* Compiler specific endianness selector - WARNING this works only with arm gcc, use appropriate defines with diab */

#ifndef __wince

#if defined (__ARMEL__)

#ifndef __LITTLE_ENDIAN

#define __LITTLE_ENDIAN

#endif /* _LITTLE_ENDIAN */

#elif defined (__ARMEB__) || CPU == SIMSPARCSOLARIS

#ifndef __BIG_ENDIAN

#define __BIG_ENDIAN

#endif /* __BIG_ENDIAN */

#else

#error Error, could not identify target endianness

#endif /* endianness selector no WinCE OSes */

#else /* ndef __wince */

#define __LITTLE_ENDIAN

#endif /* def __wince */


/* OS mode selector */
#if defined (__vxworks) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_VXWORKS_LE

#elif defined (__vxworks) && defined (__BIG_ENDIAN)

#define IX_OSAL_VXWORKS_BE

#elif defined (__linux) && defined (__BIG_ENDIAN)

#define IX_OSAL_LINUX_BE

#elif defined (__linux) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_LINUX_LE

#elif defined (BOOTLOADER_BLD) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_EBOOT_LE

#elif defined (__wince) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_WINCE_LE

#else

#error Unknown OS/Endianess combination - only vxWorks BE LE, Linux BE LE, WinCE BE LE are supported

#endif /* mode selector */



#endif /* IxOsalEndianess_H */
