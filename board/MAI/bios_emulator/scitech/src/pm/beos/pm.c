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
* Environment:  BeOS
*
* Description:  Implementation for the OS Portability Manager Library, which
*               contains functions to implement OS specific services in a
*               generic, cross platform API. Porting the OS Portability
*               Manager library is the first step to porting any SciTech
*               products to a new platform.
*
****************************************************************************/

#include "pmapi.h"
#include "drvlib/os/os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: Include any BeOS specific headers here! */

/*--------------------------- Global variables ----------------------------*/

static void (PMAPIP fatalErrorCleanup)(void) = NULL;

/*----------------------------- Implementation ----------------------------*/

void PMAPI PM_init(void)
{
    /* TODO: Do any initialisation in here. This includes getting IOPL */
    /*       access for the process calling PM_init. This will get called */
    /*       more than once. */

    /* TODO: If you support the supplied MTRR register stuff (you need to */
    /*       be at ring 0 for this!), you should initialise it in here. */

/* MTRR_init(); */
}

long PMAPI PM_getOSType(void)
{ return _OS_BEOS; }

int PMAPI PM_getModeType(void)
{ return PM_386; }

void PMAPI PM_backslash(char *s)
{
    uint pos = strlen(s);
    if (s[pos-1] != '/') {
	s[pos] = '/';
	s[pos+1] = '\0';
	}
}

void PMAPI PM_setFatalErrorCleanup(
    void (PMAPIP cleanup)(void))
{
    fatalErrorCleanup = cleanup;
}

void PMAPI PM_fatalError(const char *msg)
{
    /* TODO: If you are running in a GUI environment without a console, */
    /*       this needs to be changed to bring up a fatal error message */
    /*       box and terminate the program. */
    if (fatalErrorCleanup)
	fatalErrorCleanup();
    fprintf(stderr,"%s\n", msg);
    exit(1);
}

void * PMAPI PM_getVESABuf(uint *len,uint *rseg,uint *roff)
{
    /* No BIOS access for the BeOS */
    return NULL;
}

int PMAPI PM_kbhit(void)
{
    /* TODO: This function checks if a key is available to be read. This */
    /*       should be implemented, but is mostly used by the test programs */
    /*       these days. */
    return true;
}

int PMAPI PM_getch(void)
{
    /* TODO: This returns the ASCII code of the key pressed. This */
    /*       should be implemented, but is mostly used by the test programs */
    /*       these days. */
    return 0xD;
}

int PMAPI PM_openConsole(void)
{
    /* TODO: Opens up a fullscreen console for graphics output. If your */
    /*       console does not have graphics/text modes, this can be left */
    /*       empty. The main purpose of this is to disable console switching */
    /*       when in graphics modes if you can switch away from fullscreen */
    /*       consoles (if you want to allow switching, this can be done */
    /*       elsewhere with a full save/restore state of the graphics mode). */
    return 0;
}

int PMAPI PM_getConsoleStateSize(void)
{
    /* TODO: Returns the size of the console state buffer used to save the */
    /*       state of the console before going into graphics mode. This is */
    /*       used to restore the console back to normal when we are done. */
    return 1;
}

void PMAPI PM_saveConsoleState(void *stateBuf,int console_id)
{
    /* TODO: Saves the state of the console into the state buffer. This is */
    /*       used to restore the console back to normal when we are done. */
    /*       We will always restore 80x25 text mode after being in graphics */
    /*       mode, so if restoring text mode is all you need to do this can */
    /*       be left empty. */
}

void PMAPI PM_restoreConsoleState(const void *stateBuf,int console_id)
{
    /* TODO: Restore the state of the console from the state buffer. This is */
    /*       used to restore the console back to normal when we are done. */
    /*       We will always restore 80x25 text mode after being in graphics */
    /*       mode, so if restoring text mode is all you need to do this can */
    /*       be left empty. */
}

