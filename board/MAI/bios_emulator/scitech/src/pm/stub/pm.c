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
* Environment:  *** TODO: ADD YOUR OS ENVIRONMENT NAME HERE ***
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

/* TODO: Include any OS specific headers here! */

/*--------------------------- Global variables ----------------------------*/

/* TODO: If you support access to the BIOS, the following VESABuf globals */
/*       keep track of a single VESA transfer buffer. If you don't support */
/*       access to the BIOS, remove these variables. */

static uint VESABuf_len = 1024;     /* Length of the VESABuf buffer     */
static void *VESABuf_ptr = NULL;    /* Near pointer to VESABuf          */
static uint VESABuf_rseg;           /* Real mode segment of VESABuf     */
static uint VESABuf_roff;           /* Real mode offset of VESABuf      */

static void (PMAPIP fatalErrorCleanup)(void) = NULL;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Initialise the PM library.
****************************************************************************/
void PMAPI PM_init(void)
{
    /* TODO: Do any initialisation in here. This includes getting IOPL */
    /*       access for the process calling PM_init. This will get called */
    /*       more than once. */

    /* TODO: If you support the supplied MTRR register stuff (you need to */
    /*       be at ring 0 for this!), you should initialise it in here. */

/* MTRR_init(); */
}

/****************************************************************************
REMARKS:
Return the operating system type identifier.
****************************************************************************/
long PMAPI PM_getOSType(void)
{
    /* TODO: Change this to return the define for your OS from drvlib/os.h */
    return _OS_MYOS;
}

/****************************************************************************
REMARKS:
Return the runtime type identifier (always PM_386 for protected mode)
****************************************************************************/
int PMAPI PM_getModeType(void)
{ return PM_386; }

