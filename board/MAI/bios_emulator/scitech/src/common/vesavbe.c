/****************************************************************************
*
*           The SuperVGA Kit - UniVBE Software Development Kit
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
* Environment:  IBM PC Real Mode and 16/32 bit Protected Mode.
*
* Description:  Module to implement a C callable interface to the standard
*               VESA VBE routines. You should rip out this module and use it
*               directly in your own applications, or you can use the
*               high level SDK functions.
*
*               MUST be compiled in the LARGE or FLAT models.
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vesavbe.h"
#include "pmapi.h"
#include "drvlib/os/os.h"

/*---------------------------- Global Variables ---------------------------*/

#define VBE_SUCCESS     0x004F
#define MAX_LIN_PTRS    10

static uint         VESABuf_len = 1024;/* Length of the VESABuf buffer  */
static ibool        haveRiva128;    /* True if we have a Riva128        */
static VBE_state    defState = {0}; /* Default state buffer             */
static VBE_state    *state = &defState; /* Pointer to current buffer    */
static int          VBE_shared = 0;
#ifndef REALMODE
static char         localBuf[512];  /* Global PM string translate buf   */
#define MAX_LOCAL_BUF &localBuf[511]
#endif

/*----------------------------- Implementation ----------------------------*/

/* static function in WinDirect for passing 32-bit registers to BIOS */
int PMAPI WD_int386(int intno, RMREGS *in, RMREGS *out);

void VBEAPI VBE_init(void)
/****************************************************************************
*
* Function:     VBE_init
*
* Description:  Initialises the VBE transfer buffer in real mode DC.memory.
*               This routine is called by the VESAVBE module every time
*               it needs to use the transfer buffer, so we simply allocate
*               it once and then return.
*
****************************************************************************/
{
    if (!state->VESABuf_ptr) {
	/* Allocate a global buffer for communicating with the VESA VBE */
	if ((state->VESABuf_ptr = PM_getVESABuf(&VESABuf_len, &state->VESABuf_rseg, &state->VESABuf_roff)) == NULL)
	    PM_fatalError("VESAVBE.C: Real mode memory allocation failed!");
	}
}

void * VBEAPI VBE_getRMBuf(uint *len,uint *rseg,uint *roff)
/****************************************************************************
*
* Function:     VBE_getRMBuf
*
* Description:  This function returns the location and length of the real
*               mode memory buffer for calling real mode functions.
*
****************************************************************************/
{
    *len = VESABuf_len;
    *rseg = state->VESABuf_rseg;
    *roff = state->VESABuf_roff;
    return state->VESABuf_ptr;
}

void VBEAPI VBE_setStateBuffer(VBE_state *s)
/****************************************************************************
*
* Function:     VBE_setStateBuffer
*
* Description:  This functions sets the internal state buffer for the
*               VBE module to the passed in buffer. By default the internal
*               global buffer is used, but you must use separate buffers
*               for each device in a multi-controller environment.
*
****************************************************************************/
{
    state = s;
}

void VBEAPI VBE_callESDI(RMREGS *regs, void *buffer, int size)
/****************************************************************************
*
* Function:     VBE_callESDI
* Parameters:   regs    - Registers to load when calling VBE
*               buffer  - Buffer to copy VBE info block to
*               size    - Size of buffer to fill
*
* Description:  Calls the VESA VBE and passes in a buffer for the VBE to
*               store information in, which is then copied into the users
*               buffer space. This works in protected mode as the buffer
*               passed to the VESA VBE is allocated in conventional
*               memory, and is then copied into the users memory block.
*
****************************************************************************/
{
    RMSREGS sregs;

    if (!state->VESABuf_ptr)
	PM_fatalError("You *MUST* call VBE_init() before you can call the VESAVBE.C module!");
    sregs.es = (ushort)state->VESABuf_rseg;
    regs->x.di = (ushort)state->VESABuf_roff;
    memcpy(state->VESABuf_ptr, buffer, size);
    PM_int86x(0x10, regs, regs, &sregs);
    memcpy(buffer, state->VESABuf_ptr, size);
}

#ifndef REALMODE
static char *VBE_copyStrToLocal(char *p,char *realPtr,char *max)
/****************************************************************************
*
* Function:     VBE_copyStrToLocal
* Parameters:   p       - Flat model buffer to copy to
*               realPtr - Real mode pointer to copy
* Returns:      Pointer to the next byte after string
*
* Description:  Copies the string from the real mode location pointed to
*               by 'realPtr' into the flat model buffer pointed to by
*               'p'. We return a pointer to the next byte past the copied
*               string.
*
****************************************************************************/
{
    uchar   *v;

    v = PM_mapRealPointer((uint)((ulong)realPtr >> 16), (uint)((ulong)realPtr & 0xFFFF));
    while (*v != 0 && p < max)
	*p++ = *v++;
    *p++ = 0;
    return p;
}

