/** 
 * @file IxOsalMemAccess.h
 * 
 * @brief Header file for memory access
 * 
 * @par
 * @version $Revision: 1.0 $
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

#ifndef IxOsalMemAccess_H
#define IxOsalMemAccess_H


/* Global BE switch
 * 
 *  Should be set only in BE mode and only if the component uses I/O memory.
 */

#if defined (__BIG_ENDIAN)

#define IX_OSAL_BE_MAPPING

#endif /* Global switch */


/* By default only static memory maps in use;
   define IX_OSAL_DYNAMIC_MEMORY_MAP per component if dynamic maps are
   used instead in that component */
#define IX_OSAL_STATIC_MEMORY_MAP


/* 
 * SDRAM coherency mode
 * Must be defined to BE, LE_DATA_COHERENT or LE_ADDRESS_COHERENT.
 * The mode changes depending on OS 
 */
#if defined (IX_OSAL_LINUX_BE) || defined (IX_OSAL_VXWORKS_BE)

#define IX_SDRAM_BE

#elif defined (IX_OSAL_VXWORKS_LE)

#define IX_SDRAM_LE_DATA_COHERENT

#elif defined (IX_OSAL_LINUX_LE)

#define IX_SDRAM_LE_DATA_COHERENT

#elif defined (IX_OSAL_WINCE_LE)

#define IX_SDRAM_LE_DATA_COHERENT

#elif defined (IX_OSAL_EBOOT_LE)

#define IX_SDRAM_LE_ADDRESS_COHERENT

#endif




/**************************************
 * Retrieve current component mapping *
 **************************************/

/*
 * Only use customized mapping for LE. 
 * 
 */
#if defined (IX_OSAL_VXWORKS_LE) || defined (IX_OSAL_LINUX_LE) || defined (IX_OSAL_WINCE_LE) || defined (IX_OSAL_EBOOT_LE)

#include "IxOsalOsIxp400CustomizedMapping.h"

#endif


/*******************************************************************
 * Turn off IX_STATIC_MEMORY map for components using dynamic maps *
 *******************************************************************/
#ifdef IX_OSAL_DYNAMIC_MEMORY_MAP

#undef IX_OSAL_STATIC_MEMORY_MAP

#endif


/************************************************************
 * Turn off BE access for components using LE or no mapping *
 ************************************************************/

#if ( defined (IX_OSAL_LE_AC_MAPPING) || defined (IX_OSAL_LE_DC_MAPPING) || defined (IX_OSAL_NO_MAPPING) )

#undef IX_OSAL_BE_MAPPING

#endif


/*****************
 * Safety checks *
 *****************/

/* Default to no_mapping */
#if !defined (IX_OSAL_BE_MAPPING) && !defined (IX_OSAL_LE_AC_MAPPING) && !defined (IX_OSAL_LE_DC_MAPPING) && !defined (IX_OSAL_NO_MAPPING)

#define IX_OSAL_NO_MAPPING

#endif /* check at least one mapping */

/* No more than one mapping can be defined for a component */
#if   (defined (IX_OSAL_BE_MAPPING)    && defined (IX_OSAL_LE_AC_MAPPING))  \
    ||(defined (IX_OSAL_BE_MAPPING)    && defined (IX_OSAL_LE_DC_MAPPING))  \
    ||(defined (IX_OSAL_BE_MAPPING)    && defined (IX_OSAL_NO_MAPPING))     \
    ||(defined (IX_OSAL_LE_DC_MAPPING) && defined (IX_OSAL_NO_MAPPING))     \
    ||(defined (IX_OSAL_LE_DC_MAPPING) && defined (IX_OSAL_LE_AC_MAPPING))	\
    ||(defined (IX_OSAL_LE_AC_MAPPING) && defined (IX_OSAL_NO_MAPPING))


#ifdef IX_OSAL_BE_MAPPING
#warning IX_OSAL_BE_MAPPING is defined
#endif

#ifdef IX_OSAL_LE_AC_MAPPING
#warning IX_OSAL_LE_AC_MAPPING is defined
#endif

#ifdef IX_OSAL_LE_DC_MAPPING
#warning IX_OSAL_LE_DC_MAPPING is defined
#endif

#ifdef IX_OSAL_NO_MAPPING
#warning IX_OSAL_NO_MAPPING is defined
#endif

#error More than one I/O mapping is defined, please check your component mapping

#endif /* check at most one mapping */


