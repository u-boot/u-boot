/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*               Heavily based on code copyright (C) Richard Gooch
*
* Language:     ANSI C
* Environment:  32-bit Ring 0 device driver
*
* Description:  Generic Memory Type Range Register (MTRR) functions to
*               manipulate the MTRR registers on supported CPU's. This code
*               *must* run at ring 0, so you can't normally include this
*               code directly in normal applications (the except is DOS4GW
*               apps which run at ring 0 under real DOS). Thus this code
*               will normally be compiled into a ring 0 device driver for
*               the target operating system.
*
****************************************************************************/

#include "pmapi.h"
#include "ztimerc.h"
#include "mtrr.h"

#ifndef REALMODE

/*--------------------------- Global variables ----------------------------*/

/* Intel pre-defined MTRR registers */

#define NUM_FIXED_RANGES        88
#define INTEL_cap_MSR           0x0FE
#define INTEL_defType_MSR       0x2FF
#define INTEL_fix64K_00000_MSR  0x250
#define INTEL_fix16K_80000_MSR  0x258
#define INTEL_fix16K_A0000_MSR  0x259
#define INTEL_fix4K_C0000_MSR   0x268
#define INTEL_fix4K_C8000_MSR   0x269
#define INTEL_fix4K_D0000_MSR   0x26A
#define INTEL_fix4K_D8000_MSR   0x26B
#define INTEL_fix4K_E0000_MSR   0x26C
#define INTEL_fix4K_E8000_MSR   0x26D
#define INTEL_fix4K_F0000_MSR   0x26E
#define INTEL_fix4K_F8000_MSR   0x26F

/* Macros to find the address of a paricular MSR register */

#define INTEL_physBase_MSR(reg) (0x200 + 2 * (reg))
#define INTEL_physMask_MSR(reg) (0x200 + 2 * (reg) + 1)

/* Cyrix CPU configuration register indexes */
#define CX86_CCR0 0xC0
#define CX86_CCR1 0xC1
#define CX86_CCR2 0xC2
#define CX86_CCR3 0xC3
#define CX86_CCR4 0xE8
#define CX86_CCR5 0xE9
#define CX86_CCR6 0xEA
#define CX86_DIR0 0xFE
#define CX86_DIR1 0xFF
#define CX86_ARR_BASE 0xC4
#define CX86_RCR_BASE 0xDC

/* Structure to maintain machine state while updating MTRR registers */

typedef struct {
    ulong   flags;
    ulong   defTypeLo;
    ulong   defTypeHi;
    ulong   cr4Val;
    ulong   ccr3;
    } MTRRContext;

static int      numMTRR = -1;
static int      cpuFamily,cpuType,cpuStepping;
static void     (*getMTRR)(uint reg,ulong *base,ulong *size,int *type) = NULL;
static void     (*setMTRR)(uint reg,ulong base,ulong size,int type) = NULL;
static int      (*getFreeRegion)(ulong base,ulong size) = NULL;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
RETURNS:
Returns non-zero if we have the write-combining memory type
****************************************************************************/
static int MTRR_haveWriteCombine(void)
{
    ulong   config,dummy;

    switch (cpuFamily) {
	case CPU_AMD:
	    if (cpuType < CPU_AMDAthlon) {
		/* AMD K6-2 stepping 8 and later support the MTRR registers.
		 * The earlier K6-2 steppings (300Mhz models) do not
		 * support MTRR's.
		 */
		if ((cpuType < CPU_AMDK6_2) || (cpuType == CPU_AMDK6_2 && cpuStepping < 8))
		    return 0;
		return 1;
		}
	    /* Fall through for AMD Athlon which uses P6 style MTRR's */
	case CPU_Intel:
	    _MTRR_readMSR(INTEL_cap_MSR,&config,&dummy);
	    return (config & (1 << 10));
	case CPU_Cyrix:
	    /* Cyrix 6x86 and later support the MTRR registers */
	    if (cpuType < CPU_Cyrix6x86)
		return 0;
	    return 1;
	}
    return 0;
}