static void VBE_copyShortToLocal(ushort *p,ushort *realPtr)
/****************************************************************************
*
* Function:     VBE_copyShortToLocal
* Parameters:   p       - Flat model buffer to copy to
*               realPtr - Real mode pointer to copy
*
* Description:  Copies the mode table from real mode memory to the flat
*               model buffer.
*
****************************************************************************/
{
    ushort  *v;

    v = PM_mapRealPointer((uint)((ulong)realPtr >> 16),(uint)((ulong)realPtr & 0xFFFF));
    while (*v != 0xFFFF)
	*p++ = *v++;
    *p = 0xFFFF;
}
#endif

int VBEAPI VBE_detectEXT(VBE_vgaInfo *vgaInfo,ibool forceUniVBE)
/****************************************************************************
*
* Function:     VBE_detect
* Parameters:   vgaInfo - Place to store the VGA information block
* Returns:      VBE version number, or 0 if not detected.
*
* Description:  Detects if a VESA VBE is out there and functioning
*               correctly. If we detect a VBE interface we return the
*               VGAInfoBlock returned by the VBE and the VBE version number.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F00;     /* Get SuperVGA information */
    if (forceUniVBE) {
	regs.x.bx = 0x1234;
	regs.x.cx = 0x4321;
	}
    else {
	regs.x.bx = 0;
	regs.x.cx = 0;
	}
    strncpy(vgaInfo->VESASignature,"VBE2",4);
    VBE_callESDI(&regs, vgaInfo, sizeof(*vgaInfo));
    if (regs.x.ax != VBE_SUCCESS)
	return 0;
    if (strncmp(vgaInfo->VESASignature,"VESA",4) != 0)
	return 0;

    /* Check for bogus BIOSes that return a VBE version number that is
     * not correct, and fix it up. We also check the OemVendorNamePtr for a
     * valid value, and if it is invalid then we also reset to VBE 1.2.
     */
    if (vgaInfo->VESAVersion >= 0x200 && vgaInfo->OemVendorNamePtr == 0)
	vgaInfo->VESAVersion = 0x102;
#ifndef REALMODE
    /* Relocate all the indirect information (mode tables, OEM strings
     * etc) from the low 1Mb memory region into a static buffer in
     * our default data segment. We do this to insulate the application
     * from mapping the strings from real mode to protected mode.
     */
    {
	char *p,*p2;
     p2 = VBE_copyStrToLocal(localBuf,vgaInfo->OemStringPtr,MAX_LOCAL_BUF);
     vgaInfo->OemStringPtr = localBuf;
     if (vgaInfo->VESAVersion >= 0x200) {
	 p = VBE_copyStrToLocal(p2,vgaInfo->OemVendorNamePtr,MAX_LOCAL_BUF);
	 vgaInfo->OemVendorNamePtr = p2;
	 p2 = VBE_copyStrToLocal(p,vgaInfo->OemProductNamePtr,MAX_LOCAL_BUF);
	 vgaInfo->OemProductNamePtr = p;
	 p = VBE_copyStrToLocal(p2,vgaInfo->OemProductRevPtr,MAX_LOCAL_BUF);
	 vgaInfo->OemProductRevPtr = p2;
	 VBE_copyShortToLocal((ushort*)p,vgaInfo->VideoModePtr);
	 vgaInfo->VideoModePtr = (ushort*)p;
	 }
     else {
	 VBE_copyShortToLocal((ushort*)p2,vgaInfo->VideoModePtr);
	 vgaInfo->VideoModePtr = (ushort*)p2;
	 }
    }
#endif
    state->VBEMemory = vgaInfo->TotalMemory * 64;

    /* Check for Riva128 based cards since they have broken triple buffering
     * and stereo support.
     */
    haveRiva128 = false;
    if (vgaInfo->VESAVersion >= 0x300 &&
	   (strstr(vgaInfo->OemStringPtr,"NVidia") != NULL ||
	    strstr(vgaInfo->OemStringPtr,"Riva") != NULL)) {
	haveRiva128 = true;
	}

    /* Check for Matrox G400 cards which claim to be VBE 3.0
     * compliant yet they don't implement the refresh rate control
     * functions.
     */
    if (vgaInfo->VESAVersion >= 0x300 && (strcmp(vgaInfo->OemProductNamePtr,"Matrox G400") == 0))
	vgaInfo->VESAVersion = 0x200;
    return (state->VBEVersion = vgaInfo->VESAVersion);
}

int VBEAPI VBE_detect(VBE_vgaInfo *vgaInfo)
/****************************************************************************
*
* Function:     VBE_detect
* Parameters:   vgaInfo - Place to store the VGA information block
* Returns:      VBE version number, or 0 if not detected.
*
* Description:  Detects if a VESA VBE is out there and functioning
*               correctly. If we detect a VBE interface we return the
*               VGAInfoBlock returned by the VBE and the VBE version number.
*
****************************************************************************/
{
    return VBE_detectEXT(vgaInfo,false);
}

