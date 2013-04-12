/* Core.h - Basic core logic functions and definitions */

/* Copyright Galileo Technology. */

/*
DESCRIPTION
This header file contains simple read/write macros for addressing
the SDRAM, devices, GT`s internal registers and PCI (using the PCI`s address
space). The macros take care of Big/Little endian conversions.
*/

#ifndef __INCcoreh
#define __INCcoreh

/* includes */
#include "gt64260R.h"
#include <stdbool.h>

extern unsigned int INTERNAL_REG_BASE_ADDR;

/*
 * GT-6426x variants
 */
#define GT_64260	0  /* includes both 64260A and 64260B */
#define GT_64261	1

#if (CONFIG_SYS_GT_6426x == GT_64260)
#ifdef CONFIG_ETHER_PORT_MII
#define GAL_ETH_DEVS 2
#else
#define GAL_ETH_DEVS 3
#endif
#elif (CONFIG_SYS_GT_6426x == GT_64261)
#define GAL_ETH_DEVS 2
#else
#define GAL_ETH_DEVS 3	/* default to a 64260 */
#endif

/****************************************/
/*          GENERAL Definitions			*/
/****************************************/

#define NO_BIT          0x00000000
#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000

#define _1K             0x00000400
#define _2K             0x00000800
#define _4K             0x00001000
#define _8K             0x00002000
#define _16K            0x00004000
#define _32K            0x00008000
#define _64K            0x00010000
#define _128K           0x00020000
#define _256K           0x00040000
#define _512K           0x00080000

#define _1M             0x00100000
#define _2M             0x00200000
#define _3M             0x00300000
#define _4M             0x00400000
#define _5M             0x00500000
#define _6M             0x00600000
#define _7M             0x00700000
#define _8M             0x00800000
#define _9M             0x00900000
#define _10M            0x00a00000
#define _11M            0x00b00000
#define _12M            0x00c00000
#define _13M            0x00d00000
#define _14M            0x00e00000
#define _15M            0x00f00000
#define _16M            0x01000000

#define _32M            0x02000000
#define _64M            0x04000000
#define _128M           0x08000000
#define _256M           0x10000000
#define _512M           0x20000000

#define _1G             0x40000000
#define _2G             0x80000000

/* Little to Big endian conversion macros */

#ifdef LE /* Little Endian */
#define SHORT_SWAP(X) (X)
#define WORD_SWAP(X) (X)
#define LONG_SWAP(X) ((l64)(X))

#else    /* Big Endian */
#define SHORT_SWAP(X) ((X <<8 ) | (X >> 8))

#define WORD_SWAP(X) (((X)&0xff)<<24)+      \
		    (((X)&0xff00)<<8)+      \
		    (((X)&0xff0000)>>8)+    \
		    (((X)&0xff000000)>>24)

#define LONG_SWAP(X) ( (l64) (((X)&0xffULL)<<56)+               \
			    (((X)&0xff00ULL)<<40)+              \
			    (((X)&0xff0000ULL)<<24)+            \
			    (((X)&0xff000000ULL)<<8)+           \
			    (((X)&0xff00000000ULL)>>8)+         \
			    (((X)&0xff0000000000ULL)>>24)+      \
			    (((X)&0xff000000000000ULL)>>40)+    \
			    (((X)&0xff00000000000000ULL)>>56))

#endif

#ifndef NULL
#define NULL 0
#endif

/* Those two definitions were defined to be compatible with MIPS */
#define NONE_CACHEABLE		0x00000000
#define CACHEABLE			0x00000000

/* 750 cache line */
#define CACHE_LINE_SIZE 32
#define CACHELINE_MASK_BITS (CACHE_LINE_SIZE - 1)
#define CACHELINE_ROUNDUP(A) (((A)+CACHELINE_MASK_BITS) & ~CACHELINE_MASK_BITS)

/* Read/Write to/from GT`s internal registers */
#define GT_REG_READ(offset, pData)                                          \
*pData = ( *((volatile unsigned int *)(NONE_CACHEABLE |                     \
		INTERNAL_REG_BASE_ADDR | (offset))) ) ;                                              \
*pData = WORD_SWAP(*pData)

#define GTREGREAD(offset)                                                   \
	 (WORD_SWAP( *((volatile unsigned int *)(NONE_CACHEABLE |            \
		   INTERNAL_REG_BASE_ADDR | (offset))) ))

#define GT_REG_WRITE(offset, data)                                          \
*((unsigned int *)( INTERNAL_REG_BASE_ADDR | (offset))) =                   \
		    WORD_SWAP(data)

/* Write 32/16/8 bit */
#define WRITE_CHAR(address, data)                                           \
	*((unsigned char *)(address)) = data
#define WRITE_SHORT(address, data)                                          \
	*((unsigned short *)(address)) = data
#define WRITE_WORD(address, data)                                           \
	*((unsigned int *)(address)) = data

/* Read 32/16/8 bits - returns data in variable. */
#define READ_CHAR(address, pData)                                           \
	*pData = *((volatile unsigned char *)(address))

#define READ_SHORT(address, pData)                                          \
	*pData = *((volatile unsigned short *)(address))

#define READ_WORD(address, pData)                                           \
	*pData = *((volatile unsigned int *)(address))

/* Read 32/16/8 bit - returns data direct. */
#define READCHAR(address)                                                   \
	*((volatile unsigned char *)((address) | NONE_CACHEABLE))

#define READSHORT(address)                                                  \
	*((volatile unsigned short *)((address) | NONE_CACHEABLE))

#define READWORD(address)                                                   \
	*((volatile unsigned int *)((address) | NONE_CACHEABLE))

/* Those two Macros were defined to be compatible with MIPS */
#define VIRTUAL_TO_PHY(x)    (((unsigned int)x) & 0xffffffff)
#define PHY_TO_VIRTUAL(x)    (((unsigned int)x) | NONE_CACHEABLE)

/*  SET_REG_BITS(regOffset,bits) -
   gets register offset and bits: a 32bit value. It set to logic '1' in the
   internal register the bits which given as an input example:
   SET_REG_BITS(0x840,BIT3 | BIT24 | BIT30) - set bits: 3,24 and 30 to logic
   '1' in register 0x840 while the other bits stays as is. */
#define SET_REG_BITS(regOffset,bits) \
	*(unsigned int*)(NONE_CACHEABLE | INTERNAL_REG_BASE_ADDR |  \
	regOffset) |= (unsigned int)WORD_SWAP(bits)

/*  RESET_REG_BITS(regOffset,bits) -
   gets register offset and bits: a 32bit value. It set to logic '0' in the
   internal register the bits which given as an input example:
   RESET_REG_BITS(0x840,BIT3 | BIT24 | BIT30) - set bits: 3,24 and 30 to logic
   '0' in register 0x840 while the other bits stays as is. */
#define RESET_REG_BITS(regOffset,bits) \
	*(unsigned int*)(NONE_CACHEABLE | INTERNAL_REG_BASE_ADDR   \
	| regOffset) &= ~( (unsigned int)WORD_SWAP(bits) )

#endif /* __INCcoreh */
