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
* Language:     ANSI C
* Environment:  32-bit DOS
*
* Description:  Main C module for the VFlat framebuffer routines. The page
*               fault handler is always installed to handle up to a 4Mb
*               framebuffer with a window size of 4Kb or 64Kb in size.
*
****************************************************************************/

#include "pmapi.h"
#include <stdlib.h>
#include <dos.h>

/*-------------------------------------------------------------------------*/
/* DOS4G/W, PMODE/W and CauseWay support.                                  */
/*-------------------------------------------------------------------------*/

#if defined(DOS4GW)

#define VFLAT_START_ADDR    0xF0000000U
#define VFLAT_END_ADDR      0xF03FFFFFU
#define VFLAT_LIMIT         (VFLAT_END_ADDR - VFLAT_START_ADDR)
#define PAGE_PRESENT        1
#define PAGE_NOTPRESENT     0
#define PAGE_READ           0
#define PAGE_WRITE          2

PRIVATE ibool   installed = false;
PRIVATE ibool   haveDPMI = false;
PUBLIC  ibool   _ASMAPI VF_haveCauseWay = false;
PUBLIC  uchar * _ASMAPI VF_zeroPtr = NULL;

/* Low level assembler code */

int     _ASMAPI InitPaging(void);
void    _ASMAPI ClosePaging(void);
void    _ASMAPI MapPhysical2Linear(ulong pAddr, ulong lAddr, int pages, int flags);
void    _ASMAPI InstallFaultHandler(ulong baseAddr,int bankSize);
void    _ASMAPI RemoveFaultHandler(void);
void    _ASMAPI InstallBankFunc(int codeLen,void *bankFunc);

void * _ASMAPI VF_malloc(uint size)
{ return PM_malloc(size); }

void _ASMAPI VF_free(void *p)
{ PM_free(p); }

PRIVATE ibool CheckDPMI(void)
/****************************************************************************
*
* Function:     CheckDPMI
* Returns:      True if we are running under DPMI
*
****************************************************************************/
{
    PMREGS  regs;

    if (haveDPMI)
	return true;

    /* Check if we are running under DPMI in which case we will not be
     * able to install our page fault handlers. We can however use the
     * DVA.386 or VFLATD.386 virtual device drivers if they are present.
     */
    regs.x.ax = 0xFF00;
    PM_int386(0x31,&regs,&regs);
    if (!regs.x.cflag && (regs.e.edi & 8))
	return (haveDPMI = true);
    return false;
}

ibool PMAPI VF_available(void)
/****************************************************************************
*
* Function:     VF_available
* Returns:      True if virtual buffer is available, false if not.
*
****************************************************************************/
{
    if (!VF_zeroPtr)
	VF_zeroPtr = PM_mapPhysicalAddr(0,0xFFFFFFFF,true);
    if (CheckDPMI())
	return false;

    /* Standard DOS4GW, PMODE/W and Causeway */
    if (InitPaging() == -1)
	return false;
    ClosePaging();
    return true;
}

void * PMAPI InitDPMI(ulong baseAddr,int bankSize,int codeLen,void *bankFunc)
/****************************************************************************
*
* Function:     InitDOS4GW
* Parameters:   baseAddr    - Base address of framebuffer bank window
*               bankSize    - Physical size of banks in Kb (4 or 64)
*               codeLen     - Length of 32 bit bank switch function
*               bankFunc    - Pointer to protected mode bank function
* Returns:      Near pointer to virtual framebuffer, or NULL on failure.
*
* Description:  Installs the virtual linear framebuffer handling for
*               DPMI environments. This requires the DVA.386 or VFLATD.386
*               virtual device drivers to be installed and functioning.
*
****************************************************************************/
{
    (void)baseAddr;
    (void)bankSize;
    (void)codeLen;
    (void)bankFunc;
    return NULL;
}

void * PMAPI InitDOS4GW(ulong baseAddr,int bankSize,int codeLen,void *bankFunc)
/****************************************************************************
*
* Function:     InitDOS4GW
* Parameters:   baseAddr    - Base address of framebuffer bank window
*               bankSize    - Physical size of banks in Kb (4 or 64)
*               codeLen     - Length of 32 bit bank switch function
*               bankFunc    - Pointer to protected mode bank function
* Returns:      Near pointer to virtual framebuffer, or NULL on failure.
*
* Description:  Installs the virtual linear framebuffer handling for
*               the DOS4GW extender.
*
****************************************************************************/
{
    int     i;

    if (InitPaging() == -1)
	return NULL;            /* Cannot do hardware paging!       */

    /* Map 4MB of video memory into linear address space (read/write) */
    if (bankSize == 64) {
	for (i = 0; i < 64; i++) {
	    MapPhysical2Linear(baseAddr,VFLAT_START_ADDR+(i<<16),16,
		PAGE_WRITE | PAGE_NOTPRESENT);
	    }
	}
    else {
	for (i = 0; i < 1024; i++) {
	    MapPhysical2Linear(baseAddr,VFLAT_START_ADDR+(i<<12),1,
		PAGE_WRITE | PAGE_NOTPRESENT);
	    }
	}

    /* Install our page fault handler and banks switch function */
    InstallFaultHandler(baseAddr,bankSize);
    InstallBankFunc(codeLen,bankFunc);
    installed = true;
    return (void*)VFLAT_START_ADDR;
}

void * PMAPI VF_init(ulong baseAddr,int bankSize,int codeLen,void *bankFunc)
/****************************************************************************
*
* Function:     VF_init
* Parameters:   baseAddr    - Base address of framebuffer bank window
*               bankSize    - Physical size of banks in Kb (4 or 64)
*               codeLen     - Length of 32 bit bank switch function
*               bankFunc    - Pointer to protected mode bank function
* Returns:      Near pointer to virtual framebuffer, or NULL on failure.
*
* Description:  Installs the virtual linear framebuffer handling.
*
****************************************************************************/
{
    if (installed)
	return (void*)VFLAT_START_ADDR;
    if (codeLen > 100)
	return NULL;                /* Bank function is too large!      */
    if (!VF_zeroPtr)
	VF_zeroPtr = PM_mapPhysicalAddr(0,0xFFFFFFFF,true);
    if (CheckDPMI())
	return InitDPMI(baseAddr,bankSize,codeLen,bankFunc);
    return InitDOS4GW(baseAddr,bankSize,codeLen,bankFunc);
}

void PMAPI VF_exit(void)
/****************************************************************************
*
* Function:     VF_exit
*
* Description:  Closes down the virtual framebuffer services and
*               restores the previous page fault handler.
*
****************************************************************************/
{
    if (installed) {
	if (haveDPMI) {
	    /* DPMI support */
	    }
	else {
	    /* Standard DOS4GW and PMODE/W support */
	    RemoveFaultHandler();
	    ClosePaging();
	    }
	installed = false;
	}
}

/*-------------------------------------------------------------------------*/
/* Support mapped out for other compilers.                                 */
/*-------------------------------------------------------------------------*/

#else

ibool PMAPI VF_available(void)
{
    return false;
}

void * PMAPI VF_init(ulong baseAddr,int bankSize,int codeLen,void *bankFunc)
{
    (void)baseAddr;
    (void)bankSize;
    (void)codeLen;
    (void)bankFunc;
    return NULL;
}

void PMAPI VF_exit(void)
{
}

#endif