/* Now set IX_OSAL_COMPONENT_MAPPING */

#ifdef IX_OSAL_BE_MAPPING
#define IX_OSAL_COMPONENT_MAPPING IX_OSAL_BE
#endif

#ifdef IX_OSAL_LE_AC_MAPPING
#define IX_OSAL_COMPONENT_MAPPING IX_OSAL_LE_AC
#endif

#ifdef IX_OSAL_LE_DC_MAPPING
#define IX_OSAL_COMPONENT_MAPPING IX_OSAL_LE_DC
#endif

#ifdef IX_OSAL_NO_MAPPING
#define IX_OSAL_COMPONENT_MAPPING IX_OSAL_LE
#endif


/* SDRAM coherency should be defined */
#if !defined (IX_SDRAM_BE) && !defined (IX_SDRAM_LE_DATA_COHERENT) && !defined (IX_SDRAM_LE_ADDRESS_COHERENT)

#error SDRAM coherency must be defined

#endif /* SDRAM coherency must be defined */

/* SDRAM coherency cannot be defined in several ways */
#if (defined (IX_SDRAM_BE) && (defined (IX_SDRAM_LE_DATA_COHERENT) || defined (IX_SDRAM_LE_ADDRESS_COHERENT))) \
    || (defined (IX_SDRAM_LE_DATA_COHERENT) && (defined (IX_SDRAM_BE) || defined (IX_SDRAM_LE_ADDRESS_COHERENT))) \
    || (defined (IX_SDRAM_LE_ADDRESS_COHERENT) && (defined (IX_SDRAM_BE) || defined (IX_SDRAM_LE_DATA_COHERENT)))

#error SDRAM coherency cannot be defined in more than one way

#endif /* SDRAM coherency must be defined exactly once */


/*********************
 * Read/write macros *
 *********************/

/* WARNING - except for addition of special cookie read/write macros (see below)
             these macros are NOT user serviceable. Please do not modify */

#define IX_OSAL_READ_LONG_RAW(wAddr)          (*(wAddr))
#define IX_OSAL_READ_SHORT_RAW(sAddr)         (*(sAddr))
#define IX_OSAL_READ_BYTE_RAW(bAddr)          (*(bAddr))
#define IX_OSAL_WRITE_LONG_RAW(wAddr, wData)  (*(wAddr) = (wData))
#define IX_OSAL_WRITE_SHORT_RAW(sAddr,sData)  (*(sAddr) = (sData))
#define IX_OSAL_WRITE_BYTE_RAW(bAddr, bData)  (*(bAddr) = (bData))

#ifdef __linux

/* Linux - specific cookie reads/writes. 
  Redefine per OS if dynamic memory maps are used
  and I/O memory is accessed via functions instead of raw pointer access. */

#define IX_OSAL_READ_LONG_COOKIE(wCookie)           (readl((UINT32) (wCookie) ))
#define IX_OSAL_READ_SHORT_COOKIE(sCookie)          (readw((UINT32) (sCookie) ))
#define IX_OSAL_READ_BYTE_COOKIE(bCookie)           (readb((UINT32) (bCookie) ))
#define IX_OSAL_WRITE_LONG_COOKIE(wCookie, wData)   (writel(wData, (UINT32) (wCookie) ))
#define IX_OSAL_WRITE_SHORT_COOKIE(sCookie, sData)  (writew(sData, (UINT32) (sCookie) ))
#define IX_OSAL_WRITE_BYTE_COOKIE(bCookie, bData)   (writeb(bData, (UINT32) (bCookie) ))

#endif /* linux */

#ifdef __wince

/* WinCE - specific cookie reads/writes. */

static __inline__ UINT32
ixOsalWinCEReadLCookie (volatile UINT32 * lCookie)
{
    return *lCookie;
}

static __inline__ UINT16
ixOsalWinCEReadWCookie (volatile UINT16 * wCookie)
{
#if 0
    UINT32 auxVal = *((volatile UINT32 *) wCookie);
    if ((unsigned) wCookie & 3)
	return (UINT16) (auxVal >> 16);
    else
	return (UINT16) (auxVal & 0xffff);
#else
    return *wCookie;
#endif
}

static __inline__ UINT8
ixOsalWinCEReadBCookie (volatile UINT8 * bCookie)
{
#if 0
    UINT32 auxVal = *((volatile UINT32 *) bCookie);
    return (UINT8) ((auxVal >> (3 - (((unsigned) bCookie & 3) << 3)) & 0xff));
#else
    return *bCookie;
#endif
}

