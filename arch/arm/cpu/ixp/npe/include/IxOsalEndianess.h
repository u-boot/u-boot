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
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
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