/****************************************************************************
REMARKS:
Add a file directory separator to the end of the filename.
****************************************************************************/
void PMAPI PM_backslash(
    char *s)
{
    uint pos = strlen(s);
    if (s[pos-1] != '/') {
	s[pos] = '/';
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
    /* TODO: If you are running in a GUI environment without a console, */
    /*       this needs to be changed to bring up a fatal error message */
    /*       box and terminate the program. */
    if (fatalErrorCleanup)
	fatalErrorCleanup();
    fprintf(stderr,"%s\n", msg);
    exit(1);
}

/****************************************************************************
REMARKS:
Exit handler to kill the VESA transfer buffer.
****************************************************************************/
static void ExitVBEBuf(void)
{
    /* TODO: If you do not have BIOS access, remove this function. */
    if (VESABuf_ptr)
	PM_freeRealSeg(VESABuf_ptr);
    VESABuf_ptr = 0;
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
    /* TODO: If you do not have BIOS access, simply delete the guts of */
    /*       this function and return NULL. */
    if (!VESABuf_ptr) {
	/* Allocate a global buffer for communicating with the VESA VBE */
	if ((VESABuf_ptr = PM_allocRealSeg(VESABuf_len, &VESABuf_rseg, &VESABuf_roff)) == NULL)
	    return NULL;
	atexit(ExitVBEBuf);
	}
    *len = VESABuf_len;
    *rseg = VESABuf_rseg;
    *roff = VESABuf_roff;
    return VESABuf_ptr;
}

/****************************************************************************
REMARKS:
Check if a key has been pressed.
****************************************************************************/
int PMAPI PM_kbhit(void)
{
    /* TODO: This function checks if a key is available to be read. This */
    /*       should be implemented, but is mostly used by the test programs */
    /*       these days. */
    return true;
}

/****************************************************************************
REMARKS:
Wait for and return the next keypress.
****************************************************************************/
int PMAPI PM_getch(void)
{
    /* TODO: This returns the ASCII code of the key pressed. This */
    /*       should be implemented, but is mostly used by the test programs */
    /*       these days. */
    return 0xD;
}

/****************************************************************************
REMARKS:
Open a fullscreen console mode for output.
****************************************************************************/
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

/****************************************************************************
REMARKS:
Return the size of the state buffer used to save the console state.
****************************************************************************/
int PMAPI PM_getConsoleStateSize(void)
{
    /* TODO: Returns the size of the console state buffer used to save the */
    /*       state of the console before going into graphics mode. This is */
    /*       used to restore the console back to normal when we are done. */
    return 1;
}

/****************************************************************************
REMARKS:
Save the state of the console into the state buffer.
****************************************************************************/
void PMAPI PM_saveConsoleState(
    void *stateBuf,
    int console_id)
{
    /* TODO: Saves the state of the console into the state buffer. This is */
    /*       used to restore the console back to normal when we are done. */
    /*       We will always restore 80x25 text mode after being in graphics */
    /*       mode, so if restoring text mode is all you need to do this can */
    /*       be left empty. */
}

/****************************************************************************
REMARKS:
Restore the state of the console from the state buffer.
****************************************************************************/
void PMAPI PM_restoreConsoleState(
    const void *stateBuf,
    int console_id)
{
    /* TODO: Restore the state of the console from the state buffer. This is */
    /*       used to restore the console back to normal when we are done. */
    /*       We will always restore 80x25 text mode after being in graphics */
    /*       mode, so if restoring text mode is all you need to do this can */
    /*       be left empty. */
}

/****************************************************************************
REMARKS:
Close the console and return to non-fullscreen console mode.
****************************************************************************/
void PMAPI PM_closeConsole(
    int console_id)
{
    /* TODO: Close the console when we are done, going back to text mode. */
}

/****************************************************************************
REMARKS:
Set the location of the OS console cursor.
****************************************************************************/
void PM_setOSCursorLocation(
    int x,
    int y)
{
    /* TODO: Set the OS console cursor location to the new value. This is */
    /*       generally used for new OS ports (used mostly for DOS). */
}

/****************************************************************************
REMARKS:
Set the width of the OS console.
****************************************************************************/
void PM_setOSScreenWidth(
    int width,
    int height)
{
    /* TODO: Set the OS console screen width. This is generally unused for */
    /*       new OS ports. */
}

/****************************************************************************
REMARKS:
Set the real time clock handler (used for software stereo modes).
****************************************************************************/
ibool PMAPI PM_setRealTimeClockHandler(
    PM_intHandler ih,
    int frequency)
{
    /* TODO: Install a real time clock interrupt handler. Normally this */
    /*       will not be supported from most OS'es in user land, so an */
    /*       alternative mechanism is needed to enable software stereo. */
    /*       Hence leave this unimplemented unless you have a high priority */
    /*       mechanism to call the 32-bit callback when the real time clock */
    /*       interrupt fires. */
    return false;
}

/****************************************************************************
REMARKS:
Set the real time clock frequency (for stereo modes).
****************************************************************************/
void PMAPI PM_setRealTimeClockFrequency(
    int frequency)
{
    /* TODO: Set the real time clock interrupt frequency. Used for stereo */
    /*       LC shutter glasses when doing software stereo. Usually sets */
    /*       the frequency to around 2048 Hz. */
}

/****************************************************************************
REMARKS:
Restore the original real time clock handler.
****************************************************************************/
void PMAPI PM_restoreRealTimeClockHandler(void)
{
    /* TODO: Restores the real time clock handler. */
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
    /* TODO: Return the boot drive letter for the OS. Normally this is 'c' */
    /*       for DOS based OS'es and '/' for Unices. */
    return '/';
}

/****************************************************************************
REMARKS:
Return the path to the VBE/AF driver files (legacy and not used).
****************************************************************************/
const char * PMAPI PM_getVBEAFPath(void)
{
    return PM_getNucleusConfigPath();
}

/****************************************************************************
REMARKS:
Return the path to the Nucleus driver files.
****************************************************************************/
const char * PMAPI PM_getNucleusPath(void)
{
    /* TODO: Change this to the default path to Nucleus driver files. The */
    /*       following is the default for Unices. */
    char *env = getenv("NUCLEUS_PATH");
    return env ? env : "/usr/lib/nucleus";
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
    /* TODO: Return a unique ID for the machine. If a unique ID is not */
    /*       available, return the machine name. */
    static char buf[128];
    gethostname(buf, 128);
    return buf;
}

/****************************************************************************
REMARKS:
Get the name of the machine on the network.
****************************************************************************/
const char * PMAPI PM_getMachineName(void)
{
    /* TODO: Return the network machine name for the machine. */
    static char buf[128];
    gethostname(buf, 128);
    return buf;
}

/****************************************************************************
REMARKS:
Return a pointer to the real mode BIOS data area.
****************************************************************************/
void * PMAPI PM_getBIOSPointer(void)
{
    /* TODO: This returns a pointer to the real mode BIOS data area. If you */
    /*       do not support BIOS access, you can simply return NULL here. */
    if (!zeroPtr)
	zeroPtr = PM_mapPhysicalAddr(0,0xFFFFF,true);
    return (void*)(zeroPtr + 0x400);
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

/****************************************************************************
REMARKS:
Free a physical address mapping allocated by PM_mapPhysicalAddr.
****************************************************************************/
void PMAPI PM_freePhysicalAddr(
    void *ptr,
    ulong limit)
{
    /* TODO: This function will free a physical memory mapping previously */
    /*       allocated with PM_mapPhysicalAddr() if at all possible. If */
    /*       you can't free physical memory mappings, simply do nothing. */
}

/****************************************************************************
REMARKS:
Find the physical address of a linear memory address in current process.
****************************************************************************/
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

/****************************************************************************
REMARKS:
Allocate a block of (unnamed) shared memory.
****************************************************************************/
void * PMAPI PM_mallocShared(
    long size)
{
    /* TODO: This is used to allocate memory that is shared between process */
    /*       that all access the common Nucleus drivers via a common display */
    /*       driver DLL. If your OS does not support shared memory (or if */
    /*       the display driver does not need to allocate shared memory */
    /*       for each process address space), this should just call PM_malloc. */
    return PM_malloc(size);
}

/****************************************************************************
REMARKS:
Free a block of shared memory.
****************************************************************************/
void PMAPI PM_freeShared(
    void *ptr)
{
    /* TODO: Free the shared memory block. This will be called in the context */
    /*       of the original calling process that allocated the shared */
    /*       memory with PM_mallocShared. Simply call PM_free if you do not */
    /*       need this. */
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
    /* TODO: This function is used to map a physical memory mapping */
    /*       previously allocated with PM_mapPhysicalAddr into the */
    /*       address space of the calling process. If the memory mapping */
    /*       allocated by PM_mapPhysicalAddr is global to all processes, */
    /*       simply return the pointer. */

    /* NOTE: This function must also handle rounding to page boundaries, */
    /*       since this function is used to map in shared memory buffers */
    /*       allocated with PM_mapPhysicalAddr(). Hence if you aligned */
    /*       the physical address above, then you also need to do it here. */
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
    /* TODO: This function maps a real mode memory pointer into the */
    /*       calling processes address space as a 32-bit near pointer. If */
    /*       you do not support BIOS access, simply return NULL here. */
    if (!zeroPtr)
	zeroPtr = PM_mapPhysicalAddr(0,0xFFFFF);
    return (void*)(zeroPtr + MK_PHYS(r_seg,r_off));
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
    /* TODO: This function allocates a block of real mode memory for the */
    /*       calling process used to communicate with real mode BIOS */
    /*       functions. If you do not support BIOS access, simply return */
    /*       NULL here. */
    return NULL;
}

/****************************************************************************
REMARKS:
Free a block of real mode memory.
****************************************************************************/
void PMAPI PM_freeRealSeg(
    void *mem)
{
    /* TODO: Frees a previously allocated real mode memory block. If you */
    /*       do not support BIOS access, this function should be empty. */
}

/****************************************************************************
REMARKS:
Issue a real mode interrupt (parameters in DPMI compatible structure)
****************************************************************************/
void PMAPI DPMI_int86(
    int intno,
    DPMI_regs *regs)
{
    /* TODO: This function calls the real mode BIOS using the passed in */
    /*       register structure. If you do not support real mode BIOS */
    /*       access, this function should be empty. */
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
    /* TODO: This function calls the real mode BIOS using the passed in */
    /*       register structure. If you do not support real mode BIOS */
    /*       access, this function should return 0. */
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
    /* TODO: This function calls the real mode BIOS using the passed in */
    /*       register structure. If you do not support real mode BIOS */
    /*       access, this function should return 0. */
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
    /* TODO: This function calls a real mode far function with a far call. */
    /*       If you do not support BIOS access, this function should be */
    /*       empty. */
}

/****************************************************************************
REMARKS:
Return the amount of available memory.
****************************************************************************/
void PMAPI PM_availableMemory(
    ulong *physical,
    ulong *total)
{
    /* TODO: Report the amount of available memory, both the amount of */
    /*       physical memory left and the amount of virtual memory left. */
    /*       If the OS does not provide these services, report 0's. */
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
    /* TODO: Allocate a block of locked, physical memory of the specified */
    /*       size. This is used for bus master operations. If this is not */
    /*       supported by the OS, return NULL and bus mastering will not */
    /*       be used. */
    return NULL;
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
    /* TODO: Free a memory block allocated with PM_allocLockedMem. */
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display banks.
****************************************************************************/
void PMAPI PM_setBankA(
    int bank)
{
    RMREGS  regs;

    /* TODO: This does a bank switch function by calling the real mode */
    /*       VESA BIOS. If you do not support BIOS access, this function should */
    /*       be empty. */
    regs.x.ax = 0x4F05;
    regs.x.bx = 0x0000;
    regs.x.dx = bank;
    PM_int86(0x10,&regs,&regs);
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display banks.
****************************************************************************/
void PMAPI PM_setBankAB(
    int bank)
{
    RMREGS  regs;

    /* TODO: This does a bank switch function by calling the real mode */
    /*       VESA BIOS. If you do not support BIOS access, this function should */
    /*       be empty. */
    regs.x.ax = 0x4F05;
    regs.x.bx = 0x0000;
    regs.x.dx = bank;
    PM_int86(0x10,&regs,&regs);
    regs.x.ax = 0x4F05;
    regs.x.bx = 0x0001;
    regs.x.dx = bank;
    PM_int86(0x10,&regs,&regs);
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
    RMREGS  regs;

    /* TODO: This changes the display start address by calling the real mode */
    /*       VESA BIOS. If you do not support BIOS access, this function */
    /*       should be empty. */
    regs.x.ax = 0x4F07;
    regs.x.bx = waitVRT;
    regs.x.cx = x;
    regs.x.dx = y;
    PM_int86(0x10,&regs,&regs);
}

/****************************************************************************
REMARKS:
Enable write combining for the memory region.
****************************************************************************/
ibool PMAPI PM_enableWriteCombine(
    ulong base,
    ulong length,
    uint type)
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

/****************************************************************************
REMARKS:
Execute the POST on the secondary BIOS for a controller.
****************************************************************************/
ibool PMAPI PM_doBIOSPOST(
    ushort axVal,
    ulong BIOSPhysAddr,
    void *mappedBIOS)
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
Load an OS specific shared library or DLL. If the OS does not support
shared libraries, simply return NULL.
****************************************************************************/
PM_MODULE PMAPI PM_loadLibrary(
    const char *szDLLName)
{
    /* TODO: This function should load a native shared library from disk */
    /*       given the path to the library. */
    (void)szDLLName;
    return NULL;
}

/****************************************************************************
REMARKS:
Get the address of a named procedure from a shared library.
****************************************************************************/
void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    /* TODO: This function should return the address of a named procedure */
    /*       from a native shared library. */
    (void)hModule;
    (void)szProcName;
    return NULL;
}

/****************************************************************************
REMARKS:
Unload a shared library.
****************************************************************************/
void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    /* TODO: This function free a previously loaded native shared library. */
    (void)hModule;
}

/****************************************************************************
REMARKS:
Enable requested I/O privledge level (usually only to set to a value of
3, and then restore it back again). If the OS is protected this function
must be implemented in order to enable I/O port access for ring 3
applications. The function should return the IOPL level active before
the switch occurred so it can be properly restored.
****************************************************************************/
int PMAPI PM_setIOPL(
    int level)
{
    /* TODO: This function should enable IOPL for the task (if IOPL is */
    /*       not always enabled for the app through some other means). */
    return level;
}

/****************************************************************************
REMARKS:
Function to find the first file matching a search criteria in a directory.
****************************************************************************/
void *PMAPI PM_findFirstFile(
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
    void *handle,
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
    void *handle)
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