static __inline__ void
ixOsalWinCEWriteLCookie (volatile UINT32 * lCookie, UINT32 lVal)
{
    *lCookie = lVal;
}

static __inline__ void
ixOsalWinCEWriteWCookie (volatile UINT16 * wCookie, UINT16 wVal)
{
#if 0
    volatile UINT32 *auxCookie =
	(volatile UINT32 *) ((unsigned) wCookie & ~3);
    if ((unsigned) wCookie & 3)
    {
	*auxCookie &= 0xffff;
	*auxCookie |= (UINT32) wVal << 16;
    }
    else
    {
	*auxCookie &= ~0xffff;
	*auxCookie |= (UINT32) wVal & 0xffff;
    }
#else
    *wCookie = wVal;
#endif
}

static __inline__ void
ixOsalWinCEWriteBCookie (volatile UINT8 * bCookie, UINT8 bVal)
{
#if 0
    volatile UINT32 *auxCookie =
	(volatile UINT32 *) ((unsigned) bCookie & ~3);
    *auxCookie &= 0xff << (3 - (((unsigned) bCookie & 3) << 3));
    *auxCookie |= (UINT32) bVal << (3 - (((unsigned) bCookie & 3) << 3));
#else
    *bCookie = bVal;
#endif
}


#define IX_OSAL_READ_LONG_COOKIE(wCookie)           (ixOsalWinCEReadLCookie(wCookie))
#define IX_OSAL_READ_SHORT_COOKIE(sCookie)          (ixOsalWinCEReadWCookie(sCookie))
#define IX_OSAL_READ_BYTE_COOKIE(bCookie)           (ixOsalWinCEReadBCookie(bCookie))
#define IX_OSAL_WRITE_LONG_COOKIE(wCookie, wData)   (ixOsalWinCEWriteLCookie(wCookie, wData))
#define IX_OSAL_WRITE_SHORT_COOKIE(sCookie, sData)  (ixOsalWinCEWriteWCookie(sCookie, sData))
#define IX_OSAL_WRITE_BYTE_COOKIE(bCookie, bData)   (ixOsalWinCEWriteBCookie(bCookie, bData))

#endif /* wince */

#if defined (__vxworks) || (defined (__linux) && defined (IX_OSAL_STATIC_MEMORY_MAP)) || \
                           (defined (__wince) && defined (IX_OSAL_STATIC_MEMORY_MAP))

#define IX_OSAL_READ_LONG_IO(wAddr)            IX_OSAL_READ_LONG_RAW(wAddr)
#define IX_OSAL_READ_SHORT_IO(sAddr)           IX_OSAL_READ_SHORT_RAW(sAddr)
#define IX_OSAL_READ_BYTE_IO(bAddr)            IX_OSAL_READ_BYTE_RAW(bAddr)
#define IX_OSAL_WRITE_LONG_IO(wAddr, wData)    IX_OSAL_WRITE_LONG_RAW(wAddr, wData)
#define IX_OSAL_WRITE_SHORT_IO(sAddr, sData)   IX_OSAL_WRITE_SHORT_RAW(sAddr, sData)
#define IX_OSAL_WRITE_BYTE_IO(bAddr, bData)    IX_OSAL_WRITE_BYTE_RAW(bAddr, bData)

#elif (defined (__linux) && !defined (IX_OSAL_STATIC_MEMORY_MAP)) || \
      (defined (__wince) && !defined (IX_OSAL_STATIC_MEMORY_MAP))

#ifndef __wince
#include <asm/io.h>
#endif /* ndef __wince */

#define IX_OSAL_READ_LONG_IO(wAddr)            IX_OSAL_READ_LONG_COOKIE(wAddr)
#define IX_OSAL_READ_SHORT_IO(sAddr)           IX_OSAL_READ_SHORT_COOKIE(sAddr)
#define IX_OSAL_READ_BYTE_IO(bAddr)            IX_OSAL_READ_BYTE_COOKIE(bAddr)
#define IX_OSAL_WRITE_LONG_IO(wAddr, wData)    IX_OSAL_WRITE_LONG_COOKIE(wAddr, wData)
#define IX_OSAL_WRITE_SHORT_IO(sAddr, sData)   IX_OSAL_WRITE_SHORT_COOKIE(sAddr, sData)
#define IX_OSAL_WRITE_BYTE_IO(bAddr, bData)    IX_OSAL_WRITE_BYTE_COOKIE(bAddr, bData)

