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
* Environment:  RTTarget-32
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
#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#ifdef  __BORLANDC__
#pragma warn -par
#endif

/*--------------------------- Global variables ----------------------------*/

static void     (PMAPIP fatalErrorCleanup)(void) = NULL;

/*----------------------------- Implementation ----------------------------*/

void MTRR_init(void);

/****************************************************************************
REMARKS:
Initialise the PM library.
****************************************************************************/
void PMAPI PM_init(void)
{
    /* TODO: dO any special init code in here. */
    MTRR_init();
}

/****************************************************************************
REMARKS:
Return the operating system type identifier.
****************************************************************************/
long PMAPI PM_getOSType(void)
{
    return _OS_RTTARGET;
}

/****************************************************************************
REMARKS:
Return the runtime type identifier.
****************************************************************************/
int PMAPI PM_getModeType(void)
{
    return PM_386;
}

/****************************************************************************
REMARKS:
Add a file directory separator to the end of the filename.
****************************************************************************/
void PMAPI PM_backslash(
    char *s)
{
    uint pos = strlen(s);
    if (s[pos-1] != '\\') {
	s[pos] = '\\';
	s[pos+1] = '\0';
	}
}

/****************************************************************************
REMARKS:
Add a user defined PM_fatalError cleanup function.
****************************************************************************/
void PMAPI PM_setFatalErrorCleanup(
    void (PMAPIP cleanup)(void))
{
    fatalErrorCleanup = cleanup;
}

/****************************************************************************
REMARKS:
Report a fatal error condition and halt the program.
****************************************************************************/
void PMAPI PM_fatalError(
    const char *msg)
{
    if (fatalErrorCleanup)
	fatalErrorCleanup();
    /* TODO: Display a fatal error message and exit! */
/*  MessageBox(NULL,msg,"Fatal Error!", MB_ICONEXCLAMATION); */
    exit(1);
}

/****************************************************************************
REMARKS:
Allocate the real mode VESA transfer buffer for communicating with the BIOS.
****************************************************************************/
void * PMAPI PM_getVESABuf(
    uint *len,
    uint *rseg,
    uint *roff)
{
    /* No BIOS access for the RTTarget */
    return NULL;
}

/****************************************************************************
REMARKS:
Check if a key has been pressed.
****************************************************************************/
int PMAPI PM_kbhit(void)
{
    /* TODO: Need to check if a key is waiting on the keyboard queue */
    return true;
}

/****************************************************************************
REMARKS:
Wait for and return the next keypress.
****************************************************************************/
int PMAPI PM_getch(void)
{
    /* TODO: Need to obtain the next keypress, and block until one is hit */
    return 0xD;
}