/****************************************************************************
PARAMETERS:
base    - The starting physical base address of the region
size    - The size in bytes of the region

RETURNS:
The index of the region on success, else -1 on error.

REMARKS:
Generic function to find the location of a free MTRR register to be used
for creating a new mapping.
****************************************************************************/
static int GENERIC_getFreeRegion(
    ulong base,
    ulong size)
{
    int     i,ltype;
    ulong   lbase,lsize;

    for (i = 0; i < numMTRR; i++) {
	getMTRR(i,&lbase,&lsize,&ltype);
	if (lsize < 1)
	    return i;
	}
    (void)base;
    (void)size;
    return -1;
}

/****************************************************************************
PARAMETERS:
base    - The starting physical base address of the region
size    - The size in bytes of the region

RETURNS:
The index of the region on success, else -1 on error.

REMARKS:
Generic function to find the location of a free MTRR register to be used
for creating a new mapping.
****************************************************************************/
static int AMDK6_getFreeRegion(
    ulong base,
    ulong size)
{
    int     i,ltype;
    ulong   lbase,lsize;

    for (i = 0; i < numMTRR; i++) {
	getMTRR(i,&lbase,&lsize,&ltype);
	if (lsize < 1)
	    return i;
	}
    (void)base;
    (void)size;
    return -1;
}

/****************************************************************************
PARAMETERS:
base    - The starting physical base address of the region
size    - The size in bytes of the region

RETURNS:
The index of the region on success, else -1 on error.

REMARKS:
Cyrix specific function to find the location of a free MTRR register to be
used for creating a new mapping.
****************************************************************************/
static int CYRIX_getFreeRegion(
    ulong base,
    ulong size)
{
    int     i,ltype;
    ulong   lbase, lsize;

    if (size > 0x2000000UL) {
	/* If we are to set up a region >32M then look at ARR7 immediately */
	getMTRR(7,&lbase,&lsize,&ltype);
	if (lsize < 1)
	    return 7;
	}
    else {
	/* Check ARR0-6 registers */
	for (i = 0; i < 7; i++) {
	    getMTRR(i,&lbase,&lsize,&ltype);
	    if (lsize < 1)
		return i;
	    }
	/* Try ARR7 but its size must be at least 256K */
	getMTRR(7,&lbase,&lsize,&ltype);
	if ((lsize < 1) && (size >= 0x40000))
	    return i;
	}
    (void)base;
    return -1;
}

/****************************************************************************
PARAMETERS:
c   - Place to store the machine context across the call

REMARKS:
Puts the processor into a state where MTRRs can be safely updated
****************************************************************************/
static void MTRR_beginUpdate(
    MTRRContext *c)
{
    c->flags = _MTRR_disableInt();
    if (cpuFamily != CPU_AMD || (cpuFamily == CPU_AMD && cpuType >= CPU_AMDAthlon)) {
	switch (cpuFamily) {
	    case CPU_Intel:
	    case CPU_AMD:
		/* Disable MTRRs, and set the default type to uncached */
		c->cr4Val = _MTRR_saveCR4();
		_MTRR_readMSR(INTEL_defType_MSR,&c->defTypeLo,&c->defTypeHi);
		_MTRR_writeMSR(INTEL_defType_MSR,c->defTypeLo & 0xF300UL,c->defTypeHi);
		break;
	    case CPU_Cyrix:
		c->ccr3 = _MTRR_getCx86(CX86_CCR3);
		_MTRR_setCx86(CX86_CCR3, (uchar)((c->ccr3 & 0x0F) | 0x10));
		break;
	    }
	}
}

/****************************************************************************
PARAMETERS:
c   - Place to restore the machine context from

REMARKS:
Restores the processor after updating any of the registers
****************************************************************************/
static void MTRR_endUpdate(
    MTRRContext *c)
{
    if (cpuFamily != CPU_AMD || (cpuFamily == CPU_AMD && cpuType >= CPU_AMDAthlon)) {
	PM_flushTLB();
	switch (cpuFamily) {
	    case CPU_Intel:
	    case CPU_AMD:
		_MTRR_writeMSR(INTEL_defType_MSR,c->defTypeLo,c->defTypeHi);
		_MTRR_restoreCR4(c->cr4Val);
		break;
	    case CPU_Cyrix:
		_MTRR_setCx86(CX86_CCR3,(uchar)c->ccr3);
		break;
	    }
	}

    /* Re-enable interrupts (if enabled previously) */
    _MTRR_restoreInt(c->flags);
}