#endif

/* Define BE macros */
#define IX_OSAL_READ_LONG_BE(wAddr)          IX_OSAL_BE_BUSTOXSL(IX_OSAL_READ_LONG_IO((volatile UINT32 *) (wAddr) ))
#define IX_OSAL_READ_SHORT_BE(sAddr)         IX_OSAL_BE_BUSTOXSS(IX_OSAL_READ_SHORT_IO((volatile UINT16 *) (sAddr) ))
#define IX_OSAL_READ_BYTE_BE(bAddr)          IX_OSAL_BE_BUSTOXSB(IX_OSAL_READ_BYTE_IO((volatile UINT8 *) (bAddr) ))
#define IX_OSAL_WRITE_LONG_BE(wAddr, wData)  IX_OSAL_WRITE_LONG_IO((volatile UINT32 *) (wAddr), IX_OSAL_BE_XSTOBUSL((UINT32) (wData) ))
#define IX_OSAL_WRITE_SHORT_BE(sAddr, sData) IX_OSAL_WRITE_SHORT_IO((volatile UINT16 *) (sAddr), IX_OSAL_BE_XSTOBUSS((UINT16) (sData) ))
#define IX_OSAL_WRITE_BYTE_BE(bAddr, bData)  IX_OSAL_WRITE_BYTE_IO((volatile UINT8 *) (bAddr), IX_OSAL_BE_XSTOBUSB((UINT8) (bData) ))

/* Define LE AC macros */
#define IX_OSAL_READ_LONG_LE_AC(wAddr)          IX_OSAL_READ_LONG_IO((volatile UINT32 *) IX_OSAL_LE_AC_BUSTOXSL((UINT32) (wAddr) ))
#define IX_OSAL_READ_SHORT_LE_AC(sAddr)         IX_OSAL_READ_SHORT_IO((volatile UINT16 *) IX_OSAL_LE_AC_BUSTOXSS((UINT32) (sAddr) ))
#define IX_OSAL_READ_BYTE_LE_AC(bAddr)          IX_OSAL_READ_BYTE_IO((volatile UINT8 *) IX_OSAL_LE_AC_BUSTOXSB((UINT32) (bAddr) ))
#define IX_OSAL_WRITE_LONG_LE_AC(wAddr, wData)  IX_OSAL_WRITE_LONG_IO((volatile UINT32 *) IX_OSAL_LE_AC_XSTOBUSL((UINT32) (wAddr) ), (UINT32) (wData))
#define IX_OSAL_WRITE_SHORT_LE_AC(sAddr, sData) IX_OSAL_WRITE_SHORT_IO((volatile UINT16 *) IX_OSAL_LE_AC_XSTOBUSS((UINT32) (sAddr) ), (UINT16) (sData))
#define IX_OSAL_WRITE_BYTE_LE_AC(bAddr, bData)  IX_OSAL_WRITE_BYTE_IO((volatile UINT8 *) IX_OSAL_LE_AC_XSTOBUSB((UINT32) (bAddr) ), (UINT8) (bData))


/* Inline functions are required here to avoid reading the same I/O location 2 or 4 times for the byte swap */
static __inline__ UINT32
ixOsalDataCoherentLongReadSwap (volatile UINT32 * wAddr)
{
    UINT32 wData = IX_OSAL_READ_LONG_IO (wAddr);
    return IX_OSAL_LE_DC_BUSTOXSL (wData);
}

static __inline__ UINT16
ixOsalDataCoherentShortReadSwap (volatile UINT16 * sAddr)
{
    UINT16 sData = IX_OSAL_READ_SHORT_IO (sAddr);
    return IX_OSAL_LE_DC_BUSTOXSS (sData);
}

static __inline__ void
ixOsalDataCoherentLongWriteSwap (volatile UINT32 * wAddr, UINT32 wData)
{
    wData = IX_OSAL_LE_DC_XSTOBUSL (wData);
    IX_OSAL_WRITE_LONG_IO (wAddr, wData);
}

static __inline__ void
ixOsalDataCoherentShortWriteSwap (volatile UINT16 * sAddr, UINT16 sData)
{
    sData = IX_OSAL_LE_DC_XSTOBUSS (sData);
    IX_OSAL_WRITE_SHORT_IO (sAddr, sData);
}

/* Define LE DC macros */

