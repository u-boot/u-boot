/****************************************************************************
*
*                        BIOS emulator and interface
*                      to Realmode X86 Emulator Library
*
*               Copyright (C) 1996-1999 SciTech Software, Inc.
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.  The authors makes no
*  representations about the suitability of this software for any purpose.
*  It is provided "as is" without express or implied warranty.
*
*  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
*  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
*  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
*  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
*  PERFORMANCE OF THIS SOFTWARE.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
* Developer:    Kendall Bennett
*
* Description:  Header file for the real mode x86 BIOS emulator, which is
*               used to warmboot any number of VGA compatible PCI/AGP
*               controllers under any OS, on any processor family that
*               supports PCI. We also allow the user application to call
*               real mode BIOS functions and Int 10h functions (including
*               the VESA BIOS).
*
****************************************************************************/

#ifndef __BIOSEMU_H
#define __BIOSEMU_H

#include "x86emu.h"
#include "pmapi.h"
#include "pcilib.h"

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)

/****************************************************************************
REMARKS:
Data structure used to describe the details specific to a particular VGA
controller. This information is used to allow the VGA controller to be
swapped on the fly within the BIOS emulator.

HEADER:
biosemu.h

MEMBERS:
pciInfo         - PCI device information block for the controller
BIOSImage       - Pointer to a read/write copy of the BIOS image
BIOSImageLen    - Length of the BIOS image
LowMem          - Copy of key low memory areas
****************************************************************************/
typedef struct {
    PCIDeviceInfo   *pciInfo;
    void            *BIOSImage;
    ulong           BIOSImageLen;
    uchar           LowMem[1536];
    } BE_VGAInfo;

/****************************************************************************
REMARKS:
Data structure used to describe the details for the BIOS emulator system
environment as used by the X86 emulator library.

HEADER:
biosemu.h

MEMBERS:
vgaInfo         - VGA BIOS information structure
biosmem_base    - Base of the BIOS image
biosmem_limit   - Limit of the BIOS image
busmem_base     - Base of the VGA bus memory
****************************************************************************/
typedef struct {
    BE_VGAInfo      vgaInfo;
    ulong           biosmem_base;
    ulong           biosmem_limit;
    ulong           busmem_base;
    } BE_sysEnv;

/****************************************************************************
REMARKS:
Structure defining all the BIOS Emulator API functions as exported from
the Binary Portable DLL.
{secret}
****************************************************************************/
typedef struct {
    ulong   dwSize;
    ibool   (PMAPIP BE_init)(u32 debugFlags,int memSize,BE_VGAInfo *info);
    void    (PMAPIP BE_setVGA)(BE_VGAInfo *info);
    void    (PMAPIP BE_getVGA)(BE_VGAInfo *info);
    void *  (PMAPIP BE_mapRealPointer)(uint r_seg,uint r_off);
    void *  (PMAPIP BE_getVESABuf)(uint *len,uint *rseg,uint *roff);
    void    (PMAPIP BE_callRealMode)(uint seg,uint off,RMREGS *regs,RMSREGS *sregs);
    int     (PMAPIP BE_int86)(int intno,RMREGS *in,RMREGS *out);
    int     (PMAPIP BE_int86x)(int intno,RMREGS *in,RMREGS *out,RMSREGS *sregs);
    void *  reserved1;
    void    (PMAPIP BE_exit)(void);
    } BE_exports;

/****************************************************************************
REMARKS:
Function pointer type for the Binary Portable DLL initialisation entry point.
{secret}
****************************************************************************/
typedef BE_exports * (PMAPIP BE_initLibrary_t)(PM_imports *PMImp);

#pragma pack()

/*---------------------------- Global variables ---------------------------*/

#ifdef  __cplusplus
extern "C" {                        /* Use "C" linkage when in C++ mode */
#endif

/* {secret} Global BIOS emulator system environment */
extern BE_sysEnv _BE_env;

/*-------------------------- Function Prototypes --------------------------*/

/* BIOS emulator library entry points */

ibool   PMAPI BE_init(u32 debugFlags,int memSize,BE_VGAInfo *info);
void    PMAPI BE_setVGA(BE_VGAInfo *info);
void    PMAPI BE_getVGA(BE_VGAInfo *info);
void    PMAPI BE_setDebugFlags(u32 debugFlags);
void *  PMAPI BE_mapRealPointer(uint r_seg,uint r_off);
void *  PMAPI BE_getVESABuf(uint *len,uint *rseg,uint *roff);
void    PMAPI BE_callRealMode(uint seg,uint off,RMREGS *regs,RMSREGS *sregs);
int     PMAPI BE_int86(int intno,RMREGS *in,RMREGS *out);
int     PMAPI BE_int86x(int intno,RMREGS *in,RMREGS *out,RMSREGS *sregs);
void    PMAPI BE_exit(void);

#ifdef  __cplusplus
}                                   /* End of "C" linkage for C++       */
#endif

#endif /* __BIOSEMU_H */