/****************************************************************************
PARAMETERS:
reg     - MTRR register to read
base    - Place to store the starting physical base address of the region
size    - Place to store the size in bytes of the region
type    - Place to store the type of the MTRR register

REMARKS:
Intel specific function to read the value of a specific MTRR register.
****************************************************************************/
static void INTEL_getMTRR(
    uint reg,
    ulong *base,
    ulong *size,
    int *type)
{
    ulong hi,maskLo,baseLo;

    _MTRR_readMSR(INTEL_physMask_MSR(reg),&maskLo,&hi);
    if ((maskLo & 0x800) == 0) {
	/* MTRR is disabled, so it is free */
	*base = 0;
	*size = 0;
	*type = 0;
	return;
	}
    _MTRR_readMSR(INTEL_physBase_MSR(reg),&baseLo,&hi);
    maskLo = (maskLo & 0xFFFFF000UL);
    *size = ~(maskLo - 1);
    *base = (baseLo & 0xFFFFF000UL);
    *type = (baseLo & 0xFF);
}

/****************************************************************************
PARAMETERS:
reg     - MTRR register to set
base    - The starting physical base address of the region
size    - The size in bytes of the region
type    - Type to place into the MTRR register

REMARKS:
Intel specific function to set the value of a specific MTRR register to
the passed in base, size and type.
****************************************************************************/
static void INTEL_setMTRR(
    uint reg,
    ulong base,
    ulong size,
    int type)
{
    MTRRContext c;

    MTRR_beginUpdate(&c);
    if (size == 0) {
	/* The invalid bit is kept in the mask, so we simply clear the
	 * relevant mask register to disable a range.
	 */
	_MTRR_writeMSR(INTEL_physMask_MSR(reg),0,0);
	}
    else {
	_MTRR_writeMSR(INTEL_physBase_MSR(reg),base | type,0);
	_MTRR_writeMSR(INTEL_physMask_MSR(reg),~(size - 1) | 0x800,0);
	}
    MTRR_endUpdate(&c);
}

/****************************************************************************
REMARKS:
Disabled banked write combing for Intel processors. We always disable this
because it invariably causes problems with older hardware.
****************************************************************************/
static void INTEL_disableBankedWriteCombine(void)
{
    MTRRContext c;

    MTRR_beginUpdate(&c);
    _MTRR_writeMSR(INTEL_fix16K_A0000_MSR,0,0);
    MTRR_endUpdate(&c);
}

/****************************************************************************
PARAMETERS:
reg     - MTRR register to set
base    - The starting physical base address of the region
size    - The size in bytes of the region
type    - Type to place into the MTRR register

REMARKS:
Intel specific function to set the value of a specific MTRR register to
the passed in base, size and type.
****************************************************************************/
static void AMD_getMTRR(
    uint reg,
    ulong *base,
    ulong *size,
    int *type)
{
    ulong   low,high;

    /*  Upper dword is region 1, lower is region 0  */
    _MTRR_readMSR(0xC0000085, &low, &high);
    if (reg == 1)
	low = high;

    /* Find the base and type for the region */
    *base = low & 0xFFFE0000;
    *type = 0;
    if (low & 1)
	*type = PM_MTRR_UNCACHABLE;
    if (low & 2)
	*type = PM_MTRR_WRCOMB;
    if ((low & 3) == 0) {
	*size = 0;
	return;
	}

    /* This needs a little explaining. The size is stored as an
     * inverted mask of bits of 128K granularity 15 bits long offset
     * 2 bits
     *
     * So to get a size we do invert the mask and add 1 to the lowest
     * mask bit (4 as its 2 bits in). This gives us a size we then shift
     * to turn into 128K blocks
     *
     *  eg              111 1111 1111 1100      is 512K
     *
     *  invert          000 0000 0000 0011
     *  +1              000 0000 0000 0100
     *  *128K   ...
     */
    low = (~low) & 0x0FFFC;
    *size = (low + 4) << 15;
}