#define IX_OSAL_READ_LONG_LE_DC(wAddr)          ixOsalDataCoherentLongReadSwap((volatile UINT32 *) (wAddr) )
#define IX_OSAL_READ_SHORT_LE_DC(sAddr)         ixOsalDataCoherentShortReadSwap((volatile UINT16 *) (sAddr) )
#define IX_OSAL_READ_BYTE_LE_DC(bAddr)          IX_OSAL_LE_DC_BUSTOXSB(IX_OSAL_READ_BYTE_IO((volatile UINT8 *) (bAddr) ))
#define IX_OSAL_WRITE_LONG_LE_DC(wAddr, wData)  ixOsalDataCoherentLongWriteSwap((volatile UINT32 *) (wAddr), (UINT32) (wData))
#define IX_OSAL_WRITE_SHORT_LE_DC(sAddr, sData) ixOsalDataCoherentShortWriteSwap((volatile UINT16 *) (sAddr), (UINT16) (sData))
#define IX_OSAL_WRITE_BYTE_LE_DC(bAddr, bData)  IX_OSAL_WRITE_BYTE_IO((volatile UINT8 *) (bAddr), IX_OSAL_LE_DC_XSTOBUSB((UINT8) (bData)))

#if defined (IX_OSAL_BE_MAPPING)

#define IX_OSAL_READ_LONG(wAddr)            IX_OSAL_READ_LONG_BE(wAddr) 
#define IX_OSAL_READ_SHORT(sAddr)	        IX_OSAL_READ_SHORT_BE(sAddr) 
#define IX_OSAL_READ_BYTE(bAddr)	        IX_OSAL_READ_BYTE_BE(bAddr) 
#define IX_OSAL_WRITE_LONG(wAddr, wData) 	IX_OSAL_WRITE_LONG_BE(wAddr, wData)
#define IX_OSAL_WRITE_SHORT(sAddr, sData)	IX_OSAL_WRITE_SHORT_BE(sAddr, sData)
#define IX_OSAL_WRITE_BYTE(bAddr, bData)	IX_OSAL_WRITE_BYTE_BE(bAddr, bData)

#elif defined (IX_OSAL_LE_AC_MAPPING)

#define IX_OSAL_READ_LONG(wAddr)            IX_OSAL_READ_LONG_LE_AC(wAddr) 
#define IX_OSAL_READ_SHORT(sAddr)	        IX_OSAL_READ_SHORT_LE_AC(sAddr) 
#define IX_OSAL_READ_BYTE(bAddr)	        IX_OSAL_READ_BYTE_LE_AC(bAddr) 
#define IX_OSAL_WRITE_LONG(wAddr, wData) 	IX_OSAL_WRITE_LONG_LE_AC(wAddr, wData)
#define IX_OSAL_WRITE_SHORT(sAddr, sData)	IX_OSAL_WRITE_SHORT_LE_AC(sAddr, sData)
#define IX_OSAL_WRITE_BYTE(bAddr, bData)	IX_OSAL_WRITE_BYTE_LE_AC(bAddr, bData)

#elif defined (IX_OSAL_LE_DC_MAPPING)

#define IX_OSAL_READ_LONG(wAddr)            IX_OSAL_READ_LONG_LE_DC(wAddr) 
#define IX_OSAL_READ_SHORT(sAddr)	        IX_OSAL_READ_SHORT_LE_DC(sAddr) 
#define IX_OSAL_READ_BYTE(bAddr)	        IX_OSAL_READ_BYTE_LE_DC(bAddr) 
#define IX_OSAL_WRITE_LONG(wAddr, wData) 	IX_OSAL_WRITE_LONG_LE_DC(wAddr, wData)
#define IX_OSAL_WRITE_SHORT(sAddr, sData)	IX_OSAL_WRITE_SHORT_LE_DC(sAddr, sData)
#define IX_OSAL_WRITE_BYTE(bAddr, bData)	IX_OSAL_WRITE_BYTE_LE_DC(bAddr, bData)

#endif   /* End of BE and LE coherency mode switch */


/* Reads/writes to and from memory shared with NPEs - depends on the SDRAM coherency */

#if defined (IX_SDRAM_BE)

#define IX_OSAL_READ_BE_SHARED_LONG(wAddr)            IX_OSAL_READ_LONG_RAW(wAddr)
#define IX_OSAL_READ_BE_SHARED_SHORT(sAddr)           IX_OSAL_READ_SHORT_RAW(sAddr)
#define IX_OSAL_READ_BE_SHARED_BYTE(bAddr)            IX_OSAL_READ_BYTE_RAW(bAddr)