void PMAPI PM_closeConsole(int console_id)
{
    /* TODO: Close the console when we are done, going back to text mode. */
}

void PM_setOSCursorLocation(int x,int y)
{
    /* TODO: Set the OS console cursor location to the new value. This is */
    /*       generally used for new OS ports (used mostly for DOS). */
}

void PM_setOSScreenWidth(int width,int height)
{
    /* TODO: Set the OS console screen width. This is generally unused for */
    /*       new OS ports. */
}

ibool PMAPI PM_setRealTimeClockHandler(PM_intHandler ih, int frequency)
{
    /* TODO: Install a real time clock interrupt handler. Normally this */
    /*       will not be supported from most OS'es in user land, so an */
    /*       alternative mechanism is needed to enable software stereo. */
    /*       Hence leave this unimplemented unless you have a high priority */
    /*       mechanism to call the 32-bit callback when the real time clock */
    /*       interrupt fires. */
    return false;
}

void PMAPI PM_setRealTimeClockFrequency(int frequency)
{
    /* TODO: Set the real time clock interrupt frequency. Used for stereo */
    /*       LC shutter glasses when doing software stereo. Usually sets */
    /*       the frequency to around 2048 Hz. */
}

void PMAPI PM_restoreRealTimeClockHandler(void)
{
    /* TODO: Restores the real time clock handler. */
}

char * PMAPI PM_getCurrentPath(
    char *path,
    int maxLen)
{
    return getcwd(path,maxLen);
}

char PMAPI PM_getBootDrive(void)
{ return '/'; }

const char * PMAPI PM_getVBEAFPath(void)
{ return PM_getNucleusConfigPath(); }

const char * PMAPI PM_getNucleusPath(void)
{
    char *env = getenv("NUCLEUS_PATH");
    return env ? env : "/usr/lib/nucleus";
}

const char * PMAPI PM_getNucleusConfigPath(void)
{
    static char path[256];
    strcpy(path,PM_getNucleusPath());
    PM_backslash(path);
    strcat(path,"config");
    return path;
}

const char * PMAPI PM_getUniqueID(void)
{
    /* TODO: Return a unique ID for the machine. If a unique ID is not */
    /*       available, return the machine name. */
    static char buf[128];
    gethostname(buf, 128);
    return buf;
}

const char * PMAPI PM_getMachineName(void)
{
    /* TODO: Return the network machine name for the machine. */
    static char buf[128];
    gethostname(buf, 128);
    return buf;
}

void * PMAPI PM_getBIOSPointer(void)
{
    /* No BIOS access on the BeOS */
    return NULL;
}

void * PMAPI PM_getA0000Pointer(void)
{
    static void *bankPtr;
    if (!bankPtr)
	bankPtr = PM_mapPhysicalAddr(0xA0000,0xFFFF,true);
    return bankPtr;
}

void * PMAPI PM_mapPhysicalAddr(ulong base,ulong limit,ibool isCached)
{
    /* TODO: This function maps a physical memory address to a linear */
    /*       address in the address space of the calling process. */

    /* NOTE: This function *must* be able to handle any phsyical base */
    /*       address, and hence you will have to handle rounding of */
    /*       the physical base address to a page boundary (ie: 4Kb on */
    /*       x86 CPU's) to be able to properly map in the memory */
    /*       region. */

    /* NOTE: If possible the isCached bit should be used to ensure that */
    /*       the PCD (Page Cache Disable) and PWT (Page Write Through) */
    /*       bits are set to disable caching for a memory mapping used */
    /*       for MMIO register access. We also disable caching using */
    /*       the MTRR registers for Pentium Pro and later chipsets so if */
    /*       MTRR support is enabled for your OS then you can safely ignore */
    /*       the isCached flag and always enable caching in the page */
    /*       tables. */
    return NULL;
}