/****************************************************************************
PARAMETERS:
reg     - MTRR register to set
base    - The starting physical base address of the region
size    - The size in bytes of the region
type    - Type to place into the MTRR register

REMARKS:
Intel specific function to set the value of a specific MTRR register to
the passed in base, size and type.
****************************************************************************/
static void AMD_setMTRR(
    uint reg,
    ulong base,
    ulong size,
    int type)
{
    ulong       low,high,newVal;
    MTRRContext c;

    MTRR_beginUpdate(&c);
    _MTRR_readMSR(0xC0000085, &low, &high);
    if (size == 0) {
	/* Clear register to disable */
	if (reg)
	    high = 0;
	else
	    low = 0;
	}
    else {
	/* Set the register to the base (already shifted for us), the
	 * type (off by one) and an inverted bitmask of the size
	 * The size is the only odd bit. We are fed say 512K
	 * We invert this and we get 111 1111 1111 1011 but
	 * if you subtract one and invert you get the desired
	 * 111 1111 1111 1100 mask
	 */
	newVal = (((~(size-1)) >> 15) & 0x0001FFFC) | base | (type+1);
	if (reg)
	    high = newVal;
	else
	    low = newVal;
	}

    /* The writeback rule is quite specific. See the manual. Its
     * disable local interrupts, write back the cache, set the MTRR
     */
    PM_flushTLB();
    _MTRR_writeMSR(0xC0000085, low, high);
    MTRR_endUpdate(&c);
}

/****************************************************************************
PARAMETERS:
reg     - MTRR register to set
base    - The starting physical base address of the region
size    - The size in bytes of the region
type    - Type to place into the MTRR register

REMARKS:
Intel specific function to set the value of a specific MTRR register to
the passed in base, size and type.
****************************************************************************/
static void CYRIX_getMTRR(
    uint reg,
    ulong *base,
    ulong *size,
    int *type)
{
    MTRRContext c;
    uchar       arr = CX86_ARR_BASE + reg*3;
    uchar       rcr,shift;

    /* Save flags and disable interrupts */
    MTRR_beginUpdate(&c);
    ((uchar*)base)[3]  = _MTRR_getCx86(arr);
    ((uchar*)base)[2]  = _MTRR_getCx86((uchar)(arr+1));
    ((uchar*)base)[1]  = _MTRR_getCx86((uchar)(arr+2));
    rcr = _MTRR_getCx86((uchar)(CX86_RCR_BASE + reg));
    MTRR_endUpdate(&c);

    /* Enable interrupts if it was enabled previously */
    shift = ((uchar*)base)[1] & 0x0f;
    *base &= 0xFFFFF000UL;

    /* Power of two, at least 4K on ARR0-ARR6, 256K on ARR7
     * Note: shift==0xF means 4G, this is unsupported.
     */
    if (shift)
	*size = (reg < 7 ? 0x800UL : 0x20000UL) << shift;
    else
	*size = 0;

    /* Bit 0 is Cache Enable on ARR7, Cache Disable on ARR0-ARR6 */
    if (reg < 7) {
	switch (rcr) {
	    case  1: *type = PM_MTRR_UNCACHABLE; break;
	    case  8: *type = PM_MTRR_WRBACK;     break;
	    case  9: *type = PM_MTRR_WRCOMB;     break;
	    case 24:
	    default: *type = PM_MTRR_WRTHROUGH;  break;
	    }
	}
    else {
	switch (rcr) {
	    case  0: *type = PM_MTRR_UNCACHABLE; break;
	    case  8: *type = PM_MTRR_WRCOMB;     break;
	    case  9: *type = PM_MTRR_WRBACK;     break;
	    case 25:
	    default: *type = PM_MTRR_WRTHROUGH;  break;
	    }
	}
}