#define IX_OSAL_WRITE_BE_SHARED_LONG(wAddr, wData)    IX_OSAL_WRITE_LONG_RAW(wAddr, wData)
#define IX_OSAL_WRITE_BE_SHARED_SHORT(sAddr, sData)   IX_OSAL_WRITE_SHORT_RAW(sAddr, sData)
#define IX_OSAL_WRITE_BE_SHARED_BYTE(bAddr, bData)    IX_OSAL_WRITE_BYTE_RAW(bAddr, bData)

#define IX_OSAL_SWAP_BE_SHARED_LONG(wData)            (wData)
#define IX_OSAL_SWAP_BE_SHARED_SHORT(sData)           (sData)
#define IX_OSAL_SWAP_BE_SHARED_BYTE(bData)            (bData)

#elif defined (IX_SDRAM_LE_ADDRESS_COHERENT)

#define IX_OSAL_READ_BE_SHARED_LONG(wAddr)            IX_OSAL_READ_LONG_RAW(wAddr)
#define IX_OSAL_READ_BE_SHARED_SHORT(sAddr)           IX_OSAL_READ_SHORT_RAW(IX_OSAL_SWAP_SHORT_ADDRESS(sAddr))
#define IX_OSAL_READ_BE_SHARED_BYTE(bAddr)            IX_OSAL_READ_BYTE_RAW(IX_OSAL_SWAP_BYTE_ADDRESS(bAddr))

#define IX_OSAL_WRITE_BE_SHARED_LONG(wAddr, wData)    IX_OSAL_WRITE_LONG_RAW(wAddr, wData)
#define IX_OSAL_WRITE_BE_SHARED_SHORT(sAddr, sData)   IX_OSAL_WRITE_SHORT_RAW(IX_OSAL_SWAP_SHORT_ADDRESS(sAddr), sData)
#define IX_OSAL_WRITE_BE_SHARED_BYTE(bAddr, bData)    IX_OSAL_WRITE_BYTE_RAW(IX_OSAL_SWAP_BYTE_ADDRESS(bAddr), bData)

#define IX_OSAL_SWAP_BE_SHARED_LONG(wData)            (wData)
#define IX_OSAL_SWAP_BE_SHARED_SHORT(sData)           (sData)
#define IX_OSAL_SWAP_BE_SHARED_BYTE(bData)            (bData)

#elif defined (IX_SDRAM_LE_DATA_COHERENT)

#define IX_OSAL_READ_BE_SHARED_LONG(wAddr)            IX_OSAL_SWAP_LONG(IX_OSAL_READ_LONG_RAW(wAddr))
#define IX_OSAL_READ_BE_SHARED_SHORT(sAddr)           IX_OSAL_SWAP_SHORT(IX_OSAL_READ_SHORT_RAW(sAddr))
#define IX_OSAL_READ_BE_SHARED_BYTE(bAddr)            IX_OSAL_READ_BYTE_RAW(bAddr)

#define IX_OSAL_WRITE_BE_SHARED_LONG(wAddr, wData)    IX_OSAL_WRITE_LONG_RAW(wAddr, IX_OSAL_SWAP_LONG(wData))
#define IX_OSAL_WRITE_BE_SHARED_SHORT(sAddr, sData)   IX_OSAL_WRITE_SHORT_RAW(sAddr, IX_OSAL_SWAP_SHORT(sData))
#define IX_OSAL_WRITE_BE_SHARED_BYTE(bAddr, bData)    IX_OSAL_WRITE_BYTE_RAW(bAddr, bData)

#define IX_OSAL_SWAP_BE_SHARED_LONG(wData)            IX_OSAL_SWAP_LONG(wData)
#define IX_OSAL_SWAP_BE_SHARED_SHORT(sData)           IX_OSAL_SWAP_SHORT(sData)

#endif


#define IX_OSAL_COPY_BE_SHARED_LONG_ARRAY(wDestAddr, wSrcAddr, wCount) \
  { \
    UINT32 i; \
    \
    for ( i = 0 ; i < wCount ; i++ ) \
    { \
      * (((UINT32 *) wDestAddr) + i) = IX_OSAL_READ_BE_SHARED_LONG(((UINT32 *) wSrcAddr) + i); \
    }; \
  };

#endif /* IxOsalMemAccess_H */