void PMAPI PM_freePhysicalAddr(void *ptr,ulong limit)
{
    /* TODO: This function will free a physical memory mapping previously */
    /*       allocated with PM_mapPhysicalAddr() if at all possible. If */
    /*       you can't free physical memory mappings, simply do nothing. */
}

ulong PMAPI PM_getPhysicalAddr(void *p)
{
    /* TODO: This function should find the physical address of a linear */
    /*       address. */
    return 0xFFFFFFFFUL;
}

void PMAPI PM_sleep(ulong milliseconds)
{
    /* TODO: Put the process to sleep for milliseconds */
}

int PMAPI PM_getCOMPort(int port)
{
    /* TODO: Re-code this to determine real values using the Plug and Play */
    /*       manager for the OS. */
    switch (port) {
	case 0: return 0x3F8;
	case 1: return 0x2F8;
	}
    return 0;
}

int PMAPI PM_getLPTPort(int port)
{
    /* TODO: Re-code this to determine real values using the Plug and Play */
    /*       manager for the OS. */
    switch (port) {
	case 0: return 0x3BC;
	case 1: return 0x378;
	case 2: return 0x278;
	}
    return 0;
}

void * PMAPI PM_mallocShared(long size)
{
    /* TODO: This is used to allocate memory that is shared between process */
    /*       that all access the common Nucleus drivers via a common display */
    /*       driver DLL. If your OS does not support shared memory (or if */
    /*       the display driver does not need to allocate shared memory */
    /*       for each process address space), this should just call PM_malloc. */
    return PM_malloc(size);
}

void PMAPI PM_freeShared(void *ptr)
{
    /* TODO: Free the shared memory block. This will be called in the context */
    /*       of the original calling process that allocated the shared */
    /*       memory with PM_mallocShared. Simply call free if you do not */
    /*       need this. */
    PM_free(ptr);
}

void * PMAPI PM_mapToProcess(void *base,ulong limit)
{
    /* TODO: This function is used to map a physical memory mapping */
    /*       previously allocated with PM_mapPhysicalAddr into the */
    /*       address space of the calling process. If the memory mapping */
    /*       allocated by PM_mapPhysicalAddr is global to all processes, */
    /*       simply return the pointer. */
    return base;
}

void * PMAPI PM_mapRealPointer(uint r_seg,uint r_off)
{
    /* No BIOS access on the BeOS */
    return NULL;
}

void * PMAPI PM_allocRealSeg(uint size,uint *r_seg,uint *r_off)
{
    /* No BIOS access on the BeOS */
    return NULL;
}

void PMAPI PM_freeRealSeg(void *mem)
{
    /* No BIOS access on the BeOS */
}

void PMAPI DPMI_int86(int intno, DPMI_regs *regs)
{
    /* No BIOS access on the BeOS */
}

int PMAPI PM_int86(int intno, RMREGS *in, RMREGS *out)
{
    /* No BIOS access on the BeOS */
    return 0;
}

int PMAPI PM_int86x(int intno, RMREGS *in, RMREGS *out,
    RMSREGS *sregs)
{
    /* No BIOS access on the BeOS */
    return 0;
}

void PMAPI PM_callRealMode(uint seg,uint off, RMREGS *in,
    RMSREGS *sregs)
{
    /* No BIOS access on the BeOS */
}

void PMAPI PM_availableMemory(ulong *physical,ulong *total)
{
    /* TODO: Report the amount of available memory, both the amount of */
    /*       physical memory left and the amount of virtual memory left. */
    /*       If the OS does not provide these services, report 0's. */
    *physical = *total = 0;
}

void * PMAPI PM_allocLockedMem(uint size,ulong *physAddr,ibool contiguous,ibool below16Meg)
{
    /* TODO: Allocate a block of locked, physical memory of the specified */
    /*       size. This is used for bus master operations. If this is not */
    /*       supported by the OS, return NULL and bus mastering will not */
    /*       be used. */
    return NULL;
}

