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
* Description:  Module implementing the BIOS specific functions.
*
****************************************************************************/

#include "biosemui.h"

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
PARAMETERS:
intno   - Interrupt number being serviced

REMARKS:
Handler for undefined interrupts.
****************************************************************************/
static void X86API undefined_intr(
    int intno)
{
    if (BE_rdw(intno * 4 + 2) == BIOS_SEG)
	printk("biosEmu: undefined interrupt %xh called!\n",intno);
    else
	X86EMU_prepareForInt(intno);
}

/****************************************************************************
PARAMETERS:
intno   - Interrupt number being serviced

REMARKS:
This function handles the default system BIOS Int 10h (the default is stored
in the Int 42h vector by the system BIOS at bootup). We only need to handle
a small number of special functions used by the BIOS during POST time.
****************************************************************************/
static void X86API int42(
    int intno)
{
    if (M.x86.R_AH == 0x12 && M.x86.R_BL == 0x32) {
	if (M.x86.R_AL == 0) {
	    /* Enable CPU accesses to video memory */
	    PM_outpb(0x3c2, PM_inpb(0x3cc) | (u8)0x02);
	    return;
	    }
	else if (M.x86.R_AL == 1) {
	    /* Disable CPU accesses to video memory */
	    PM_outpb(0x3c2, PM_inpb(0x3cc) & (u8)~0x02);
	    return;
	    }
#ifdef  DEBUG
	else {
	    printk("biosEmu/bios.int42: unknown function AH=0x12, BL=0x32, AL=%#02x\n",M.x86.R_AL);
	    }
#endif
	}
#ifdef  DEBUG
    else {
	printk("biosEmu/bios.int42: unknown function AH=%#02x, AL=%#02x, BL=%#02x\n",M.x86.R_AH, M.x86.R_AL, M.x86.R_BL);
	}
#endif
}

/****************************************************************************
PARAMETERS:
intno   - Interrupt number being serviced

REMARKS:
This function handles the default system BIOS Int 10h. If the POST code
has not yet re-vectored the Int 10h BIOS interrupt vector, we handle this
by simply calling the int42 interrupt handler above. Very early in the
BIOS POST process, the vector gets replaced and we simply let the real
mode interrupt handler process the interrupt.
****************************************************************************/
static void X86API int10(
    int intno)
{
    if (BE_rdw(intno * 4 + 2) == BIOS_SEG)
	int42(intno);
    else
	X86EMU_prepareForInt(intno);
}

/* Result codes returned by the PCI BIOS */

#define SUCCESSFUL          0x00
#define FUNC_NOT_SUPPORT    0x81
#define BAD_VENDOR_ID       0x83
#define DEVICE_NOT_FOUND    0x86
#define BAD_REGISTER_NUMBER 0x87
#define SET_FAILED          0x88
#define BUFFER_TOO_SMALL    0x89