/****************************************************************************
PARAMETERS:
reg     - MTRR register to set
base    - The starting physical base address of the region
size    - The size in bytes of the region
type    - Type to place into the MTRR register

REMARKS:
Intel specific function to set the value of a specific MTRR register to
the passed in base, size and type.
****************************************************************************/
static void CYRIX_setMTRR(
    uint reg,
    ulong base,
    ulong size,
    int type)
{
    MTRRContext c;
    uchar       arr = CX86_ARR_BASE + reg*3;
    uchar       arr_type,arr_size;

    /* Count down from 32M (ARR0-ARR6) or from 2G (ARR7) */
    size >>= (reg < 7 ? 12 : 18);
    size &= 0x7FFF; /* Make sure arr_size <= 14 */
    for (arr_size = 0; size; arr_size++, size >>= 1)
	;
    if (reg < 7) {
	switch (type) {
	    case PM_MTRR_UNCACHABLE:    arr_type =  1; break;
	    case PM_MTRR_WRCOMB:        arr_type =  9; break;
	    case PM_MTRR_WRTHROUGH:     arr_type = 24; break;
	    default:                    arr_type =  8; break;
	    }
	}
    else {
	switch (type) {
	    case PM_MTRR_UNCACHABLE:    arr_type =  0; break;
	    case PM_MTRR_WRCOMB:        arr_type =  8; break;
	    case PM_MTRR_WRTHROUGH:     arr_type = 25; break;
	    default:                    arr_type =  9; break;
	    }
	}
    MTRR_beginUpdate(&c);
    _MTRR_setCx86((uchar)arr,     ((uchar*)&base)[3]);
    _MTRR_setCx86((uchar)(arr+1), ((uchar*)&base)[2]);
    _MTRR_setCx86((uchar)(arr+2), (uchar)((((uchar*)&base)[1]) | arr_size));
    _MTRR_setCx86((uchar)(CX86_RCR_BASE + reg), (uchar)arr_type);
    MTRR_endUpdate(&c);
}

/****************************************************************************
REMARKS:
On Cyrix 6x86(MX) and MII the ARR3 is special: it has connection
with the SMM (System Management Mode) mode. So we need the following:
Check whether SMI_LOCK (CCR3 bit 0) is set
  if it is set, ARR3 cannot be changed  (it cannot be changed until the
    next processor reset)
  if it is reset, then we can change it, set all the needed bits:
   - disable access to SMM memory through ARR3 range (CCR1 bit 7 reset)
   - disable access to SMM memory (CCR1 bit 2 reset)
   - disable SMM mode (CCR1 bit 1 reset)
   - disable write protection of ARR3 (CCR6 bit 1 reset)
   - (maybe) disable ARR3
Just to be sure, we enable ARR usage by the processor (CCR5 bit 5 set)
****************************************************************************/
static void CYRIX_initARR(void)
{
    MTRRContext c;
    uchar       ccr[7];
    int         ccrc[7] = { 0, 0, 0, 0, 0, 0, 0 };

    /* Begin updating */
    MTRR_beginUpdate(&c);

    /* Save all CCRs locally */
    ccr[0] = _MTRR_getCx86(CX86_CCR0);
    ccr[1] = _MTRR_getCx86(CX86_CCR1);
    ccr[2] = _MTRR_getCx86(CX86_CCR2);
    ccr[3] = (uchar)c.ccr3;
    ccr[4] = _MTRR_getCx86(CX86_CCR4);
    ccr[5] = _MTRR_getCx86(CX86_CCR5);
    ccr[6] = _MTRR_getCx86(CX86_CCR6);
    if (ccr[3] & 1)
	ccrc[3] = 1;
    else {
	/* Disable SMM mode (bit 1), access to SMM memory (bit 2) and
	 * access to SMM memory through ARR3 (bit 7).
	 */
	if (ccr[6] & 0x02) {
	    ccr[6] &= 0xFD;
	    ccrc[6] = 1;        /* Disable write protection of ARR3. */
	    _MTRR_setCx86(CX86_CCR6,ccr[6]);
	    }
	}

    /* If we changed CCR1 in memory, change it in the processor, too. */
    if (ccrc[1])
	_MTRR_setCx86(CX86_CCR1,ccr[1]);

    /* Enable ARR usage by the processor */
    if (!(ccr[5] & 0x20)) {
	ccr[5] |= 0x20;
	ccrc[5] = 1;
	_MTRR_setCx86(CX86_CCR5,ccr[5]);
	}

    /* We are finished updating */
    MTRR_endUpdate(&c);
}