void PMAPI PM_freeLockedMem(void *p,uint size,ibool contiguous)
{
    /* TODO: Free a memory block allocated with PM_allocLockedMem. */
}

void PMAPI PM_setBankA(int bank)
{
    /* No BIOS access on the BeOS */
}

void PMAPI PM_setBankAB(int bank)
{
    /* No BIOS access on the BeOS */
}

void PMAPI PM_setCRTStart(int x,int y,int waitVRT)
{
    /* No BIOS access on the BeOS */
}

ibool PMAPI PM_enableWriteCombine(ulong base,ulong length,uint type)
{
    /* TODO: This function should enable Pentium Pro and Pentium II MTRR */
    /*       write combining for the passed in physical memory base address */
    /*       and length. Normally this is done via calls to an OS specific */
    /*       device driver as this can only be done at ring 0. */
    /* */
    /* NOTE: This is a *very* important function to implement! If you do */
    /*       not implement, graphics performance on the latest Intel chips */
    /*       will be severly impaired. For sample code that can be used */
    /*       directly in a ring 0 device driver, see the MSDOS implementation */
    /*       which includes assembler code to do this directly (if the */
    /*       program is running at ring 0). */
    return false;
}

ibool PMAPI PM_doBIOSPOST(ushort axVal,ulong BIOSPhysAddr,void *mappedBIOS)
{
    /* TODO: This function is used to run the BIOS POST code on a secondary */
    /*       controller to initialise it for use. This is not necessary */
    /*       for multi-controller operation, but it will make it a lot */
    /*       more convenicent for end users (otherwise they have to boot */
    /*       the system once with the secondary controller as primary, and */
    /*       then boot with both controllers installed). */
    /* */
    /*       Even if you don't support full BIOS access, it would be */
    /*       adviseable to be able to POST the secondary controllers in the */
    /*       system using this function as a minimum requirement. Some */
    /*       graphics hardware has registers that contain values that only */
    /*       the BIOS knows about, which makes bring up a card from cold */
    /*       reset difficult if the BIOS has not POST'ed it. */
    return false;
}

/****************************************************************************
REMARKS:
Function to find the first file matching a search criteria in a directory.
****************************************************************************/
ulong PMAPI PM_findFirstFile(
    const char *filename,
    PM_findData *findData)
{
    (void)filename;
    (void)findData;
    return PM_FILE_INVALID;
}

/****************************************************************************
REMARKS:
Function to find the next file matching a search criteria in a directory.
****************************************************************************/
ibool PMAPI PM_findNextFile(
    ulong handle,
    PM_findData *findData)
{
    (void)handle;
    (void)findData;
    return false;
}

/****************************************************************************
REMARKS:
Function to close the find process
****************************************************************************/
void PMAPI PM_findClose(
    ulong handle)
{
    (void)handle;
}

/****************************************************************************
REMARKS:
Function to determine if a drive is a valid drive or not. Under Unix this
function will return false for anything except a value of 3 (considered
the root drive, and equivalent to C: for non-Unix systems). The drive
numbering is:

    1   - Drive A:
    2   - Drive B:
    3   - Drive C:
    etc

****************************************************************************/
ibool PMAPI PM_driveValid(
    char drive)
{
    if (drive == 3)
	return true;
    return false;
}

/****************************************************************************
REMARKS:
Function to get the current working directory for the specififed drive.
Under Unix this will always return the current working directory regardless
of what the value of 'drive' is.
****************************************************************************/
void PMAPI PM_getdcwd(
    int drive,
    char *dir,
    int len)
{
    (void)drive;
    getcwd(dir,len);
}

/****************************************************************************
REMARKS:
Function to change the file attributes for a specific file.
****************************************************************************/
void PMAPI PM_setFileAttr(
    const char *filename,
    uint attrib)
{
    /* TODO: Set the file attributes for a file */
    (void)filename;
    (void)attrib;
}