ibool VBEAPI VBE_getModeInfo(int mode,VBE_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     VBE_getModeInfo
* Parameters:   mode        - VBE mode to get information for
*               modeInfo    - Place to store VBE mode information
* Returns:      True on success, false if function failed.
*
* Description:  Obtains information about a specific video mode from the
*               VBE. You should use this function to find the video mode
*               you wish to set, as the new VBE 2.0 mode numbers may be
*               completely arbitrary.
*
****************************************************************************/
{
    RMREGS  regs;
    int     bits;

    regs.x.ax = 0x4F01;             /* Get mode information         */
    regs.x.cx = (ushort)mode;
    VBE_callESDI(&regs, modeInfo, sizeof(*modeInfo));
    if (regs.x.ax != VBE_SUCCESS)
	return false;
    if ((modeInfo->ModeAttributes & vbeMdAvailable) == 0)
	return false;

    /* Map out triple buffer and stereo flags for NVidia Riva128
     * chips.
     */
    if (haveRiva128) {
	modeInfo->ModeAttributes &= ~vbeMdTripleBuf;
	modeInfo->ModeAttributes &= ~vbeMdStereo;
	}

    /* Support old style RGB definitions for VBE 1.1 BIOSes */
    bits = modeInfo->BitsPerPixel;
    if (modeInfo->MemoryModel == vbeMemPK && bits > 8) {
	modeInfo->MemoryModel = vbeMemRGB;
	switch (bits) {
	    case 15:
		modeInfo->RedMaskSize = 5;
		modeInfo->RedFieldPosition = 10;
		modeInfo->GreenMaskSize = 5;
		modeInfo->GreenFieldPosition = 5;
		modeInfo->BlueMaskSize = 5;
		modeInfo->BlueFieldPosition = 0;
		modeInfo->RsvdMaskSize = 1;
		modeInfo->RsvdFieldPosition = 15;
		break;
	    case 16:
		modeInfo->RedMaskSize = 5;
		modeInfo->RedFieldPosition = 11;
		modeInfo->GreenMaskSize = 5;
		modeInfo->GreenFieldPosition = 5;
		modeInfo->BlueMaskSize = 5;
		modeInfo->BlueFieldPosition = 0;
		modeInfo->RsvdMaskSize = 0;
		modeInfo->RsvdFieldPosition = 0;
		break;
	    case 24:
		modeInfo->RedMaskSize = 8;
		modeInfo->RedFieldPosition = 16;
		modeInfo->GreenMaskSize = 8;
		modeInfo->GreenFieldPosition = 8;
		modeInfo->BlueMaskSize = 8;
		modeInfo->BlueFieldPosition = 0;
		modeInfo->RsvdMaskSize = 0;
		modeInfo->RsvdFieldPosition = 0;
		break;
	    }
	}

    /* Convert the 32k direct color modes of VBE 1.2+ BIOSes to
     * be recognised as 15 bits per pixel modes.
     */
    if (bits == 16 && modeInfo->RsvdMaskSize == 1)
	modeInfo->BitsPerPixel = 15;

    /* Fix up bogus BIOS'es that report incorrect reserved pixel masks
     * for 32K color modes. Quite a number of BIOS'es have this problem,
     * and this affects our OS/2 drivers in VBE fallback mode.
     */
    if (bits == 15 && (modeInfo->RsvdMaskSize != 1 || modeInfo->RsvdFieldPosition != 15)) {
	modeInfo->RsvdMaskSize = 1;
	modeInfo->RsvdFieldPosition = 15;
	}
    return true;
}

long VBEAPI VBE_getPageSize(VBE_modeInfo *mi)
/****************************************************************************
*
* Function:     VBE_getPageSize
* Parameters:   mi  - Pointer to mode information block
* Returns:      Caculated page size in bytes rounded to correct boundary
*
* Description:  Computes the page size in bytes for the specified mode
*               information block, rounded up to the appropriate boundary
*               (8k, 16k, 32k or 64k). Pages >= 64k in size are always
*               rounded to the nearest 64k boundary (so the start of a
*               page is always bank aligned).
*
****************************************************************************/
{
    long size;

    size = (long)mi->BytesPerScanLine * (long)mi->YResolution;
    if (mi->BitsPerPixel == 4) {
	/* We have a 16 color video mode, so round up the page size to
	 * 8k, 16k, 32k or 64k boundaries depending on how large it is.
	 */

	size = (size + 0x1FFFL) & 0xFFFFE000L;
	if (size != 0x2000) {
	    size = (size + 0x3FFFL) & 0xFFFFC000L;
	    if (size != 0x4000) {
		size = (size + 0x7FFFL) & 0xFFFF8000L;
		if (size != 0x8000)
		    size = (size + 0xFFFFL) & 0xFFFF0000L;
		}
	    }
	}
    else size = (size + 0xFFFFL) & 0xFFFF0000L;
    return size;
}

ibool VBEAPI VBE_setVideoModeExt(int mode,VBE_CRTCInfo *crtc)
/****************************************************************************
*
* Function:     VBE_setVideoModeExt
* Parameters:   mode    - SuperVGA video mode to set.
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode. This version
*               includes support for the VBE/Core 3.0 refresh rate control
*               mechanism.
*
****************************************************************************/
{
    RMREGS  regs;

    if (state->VBEVersion < 0x200 && mode < 0x100) {
	/* Some VBE implementations barf terribly if you try to set non-VBE
	 * video modes with the VBE set mode call. VBE 2.0 implementations
	 * must be able to handle this.
	 */
	regs.h.al = (ushort)mode;
	regs.h.ah = 0;
	PM_int86(0x10,&regs,&regs);
	}
    else {
	if (state->VBEVersion < 0x300 && (mode & vbeRefreshCtrl))
	    return false;
	regs.x.ax = 0x4F02;
	regs.x.bx = (ushort)mode;
	if ((mode & vbeRefreshCtrl) && crtc)
	    VBE_callESDI(&regs, crtc, sizeof(*crtc));
	else
	    PM_int86(0x10,&regs,&regs);
	if (regs.x.ax != VBE_SUCCESS)
	    return false;
	}
    return true;
}

ibool VBEAPI VBE_setVideoMode(int mode)
/****************************************************************************
*
* Function:     VBE_setVideoMode
* Parameters:   mode    - SuperVGA video mode to set.
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode.
*
****************************************************************************/
{
    return VBE_setVideoModeExt(mode,NULL);
}

int VBEAPI VBE_getVideoMode(void)
/****************************************************************************
*
* Function:     VBE_getVideoMode
* Returns:      Current video mode
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F03;
    PM_int86(0x10,&regs,&regs);
    if (regs.x.ax != VBE_SUCCESS)
	return -1;
    return regs.x.bx;
}

ibool VBEAPI VBE_setBank(int window,int bank)
/****************************************************************************
*
* Function:     VBE_setBank
* Parameters:   window  - Window to set
*               bank    - Bank number to set window to
* Returns:      True on success, false on failure.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F05;
    regs.h.bh = 0;
    regs.h.bl = window;
    regs.x.dx = bank;
    PM_int86(0x10,&regs,&regs);
    return regs.x.ax == VBE_SUCCESS;
}

int VBEAPI VBE_getBank(int window)
/****************************************************************************
*
* Function:     VBE_setBank
* Parameters:   window  - Window to read
* Returns:      Bank number for the window (-1 on failure)
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F05;
    regs.h.bh = 1;
    regs.h.bl = window;
    PM_int86(0x10,&regs,&regs);
    if (regs.x.ax != VBE_SUCCESS)
	return -1;
    return regs.x.dx;
}

ibool VBEAPI VBE_setPixelsPerLine(int pixelsPerLine,int *newBytes,
    int *newPixels,int *maxScanlines)
/****************************************************************************
*
* Function:     VBE_setPixelsPerLine
* Parameters:   pixelsPerLine   - Pixels per scanline
*               newBytes        - Storage for bytes per line value set
*               newPixels       - Storage for pixels per line value set
*               maxScanLines    - Storage for maximum number of scanlines
* Returns:      True on success, false on failure
*
* Description:  Sets the scanline length for the video mode to the specified
*               number of pixels per scanline. If you need more granularity
*               in TrueColor modes, use the VBE_setBytesPerLine routine
*               (only valid for VBE 2.0).
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 0;
    regs.x.cx = pixelsPerLine;
    PM_int86(0x10,&regs,&regs);
    *newBytes = regs.x.bx;
    *newPixels = regs.x.cx;
    *maxScanlines = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_setBytesPerLine(int bytesPerLine,int *newBytes,
    int *newPixels,int *maxScanlines)
/****************************************************************************
*
* Function:     VBE_setBytesPerLine
* Parameters:   pixelsPerLine   - Pixels per scanline
*               newBytes        - Storage for bytes per line value set
*               newPixels       - Storage for pixels per line value set
*               maxScanLines    - Storage for maximum number of scanlines
* Returns:      True on success, false on failure
*
* Description:  Sets the scanline length for the video mode to the specified
*               number of bytes per scanline (valid for VBE 2.0 only).
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 2;
    regs.x.cx = bytesPerLine;
    PM_int86(0x10,&regs,&regs);
    *newBytes = regs.x.bx;
    *newPixels = regs.x.cx;
    *maxScanlines = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_getScanlineLength(int *bytesPerLine,int *pixelsPerLine,
    int *maxScanlines)
/****************************************************************************
*
* Function:     VBE_getScanlineLength
* Parameters:   bytesPerLine    - Storage for bytes per scanline
*               pixelsPerLine   - Storage for pixels per scanline
*               maxScanLines    - Storage for maximum number of scanlines
* Returns:      True on success, false on failure
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 1;
    PM_int86(0x10,&regs,&regs);
    *bytesPerLine = regs.x.bx;
    *pixelsPerLine = regs.x.cx;
    *maxScanlines = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_getMaxScanlineLength(int *maxBytes,int *maxPixels)
/****************************************************************************
*
* Function:     VBE_getMaxScanlineLength
* Parameters:   maxBytes    - Maximum scanline width in bytes
*               maxPixels   - Maximum scanline width in pixels
* Returns:      True if successful, false if function failed
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 3;
    PM_int86(0x10,&regs,&regs);
    *maxBytes = regs.x.bx;
    *maxPixels = regs.x.cx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_setDisplayStart(int x,int y,ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setDisplayStart
* Parameters:   x,y     - Position of the first pixel to display
*               waitVRT - True to wait for retrace, false if not
* Returns:      True if function was successful.
*
* Description:  Sets the new starting display position to implement
*               hardware scrolling.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F07;
    if (waitVRT)
	regs.x.bx = 0x80;
    else regs.x.bx = 0x00;
    regs.x.cx = x;
    regs.x.dx = y;
    PM_int86(0x10,&regs,&regs);
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_getDisplayStart(int *x,int *y)
/****************************************************************************
*
* Function:     VBE_getDisplayStart
* Parameters:   x,y - Place to store starting address value
* Returns:      True if function was successful.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F07;
    regs.x.bx = 0x01;
    PM_int86(0x10,&regs,&regs);
    *x = regs.x.cx;
    *y = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_setDisplayStartAlt(ulong startAddr,ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setDisplayStartAlt
* Parameters:   startAddr   - 32-bit starting address in display memory
*               waitVRT     - True to wait for vertical retrace, false if not
* Returns:      True if function was successful, false if not supported.
*
* Description:  Sets the new starting display position to the specified
*               32-bit display start address. Note that this function is
*               different the the version above, since it takes a 32-bit
*               byte offset in video memory as the starting address which
*               gives the programmer maximum control over the stat address.
*
*               NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
    RMREGS  regs;

    if (state->VBEVersion >= 0x300) {
	regs.x.ax = 0x4F07;
	regs.x.bx = waitVRT ? 0x82 : 0x02;
	regs.e.ecx = startAddr;
	PM_int86(0x10,&regs,&regs);
	return regs.x.ax == VBE_SUCCESS;
	}
    return false;
}

int VBEAPI VBE_getDisplayStartStatus(void)
/****************************************************************************
*
* Function:     VBE_getDisplayStartStatus
* Returns:      0 if last flip not occurred, 1 if already flipped
*               -1 if not supported
*
* Description:  Returns the status of the previous display start request.
*               If this function is supported the programmer can implement
*               hardware triple buffering using this function.
*
*               NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
    RMREGS  regs;

    if (state->VBEVersion >= 0x300) {
	regs.x.ax = 0x4F07;
	regs.x.bx = 0x0004;
	PM_int86(0x10,&regs,&regs);
	if (regs.x.ax == VBE_SUCCESS)
	    return (regs.x.cx != 0);
	}
    return -1;
}

ibool VBEAPI VBE_enableStereoMode(void)
/****************************************************************************
*
* Function:     VBE_enableStereoMode
* Returns:      True if stereo mode enabled, false if not supported.
*
* Description:  Puts the system into hardware stereo mode for LC shutter
*               glasses, where the display swaps between two display start
*               addresses every vertical retrace.
*
*               NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
    RMREGS  regs;

    if (state->VBEVersion >= 0x300) {
	regs.x.ax = 0x4F07;
	regs.x.bx = 0x0005;
	PM_int86(0x10,&regs,&regs);
	return regs.x.ax == VBE_SUCCESS;
	}
    return false;
}

ibool VBEAPI VBE_disableStereoMode(void)
/****************************************************************************
*
* Function:     VBE_disableStereoMode
* Returns:      True if stereo mode disabled, false if not supported.
*
* Description:  Puts the system back into normal, non-stereo display mode
*               after having stereo mode enabled.
*
*               NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
    RMREGS  regs;

    if (state->VBEVersion >= 0x300) {
	regs.x.ax = 0x4F07;
	regs.x.bx = 0x0006;
	PM_int86(0x10,&regs,&regs);
	return regs.x.ax == VBE_SUCCESS;
	}
    return false;
}

ibool VBEAPI VBE_setStereoDisplayStart(ulong leftAddr,ulong rightAddr,
    ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setStereoDisplayStart
* Parameters:   leftAddr    - 32-bit start address for left image
*               rightAddr   - 32-bit start address for right image
*               waitVRT     - True to wait for vertical retrace, false if not
* Returns:      True if function was successful, false if not supported.
*
* Description:  Sets the new starting display position to the specified
*               32-bit display start address. Note that this function is
*               different the the version above, since it takes a 32-bit
*               byte offset in video memory as the starting address which
*               gives the programmer maximum control over the stat address.
*
*               NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
    RMREGS  regs;

    if (state->VBEVersion >= 0x300) {
	regs.x.ax = 0x4F07;
	regs.x.bx = waitVRT ? 0x83 : 0x03;
	regs.e.ecx = leftAddr;
	regs.e.edx = rightAddr;
	PM_int86(0x10,&regs,&regs);
	return regs.x.ax == VBE_SUCCESS;
	}
    return false;
}

ulong VBEAPI VBE_getClosestClock(ushort mode,ulong pixelClock)
/****************************************************************************
*
* Function:     VBE_getClosestClock
* Parameters:   mode        - VBE mode to be used (include vbeLinearBuffer)
*               pixelClock  - Desired pixel clock
* Returns:      Closest pixel clock to desired clock (-1 if not supported)
*
* Description:  Calls the VBE/Core 3.0 interface to determine the closest
*               pixel clock to the requested value. The BIOS will always
*               search for a pixel clock that is no more than 1% below the
*               requested clock or somewhere higher than the clock. If the
*               clock is higher note that it may well be many Mhz higher
*               that requested and the application will have to check that
*               the returned value is suitable for it's needs. This function
*               returns the actual pixel clock that will be programmed by
*               the hardware.
*
*               Note that if the pixel clock will be used with a linear
*               framebuffer mode, make sure you pass in the linear
*               framebuffer flag to this function.
*
*               NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
    RMREGS  regs;

    if (state->VBEVersion >= 0x300) {
	regs.x.ax = 0x4F0B;
	regs.h.bl = 0x00;
	regs.e.ecx = pixelClock;
	regs.x.dx = mode;
	PM_int86(0x10,&regs,&regs);
	if (regs.x.ax == VBE_SUCCESS)
	    return regs.e.ecx;
	}
    return -1;
}

ibool VBEAPI VBE_setDACWidth(int width)
/****************************************************************************
*
* Function:     VBE_setDACWidth
* Parameters:   width   - Width to set the DAC to
* Returns:      True on success, false on failure
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F08;
    regs.h.bl = 0x00;
    regs.h.bh = width;
    PM_int86(0x10,&regs,&regs);
    return regs.x.ax == VBE_SUCCESS;
}

int VBEAPI VBE_getDACWidth(void)
/****************************************************************************
*
* Function:     VBE_getDACWidth
* Returns:      Current width of the palette DAC
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F08;
    regs.h.bl = 0x01;
    PM_int86(0x10,&regs,&regs);
    if (regs.x.ax != VBE_SUCCESS)
	return -1;
    return regs.h.bh;
}

ibool VBEAPI VBE_setPalette(int start,int num,VBE_palette *pal,ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setPalette
* Parameters:   start   - Starting palette index to program
*               num     - Number of palette indexes to program
*               pal     - Palette buffer containing values
*               waitVRT - Wait for vertical retrace flag
* Returns:      True on success, false on failure
*
* Description:  Sets a block of palette registers by calling the VBE 2.0
*               BIOS. This function will fail on VBE 1.2 implementations.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F09;
    regs.h.bl = waitVRT ? 0x80 : 0x00;
    regs.x.cx = num;
    regs.x.dx = start;
    VBE_callESDI(&regs, pal, sizeof(VBE_palette) * num);
    return regs.x.ax == VBE_SUCCESS;
}

void * VBEAPI VBE_getBankedPointer(VBE_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     VBE_getBankedPointer
* Parameters:   modeInfo    - Mode info block for video mode
* Returns:      Selector to the linear framebuffer (0 on failure)
*
* Description:  Returns a near pointer to the VGA framebuffer area.
*
****************************************************************************/
{
    /* We just map the pointer every time, since the pointer will always
     * be in real mode memory, so we wont actually be mapping any real
     * memory.
     *
     * NOTE: We cannot currently map a near pointer to the banked frame
     *       buffer for Watcom Win386, so we create a 16:16 far pointer to
     *       the video memory. All the assembler code will render to the
     *       video memory by loading the selector rather than using a
     *       near pointer.
     */
    ulong seg = (ushort)modeInfo->WinASegment;
    if (seg != 0) {
	if (seg == 0xA000)
	    return (void*)PM_getA0000Pointer();
	else
	    return (void*)PM_mapPhysicalAddr(seg << 4,0xFFFF,true);
	}
    return NULL;
}

#ifndef REALMODE

void * VBEAPI VBE_getLinearPointer(VBE_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     VBE_getLinearPointer
* Parameters:   modeInfo    - Mode info block for video mode
* Returns:      Selector to the linear framebuffer (0 on failure)
*
* Description:  Returns a near pointer to the linear framebuffer for the video
*               mode.
*
****************************************************************************/
{
    static ulong physPtr[MAX_LIN_PTRS] = {0};
    static void *linPtr[MAX_LIN_PTRS] = {0};
    static int numPtrs = 0;
    int i;

    /* Search for an already mapped pointer */
    for (i = 0; i < numPtrs; i++) {
	if (physPtr[i] == modeInfo->PhysBasePtr)
	    return linPtr[i];
	}
    if (numPtrs < MAX_LIN_PTRS) {
	physPtr[numPtrs] = modeInfo->PhysBasePtr;
	linPtr[numPtrs] = PM_mapPhysicalAddr(modeInfo->PhysBasePtr,(state->VBEMemory * 1024L)-1,true);
	return linPtr[numPtrs++];
	}
    return NULL;
}

static void InitPMCode(void)
/****************************************************************************
*
* Function:     InitPMCode  - 32 bit protected mode version
*
* Description:  Finds the address of and relocates the protected mode
*               code block from the VBE 2.0 into a local memory block. The
*               memory block is allocated with malloc() and must be freed
*               with VBE_freePMCode() after graphics processing is complete.
*
*               Note that this buffer _must_ be recopied after each mode set,
*               as the routines will change depending on the underlying
*               video mode.
*
****************************************************************************/
{
    RMREGS      regs;
    RMSREGS     sregs;
    uchar       *code;
    int         pmLen;

    if (!state->pmInfo && state->VBEVersion >= 0x200) {
	regs.x.ax = 0x4F0A;
	regs.x.bx = 0;
	PM_int86x(0x10,&regs,&regs,&sregs);
	if (regs.x.ax != VBE_SUCCESS)
	    return;
	if (VBE_shared)
	    state->pmInfo = PM_mallocShared(regs.x.cx);
	else
	    state->pmInfo = PM_malloc(regs.x.cx);
	if (state->pmInfo == NULL)
	    return;
	state->pmInfo32 = state->pmInfo;
	pmLen = regs.x.cx;

	/* Relocate the block into our local data segment */
	code = PM_mapRealPointer(sregs.es,regs.x.di);
	memcpy(state->pmInfo,code,pmLen);

	/* Now do a sanity check on the information we recieve to ensure
	 * that is is correct. Some BIOS return totally bogus information
	 * in here (Matrox is one)! Under DOS this works OK, but under OS/2
	 * we are screwed.
	 */
	if (state->pmInfo->setWindow >= pmLen ||
	    state->pmInfo->setDisplayStart >= pmLen ||
	    state->pmInfo->setPalette >= pmLen ||
	    state->pmInfo->IOPrivInfo >= pmLen) {
	    if (VBE_shared)
		PM_freeShared(state->pmInfo);
	    else
		PM_free(state->pmInfo);
	    state->pmInfo32 = state->pmInfo = NULL;
	    return;
	    }

	/* Read the IO priveledge info and determine if we need to
	 * pass a selector to MMIO registers to the bank switch code.
	 * Since we no longer support selector allocation, we no longer
	 * support this mechanism so we disable the protected mode
	 * interface in this case.
	 */
	if (state->pmInfo->IOPrivInfo && !state->MMIOSel) {
	    ushort *p = (ushort*)((uchar*)state->pmInfo + state->pmInfo->IOPrivInfo);
	    while (*p != 0xFFFF)
		p++;
	    p++;
	    if (*p != 0xFFFF)
		VBE_freePMCode();
	    }
	}
}

void * VBEAPI VBE_getSetBank(void)
/****************************************************************************
*
* Function:     VBE_getSetBank
* Returns:      Pointer to the 32 VBE 2.0 bit bank switching routine.
*
****************************************************************************/
{
    if (state->VBEVersion >= 0x200) {
	InitPMCode();
	if (state->pmInfo)
	    return (uchar*)state->pmInfo + state->pmInfo->setWindow;
	}
    return NULL;
}

void * VBEAPI VBE_getSetDisplayStart(void)
/****************************************************************************
*
* Function:     VBE_getSetDisplayStart
* Returns:      Pointer to the 32 VBE 2.0 bit CRT start address routine.
*
****************************************************************************/
{
    if (state->VBEVersion >= 0x200) {
	InitPMCode();
	if (state->pmInfo)
	    return (uchar*)state->pmInfo + state->pmInfo->setDisplayStart;
	}
    return NULL;
}

void * VBEAPI VBE_getSetPalette(void)
/****************************************************************************
*
* Function:     VBE_getSetPalette
* Returns:      Pointer to the 32 VBE 2.0 bit palette programming routine.
*
****************************************************************************/
{
    if (state->VBEVersion >= 0x200) {
	InitPMCode();
	if (state->pmInfo)
	    return (uchar*)state->pmInfo + state->pmInfo->setPalette;
	}
    return NULL;
}

void VBEAPI VBE_freePMCode(void)
/****************************************************************************
*
* Function:     VBE_freePMCode
*
* Description:  This routine frees the protected mode code blocks that
*               we copied from the VBE 2.0 interface. This routine must
*               be after you have finished graphics processing to free up
*               the memory occupied by the routines. This is necessary
*               because the PM info memory block must be re-copied after
*               every video mode set from the VBE 2.0 implementation.
*
****************************************************************************/
{
    if (state->pmInfo) {
	if (VBE_shared)
	    PM_freeShared(state->pmInfo);
	else
	    PM_free(state->pmInfo);
	state->pmInfo = NULL;
	state->pmInfo32 = NULL;
	}
}

void VBEAPI VBE_sharePMCode(void)
/****************************************************************************
*
* Function:     VBE_sharePMCode
*
* Description:  Enables internal sharing of the PM code buffer for OS/2.
*
****************************************************************************/
{
    VBE_shared = true;
}

/* Set of code stubs used to build the final bank switch code */

#define VBE20_adjustOffset  7

static uchar VBE20A_bankFunc32_Start[] = {
    0x53,0x51,                  /*  push    ebx,ecx     */
    0x8B,0xD0,                  /*  mov     edx,eax     */
    0x33,0xDB,                  /*  xor     ebx,ebx     */
    0xB1,0x00,                  /*  mov     cl,0        */
    0xD2,0xE2,                  /*  shl     dl,cl       */
    };

static uchar VBE20_bankFunc32_End[] = {
    0x59,0x5B,                  /*  pop     ecx,ebx     */
    };

static uchar bankFunc32[100];

#define copy(p,b,a) memcpy(b,a,sizeof(a)); (p) = (b) + sizeof(a)

ibool VBEAPI VBE_getBankFunc32(int *codeLen,void **bankFunc,int dualBanks,
    int bankAdjust)
/****************************************************************************
*
* Function:     VBE_getBankFunc32
* Parameters:   codeLen     - Place to store length of code
*               bankFunc    - Place to store pointer to bank switch code
*               dualBanks   - True if dual banks are in effect
*               bankAdjust  - Bank shift adjustment factor
* Returns:      True on success, false if not compatible.
*
* Description:  Creates a local 32 bit bank switch function from the
*               VBE 2.0 bank switch code that is compatible with the
*               virtual flat framebuffer devices (does not have a return
*               instruction at the end and takes the bank number in EAX
*               not EDX). Note that this 32 bit code cannot include int 10h
*               instructions, so we can only do this if we have VBE 2.0
*               or later.
*
*               Note that we need to know the length of the 32 bit
*               bank switch function, which the standard VBE 2.0 spec
*               does not provide. In order to support this we have
*               extended the VBE 2.0 state->pmInfo structure in UniVBE 5.2 in a
*               way to support this, and we hope that this will become
*               a VBE 2.0 ammendment.
*
*               Note also that we cannot run the linear framebuffer
*               emulation code with bank switching routines that require
*               a selector to the memory mapped registers passed in ES.
*
****************************************************************************/
{
    int     len;
    uchar   *code;
    uchar   *p;

    InitPMCode();
    if (state->VBEVersion >= 0x200 && state->pmInfo32 && !state->MMIOSel) {
	code = (uchar*)state->pmInfo32 + state->pmInfo32->setWindow;
	if (state->pmInfo32->extensionSig == VBE20_EXT_SIG)
	    len = state->pmInfo32->setWindowLen-1;
	else {
	    /* We are running on a system without the UniVBE 5.2 extension.
	     * We do as best we can by scanning through the code for the
	     * ret function to determine the length. This is not foolproof,
	     * but is the best we can do.
	     */
	    p = code;
	    while (*p != 0xC3)
		p++;
	    len = p - code;
	    }
	if ((len + sizeof(VBE20A_bankFunc32_Start) + sizeof(VBE20_bankFunc32_End)) > sizeof(bankFunc32))
	    PM_fatalError("32-bit bank switch function too long!");
	copy(p,bankFunc32,VBE20A_bankFunc32_Start);
	memcpy(p,code,len);
	p += len;
	copy(p,p,VBE20_bankFunc32_End);
	*codeLen = p - bankFunc32;
	bankFunc32[VBE20_adjustOffset] = (uchar)bankAdjust;
	*bankFunc = bankFunc32;
	return true;
	}
    return false;
}

#endif