/****************************************************************************
REMARKS:
Initialise the MTRR module, by detecting the processor type and determining
if the processor supports the MTRR functionality.
****************************************************************************/
void MTRR_init(void)
{
    int     i,cpu,ltype;
    ulong   eax,edx,lbase,lsize;

    /* Check that we have a compatible CPU */
    if (numMTRR == -1) {
	numMTRR = 0;
	if (!_MTRR_isRing0())
	    return;
	cpu = CPU_getProcessorType();
	cpuFamily = cpu & CPU_familyMask;
	cpuType = cpu & CPU_mask;
	cpuStepping = (cpu & CPU_steppingMask) >> CPU_steppingShift;
	switch (cpuFamily) {
	    case CPU_Intel:
		/* Intel Pentium Pro and later support the MTRR registers */
		if (cpuType < CPU_PentiumPro)
		    return;
		_MTRR_readMSR(INTEL_cap_MSR,&eax,&edx);
		numMTRR = eax & 0xFF;
		getMTRR = INTEL_getMTRR;
		setMTRR = INTEL_setMTRR;
		getFreeRegion = GENERIC_getFreeRegion;
		INTEL_disableBankedWriteCombine();
		break;
	    case CPU_AMD:
		/* AMD K6-2 and later support the MTRR registers */
		if ((cpuType < CPU_AMDK6_2) || (cpuType == CPU_AMDK6_2 && cpuStepping < 8))
		    return;
		if (cpuType < CPU_AMDAthlon) {
		    numMTRR = 2;        /* AMD CPU's have 2 MTRR's */
		    getMTRR = AMD_getMTRR;
		    setMTRR = AMD_setMTRR;
		    getFreeRegion = AMDK6_getFreeRegion;

		    /* For some reason some IBM systems with K6-2 processors
		     * have write combined enabled for the system BIOS
		     * region from 0xE0000 to 0xFFFFFF. We need *both* MTRR's
		     * for our own graphics drivers, so if we detect any
		     * regions below the 1Meg boundary, we remove them
		     * so we can use this MTRR register ourselves.
		     */
		    for (i = 0; i < numMTRR; i++) {
			getMTRR(i,&lbase,&lsize,&ltype);
			if (lbase < 0x100000)
			    setMTRR(i,0,0,0);
			}
		    }
		else {
		    /* AMD Athlon uses P6 style MTRR's */
		    _MTRR_readMSR(INTEL_cap_MSR,&eax,&edx);
		    numMTRR = eax & 0xFF;
		    getMTRR = INTEL_getMTRR;
		    setMTRR = INTEL_setMTRR;
		    getFreeRegion = GENERIC_getFreeRegion;
		    INTEL_disableBankedWriteCombine();
		    }
		break;
	    case CPU_Cyrix:
		/* Cyrix 6x86 and later support the MTRR registers */
		if (cpuType < CPU_Cyrix6x86 || cpuType >= CPU_CyrixMediaGX)
		    return;
		numMTRR = 8;        /* Cyrix CPU's have 8 ARR's */
		getMTRR = CYRIX_getMTRR;
		setMTRR = CYRIX_setMTRR;
		getFreeRegion = CYRIX_getFreeRegion;
		CYRIX_initARR();
		break;
	    default:
		return;
	    }
	}
}