/****************************************************************************
REMARKS:
Set the location of the OS console cursor.
****************************************************************************/
void PM_setOSCursorLocation(
    int x,
    int y)
{
    /* Nothing to do for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Set the width of the OS console.
****************************************************************************/
void PM_setOSScreenWidth(
    int width,
    int height)
{
    /* Nothing to do for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Set the real time clock handler (used for software stereo modes).
****************************************************************************/
ibool PMAPI PM_setRealTimeClockHandler(
    PM_intHandler ih,
    int frequency)
{
    /* Not supported for RTTarget-32 */
    return false;
}

/****************************************************************************
REMARKS:
Set the real time clock frequency (for stereo modes).
****************************************************************************/
void PMAPI PM_setRealTimeClockFrequency(
    int frequency)
{
    /* Not supported under RTTarget-32 */
}

/****************************************************************************
REMARKS:
Restore the original real time clock handler.
****************************************************************************/
void PMAPI PM_restoreRealTimeClockHandler(void)
{
    /* Not supported under RTTarget-32 */
}

/****************************************************************************
REMARKS:
Return the current operating system path or working directory.
****************************************************************************/
char * PMAPI PM_getCurrentPath(
    char *path,
    int maxLen)
{
    return getcwd(path,maxLen);
}

/****************************************************************************
REMARKS:
Return the drive letter for the boot drive.
****************************************************************************/
char PMAPI PM_getBootDrive(void)
{
    return 'c';
}

/****************************************************************************
REMARKS:
Return the path to the VBE/AF driver files.
****************************************************************************/
const char * PMAPI PM_getVBEAFPath(void)
{
    return "c:\\";
}

/****************************************************************************
REMARKS:
Return the path to the Nucleus driver files.
****************************************************************************/
const char * PMAPI PM_getNucleusPath(void)
{
    /* TODO: Point this at the path when the Nucleus drivers will be found */
    return "c:\\nucleus";
}

/****************************************************************************
REMARKS:
Return the path to the Nucleus configuration files.
****************************************************************************/
const char * PMAPI PM_getNucleusConfigPath(void)
{
    static char path[256];
    strcpy(path,PM_getNucleusPath());
    PM_backslash(path);
    strcat(path,"config");
    return path;
}

/****************************************************************************
REMARKS:
Return a unique identifier for the machine if possible.
****************************************************************************/
const char * PMAPI PM_getUniqueID(void)
{
    return PM_getMachineName();
}

/****************************************************************************
REMARKS:
Get the name of the machine on the network.
****************************************************************************/
const char * PMAPI PM_getMachineName(void)
{
    /* Not necessary for RTTarget-32 */
    return "Unknown";
}

/****************************************************************************
REMARKS:
Return a pointer to the real mode BIOS data area.
****************************************************************************/
void * PMAPI PM_getBIOSPointer(void)
{
    /* Not used for RTTarget-32 */
    return NULL;
}

/****************************************************************************
REMARKS:
Return a pointer to 0xA0000 physical VGA graphics framebuffer.
****************************************************************************/
void * PMAPI PM_getA0000Pointer(void)
{
    static void *bankPtr;
    if (!bankPtr)
	bankPtr = PM_mapPhysicalAddr(0xA0000,0xFFFF,true);
    return bankPtr;
}

/****************************************************************************
REMARKS:
Map a physical address to a linear address in the callers process.
****************************************************************************/
void * PMAPI PM_mapPhysicalAddr(
    ulong base,
    ulong limit,
    ibool isCached)
{
    /* TODO: Map a physical memory address to a linear address */
    return NULL;
}

/****************************************************************************
REMARKS:
Free a physical address mapping allocated by PM_mapPhysicalAddr.
****************************************************************************/
void PMAPI PM_freePhysicalAddr(
    void *ptr,
    ulong limit)
{
    /* TODO: Free the physical address mapping */
}

ulong PMAPI PM_getPhysicalAddr(void *p)
{
    /* TODO: This function should find the physical address of a linear */
    /*       address. */
    return 0xFFFFFFFFUL;
}

void PMAPI PM_sleep(ulong milliseconds)
{
    Sleep(milliseconds);
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

/****************************************************************************
REMARKS:
Allocate a block of (unnamed) shared memory.
****************************************************************************/
void * PMAPI PM_mallocShared(
    long size)
{
    return PM_malloc(size);
}

/****************************************************************************
REMARKS:
Free a block of shared memory.
****************************************************************************/
void PMAPI PM_freeShared(
    void *ptr)
{
    PM_free(ptr);
}

/****************************************************************************
REMARKS:
Map a linear memory address to the calling process address space. The
address will have been allocated in another process using the
PM_mapPhysicalAddr function.
****************************************************************************/
void * PMAPI PM_mapToProcess(
    void *base,
    ulong limit)
{
    return base;
}

/****************************************************************************
REMARKS:
Map a real mode pointer to a protected mode pointer.
****************************************************************************/
void * PMAPI PM_mapRealPointer(
    uint r_seg,
    uint r_off)
{
    /* Not used for RTTarget-32 */
    return NULL;
}

/****************************************************************************
REMARKS:
Allocate a block of real mode memory
****************************************************************************/
void * PMAPI PM_allocRealSeg(
    uint size,
    uint *r_seg,
    uint *r_off)
{
    /* Not used for RTTarget-32 */
    return NULL;
}

/****************************************************************************
REMARKS:
Free a block of real mode memory.
****************************************************************************/
void PMAPI PM_freeRealSeg(
    void *mem)
{
    /* Not used for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Issue a real mode interrupt (parameters in DPMI compatible structure)
****************************************************************************/
void PMAPI DPMI_int86(
    int intno,
    DPMI_regs *regs)
{
    /* Not used for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Issue a real mode interrupt.
****************************************************************************/
int PMAPI PM_int86(
    int intno,
    RMREGS *in,
    RMREGS *out)
{
    /* Not used for RTTarget-32 */
    return 0;
}

/****************************************************************************
REMARKS:
Issue a real mode interrupt.
****************************************************************************/
int PMAPI PM_int86x(
    int intno,
    RMREGS *in,
    RMREGS *out,
    RMSREGS *sregs)
{
    /* Not used for RTTarget-32 */
    return 0;
}

/****************************************************************************
REMARKS:
Call a real mode far function.
****************************************************************************/
void PMAPI PM_callRealMode(
    uint seg,
    uint off,
    RMREGS *in,
    RMSREGS *sregs)
{
    /* Not used for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Return the amount of available memory.
****************************************************************************/
void PMAPI PM_availableMemory(
    ulong *physical,
    ulong *total)
{
    /* TODO: Figure out how to determine the available memory. Not entirely */
    /*       critical so returning 0 is OK. */
    *physical = *total = 0;
}

/****************************************************************************
REMARKS:
Allocate a block of locked, physical memory for DMA operations.
****************************************************************************/
void * PMAPI PM_allocLockedMem(
    uint size,
    ulong *physAddr,
    ibool contiguous,
    ibool below16M)
{
    /* TODO: Allocate a block of locked, phsyically contigous memory for DMA */
    return 0;
}

/****************************************************************************
REMARKS:
Free a block of locked physical memory.
****************************************************************************/
void PMAPI PM_freeLockedMem(
    void *p,
    uint size,

    ibool contiguous)
{
    /* TODO: Free a locked memory buffer */
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display banks.
****************************************************************************/
void PMAPI PM_setBankA(
    int bank)
{
    /* Not used for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display banks.
****************************************************************************/
void PMAPI PM_setBankAB(
    int bank)
{
    /* Not used for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display start address.
****************************************************************************/
void PMAPI PM_setCRTStart(
    int x,
    int y,
    int waitVRT)
{
    /* Not used for RTTarget-32 */
}

/****************************************************************************
REMARKS:
Execute the POST on the secondary BIOS for a controller.
****************************************************************************/
ibool PMAPI PM_doBIOSPOST(
    ushort axVal,
    ulong BIOSPhysAddr,
    void *mappedBIOS)
{
    /* Not used for RTTarget-32 */
    return false;
}

PM_MODULE PMAPI PM_loadLibrary(
    const char *szDLLName)
{
    /* TODO: Implement this to load shared libraries! */
    (void)szDLLName;
    return NULL;
}

void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    /* TODO: Implement this! */
    (void)hModule;
    (void)szProcName;
    return NULL;
}

void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    /* TODO: Implement this! */
    (void)hModule;
}

/****************************************************************************
REMARKS:
Function to find the first file matching a search criteria in a directory.
****************************************************************************/
ulong PMAPI PM_findFirstFile(
    const char *filename,
    PM_findData *findData)
{
    /* TODO: This function should start a directory enumeration search */
    /*       given the filename (with wildcards). The data should be */
    /*       converted and returned in the findData standard form. */
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
    /* TODO: This function should find the next file in directory enumeration */
    /*       search given the search criteria defined in the call to */
    /*       PM_findFirstFile. The data should be converted and returned */
    /*       in the findData standard form. */
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
    /* TODO: This function should close the find process. This may do */
    /*       nothing for some OS'es. */
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

/****************************************************************************
REMARKS:
Function to create a directory.
****************************************************************************/
ibool PMAPI PM_mkdir(
    const char *filename)
{
    return mkdir(filename) == 0;
}

/****************************************************************************
REMARKS:
Function to remove a directory.
****************************************************************************/
ibool PMAPI PM_rmdir(
    const char *filename)
{
    return rmdir(filename) == 0;
}