/****************************************************************************
PARAMETERS:
intno   - Interrupt number being serviced

REMARKS:
This function handles the default Int 1Ah interrupt handler for the real
mode code, which provides support for the PCI BIOS functions. Since we only
want to allow the real mode BIOS code *only* see the PCI config space for
its own device, we only return information for the specific PCI config
space that we have passed in to the init function. This solves problems
when using the BIOS to warm boot a secondary adapter when there is an
identical adapter before it on the bus (some BIOS'es get confused in this
case).
****************************************************************************/
static void X86API int1A(
    unused)
{
    u16 pciSlot;

    /* Fail if no PCI device information has been registered */
    if (!_BE_env.vgaInfo.pciInfo)
	return;
    pciSlot = (u16)(_BE_env.vgaInfo.pciInfo->slot.i >> 8);
    switch (M.x86.R_AX) {
	case 0xB101:                    /* PCI bios present? */
	    M.x86.R_AL  = 0x00;         /* no config space/special cycle generation support */
	    M.x86.R_EDX = 0x20494350;   /* " ICP" */
	    M.x86.R_BX  = 0x0210;       /* Version 2.10 */
	    M.x86.R_CL  = 0;            /* Max bus number in system */
	    CLEAR_FLAG(F_CF);
	    break;
	case 0xB102:                    /* Find PCI device */
	    M.x86.R_AH = DEVICE_NOT_FOUND;
	    if (M.x86.R_DX == _BE_env.vgaInfo.pciInfo->VendorID &&
		    M.x86.R_CX == _BE_env.vgaInfo.pciInfo->DeviceID &&
		    M.x86.R_SI == 0) {
		M.x86.R_AH = SUCCESSFUL;
		M.x86.R_BX = pciSlot;
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	case 0xB103:                    /* Find PCI class code */
	    M.x86.R_AH = DEVICE_NOT_FOUND;
	    if (M.x86.R_CL == _BE_env.vgaInfo.pciInfo->Interface &&
		    M.x86.R_CH == _BE_env.vgaInfo.pciInfo->SubClass &&
		    (u8)(M.x86.R_ECX >> 16) == _BE_env.vgaInfo.pciInfo->BaseClass) {
		M.x86.R_AH = SUCCESSFUL;
		M.x86.R_BX = pciSlot;
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	case 0xB108:                    /* Read configuration byte */
	    M.x86.R_AH = BAD_REGISTER_NUMBER;
	    if (M.x86.R_BX == pciSlot) {
		M.x86.R_AH = SUCCESSFUL;
		M.x86.R_CL = (u8)PCI_accessReg(M.x86.R_DI,0,PCI_READ_BYTE,_BE_env.vgaInfo.pciInfo);
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	case 0xB109:                    /* Read configuration word */
	    M.x86.R_AH = BAD_REGISTER_NUMBER;
	    if (M.x86.R_BX == pciSlot) {
		M.x86.R_AH = SUCCESSFUL;
		M.x86.R_CX = (u16)PCI_accessReg(M.x86.R_DI,0,PCI_READ_WORD,_BE_env.vgaInfo.pciInfo);
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	case 0xB10A:                    /* Read configuration dword */
	    M.x86.R_AH = BAD_REGISTER_NUMBER;
	    if (M.x86.R_BX == pciSlot) {
		M.x86.R_AH = SUCCESSFUL;
		M.x86.R_ECX = (u32)PCI_accessReg(M.x86.R_DI,0,PCI_READ_DWORD,_BE_env.vgaInfo.pciInfo);
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	case 0xB10B:                    /* Write configuration byte */
	    M.x86.R_AH = BAD_REGISTER_NUMBER;
	    if (M.x86.R_BX == pciSlot) {
		M.x86.R_AH = SUCCESSFUL;
		PCI_accessReg(M.x86.R_DI,M.x86.R_CL,PCI_WRITE_BYTE,_BE_env.vgaInfo.pciInfo);
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	case 0xB10C:                    /* Write configuration word */
	    M.x86.R_AH = BAD_REGISTER_NUMBER;
	    if (M.x86.R_BX == pciSlot) {
		M.x86.R_AH = SUCCESSFUL;
		PCI_accessReg(M.x86.R_DI,M.x86.R_CX,PCI_WRITE_WORD,_BE_env.vgaInfo.pciInfo);
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	case 0xB10D:                    /* Write configuration dword */
	    M.x86.R_AH = BAD_REGISTER_NUMBER;
	    if (M.x86.R_BX == pciSlot) {
		M.x86.R_AH = SUCCESSFUL;
		PCI_accessReg(M.x86.R_DI,M.x86.R_ECX,PCI_WRITE_DWORD,_BE_env.vgaInfo.pciInfo);
		}
	    CONDITIONAL_SET_FLAG((M.x86.R_AH != SUCCESSFUL), F_CF);
	    break;
	default:
	    printk("biosEmu/bios.int1a: unknown function AX=%#04x\n", M.x86.R_AX);
	}
}

/****************************************************************************
REMARKS:
This function initialises the BIOS emulation functions for the specific
PCI display device. We insulate the real mode BIOS from any other devices
on the bus, so that it will work correctly thinking that it is the only
device present on the bus (ie: avoiding any adapters present in from of
the device we are trying to control).
****************************************************************************/
void _BE_bios_init(
    u32 *intrTab)
{
    int                 i;
    X86EMU_intrFuncs    bios_intr_tab[256];

    for (i = 0; i < 256; ++i) {
	intrTab[i] = BIOS_SEG << 16;
	bios_intr_tab[i] = undefined_intr;
	}
    bios_intr_tab[0x10] = int10;
    bios_intr_tab[0x1A] = int1A;
    bios_intr_tab[0x42] = int42;
    X86EMU_setupIntrFuncs(bios_intr_tab);
}