/****************************************************************************
PARAMETERS:
base    - The starting physical base address of the region
size    - The size in bytes of the region
type    - Type to place into the MTRR register

RETURNS:
Error code describing the result.

REMARKS:
Function to enable write combining for the specified region of memory.
****************************************************************************/
int MTRR_enableWriteCombine(
    ulong base,
    ulong size,
    uint type)
{
    int     i;
    int     ltype;
    ulong   lbase,lsize,last;

    /* Check that we have a CPU that supports MTRR's and type is valid */
    if (numMTRR <= 0) {
	if (!_MTRR_isRing0())
	    return PM_MTRR_ERR_NO_OS_SUPPORT;
	return PM_MTRR_NOT_SUPPORTED;
	}
    if (type >= PM_MTRR_MAX)
	return PM_MTRR_ERR_PARAMS;

    /* If the type is WC, check that this processor supports it */
    if (!MTRR_haveWriteCombine())
	return PM_MTRR_ERR_NOWRCOMB;

    /* Adjust the boundaries depending on the CPU type */
    switch (cpuFamily) {
	case CPU_AMD:
	    if (cpuType < CPU_AMDAthlon) {
		/* Apply the K6 block alignment and size rules. In order:
		 *  o Uncached or gathering only
		 *  o 128K or bigger block
		 *  o Power of 2 block
		 *  o base suitably aligned to the power
		 */
		if (type > PM_MTRR_WRCOMB && (size < (1 << 17) || (size & ~(size-1))-size || (base & (size-1))))
		    return PM_MTRR_ERR_NOT_ALIGNED;
		break;
		}
	    /* Fall through for AMD Athlon which uses P6 style MTRR's */
	case CPU_Intel:
	case CPU_Cyrix:
	    if ((base & 0xFFF) || (size & 0xFFF)) {
		/* Base and size must be multiples of 4Kb */
		return PM_MTRR_ERR_NOT_4KB_ALIGNED;
		}
	    if (base < 0x100000) {
		/* Base must be >= 1Mb */
		return PM_MTRR_ERR_BELOW_1MB;
		}

	    /* Check upper bits of base and last are equal and lower bits
	     * are 0 for base and 1 for last
	     */
	    last = base + size - 1;
	    for (lbase = base; !(lbase & 1) && (last & 1); lbase = lbase >> 1, last = last >> 1)
		;
	    if (lbase != last) {
		/* Base is not aligned on the correct boundary */
		return PM_MTRR_ERR_NOT_ALIGNED;
		}
	    break;
	default:
	    return PM_MTRR_NOT_SUPPORTED;
	}

    /* Search for existing MTRR */
    for (i = 0; i < numMTRR; ++i) {
	getMTRR(i,&lbase,&lsize,&ltype);
	if (lbase == 0 && lsize == 0)
	    continue;
	if (base > lbase + (lsize-1))
	    continue;
	if ((base < lbase) && (base+size-1 < lbase))
	    continue;

	/* Check that we don't overlap an existing region */
	if (type != PM_MTRR_UNCACHABLE) {
	    if ((base < lbase) || (base+size-1 > lbase+lsize-1))
		return PM_MTRR_ERR_OVERLAP;
	    }
	else if (base == lbase && size == lsize) {
	    /* The region already exists so leave it alone */
	    return PM_MTRR_ERR_OK;
	    }

	/* New region is enclosed by an existing region, so only allow
	 * a new type to be created if we are setting a region to be
	 * uncacheable (such as MMIO registers within a framebuffer).
	 */
	if (ltype != (int)type) {
	    if (type == PM_MTRR_UNCACHABLE)
		continue;
	    return PM_MTRR_ERR_TYPE_MISMATCH;
	    }
	return PM_MTRR_ERR_OK;
	}

    /* Search for an empty MTRR */
    if ((i = getFreeRegion(base,size)) < 0)
	return PM_MTRR_ERR_NONE_FREE;
    setMTRR(i,base,size,type);
    return PM_MTRR_ERR_OK;
}

/****************************************************************************
PARAMETERS:
callback    - Function to callback with write combine information

REMARKS:
Function to enumerate all write combine regions currently enabled for the
processor.
****************************************************************************/
int PMAPI PM_enumWriteCombine(
    PM_enumWriteCombine_t callback)
{
    int     i,ltype;
    ulong   lbase,lsize;

    /* Check that we have a CPU that supports MTRR's and type is valid */
    if (numMTRR <= 0) {
	if (!_MTRR_isRing0())
	    return PM_MTRR_ERR_NO_OS_SUPPORT;
	return PM_MTRR_NOT_SUPPORTED;
	}

    /* Enumerate all existing MTRR's */
    for (i = 0; i < numMTRR; ++i) {
	getMTRR(i,&lbase,&lsize,&ltype);
	callback(lbase,lsize,ltype);
	}
    return PM_MTRR_ERR_OK;
}
#endif
