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
* Environment:  32 bit SMX embedded systems development.
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
#include "ztimerc.h"
#include "event.h"
#include "mtrr.h"
#include "pm_help.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#ifdef  __GNUC__
#include <unistd.h>
#include <sys/nearptr.h>
#include <sys/stat.h>
#else
#include <direct.h>
#endif
#ifdef  __BORLANDC__
#pragma warn -par
#endif

/*--------------------------- Global variables ----------------------------*/

typedef struct {
    int     oldMode;
    int     old50Lines;
    } DOS_stateBuf;

#define MAX_RM_BLOCKS   10

static struct {
    void    *p;
    uint    tag;
    } rmBlocks[MAX_RM_BLOCKS];

static uint     VESABuf_len = 1024;     /* Length of the VESABuf buffer     */
static void     *VESABuf_ptr = NULL;    /* Near pointer to VESABuf          */
static uint     VESABuf_rseg;           /* Real mode segment of VESABuf     */
static uint     VESABuf_roff;           /* Real mode offset of VESABuf      */
static void     (PMAPIP fatalErrorCleanup)(void) = NULL;
ushort _VARAPI  _PM_savedDS = 0;
static ulong    PDB = 0,*pPDB = NULL;
static uint     VXD_version = -1;

/*----------------------------- Implementation ----------------------------*/

ulong   _ASMAPI _PM_getPDB(void);
void    _ASMAPI _PM_VxDCall(VXD_regs *regs,uint off,uint sel);

/****************************************************************************
REMARKS:
External function to call the PMHELP helper VxD.
****************************************************************************/
void PMAPI PM_VxDCall(
    VXD_regs *regs)
{
}

/****************************************************************************
RETURNS:
BCD coded version number of the VxD, or 0 if not loaded (ie: 0x202 - 2.2)

REMARKS:
This function gets the version number for the VxD that we have connected to.
****************************************************************************/
uint PMAPI PMHELP_getVersion(void)
{
    return VXD_version = 0;
}

void PMAPI PM_init(void)
{
#ifndef REALMODE
    MTRR_init();
#endif
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
int PMAPI PM_enableWriteCombine(
    ulong base,
    ulong size,
    uint type)
{
#ifndef REALMODE
    return MTRR_enableWriteCombine(base,size,type);
#else
    return PM_MTRR_NOT_SUPPORTED;
#endif
}

ibool PMAPI PM_haveBIOSAccess(void)
{ return false; }

long PMAPI PM_getOSType(void)
{ return _OS_SMX; }

int PMAPI PM_getModeType(void)
{ return PM_386; }

void PMAPI PM_backslash(char *s)
{
    uint pos = strlen(s);
    if (s[pos-1] != '\\') {
	s[pos] = '\\';
	s[pos+1] = '\0';
	}
}

void PMAPI PM_setFatalErrorCleanup(
    void (PMAPIP cleanup)(void))
{
    fatalErrorCleanup = cleanup;
}

void MGLOutput(char *);

void PMAPI PM_fatalError(const char *msg)
{
    if (fatalErrorCleanup)
	fatalErrorCleanup();
    MGLOutput(msg);
/* No support for fprintf() under smx currently! */
/*  fprintf(stderr,"%s\n", msg); */
    exit(1);
}

static void ExitVBEBuf(void)
{
    if (VESABuf_ptr)
	PM_freeRealSeg(VESABuf_ptr);
    VESABuf_ptr = 0;
}

void * PMAPI PM_getVESABuf(uint *len,uint *rseg,uint *roff)
{
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

int PMAPI PM_int386(int intno, PMREGS *in, PMREGS *out)
{
    PMSREGS sregs;
    PM_segread(&sregs);
    return PM_int386x(intno,in,out,&sregs);
}

/* Routines to set and get the real mode interrupt vectors, by making
 * direct real mode calls to DOS and bypassing the DOS extenders API.
 * This is the safest way to handle this, as some servers try to be
 * smart about changing real mode vectors.
 */

void PMAPI _PM_getRMvect(int intno, long *realisr)
{
    RMREGS  regs;
    RMSREGS sregs;

    PM_saveDS();
    regs.h.ah = 0x35;
    regs.h.al = intno;
    PM_int86x(0x21, &regs, &regs, &sregs);
    *realisr = ((long)sregs.es << 16) | regs.x.bx;
}

void PMAPI _PM_setRMvect(int intno, long realisr)
{
    RMREGS  regs;
    RMSREGS sregs;

    PM_saveDS();
    regs.h.ah = 0x25;
    regs.h.al = intno;
    sregs.ds = (int)(realisr >> 16);
    regs.x.dx = (int)(realisr & 0xFFFF);
    PM_int86x(0x21, &regs, &regs, &sregs);
}

void PMAPI _PM_addRealModeBlock(void *mem,uint tag)
{
    int i;

    for (i = 0; i < MAX_RM_BLOCKS; i++) {
	if (rmBlocks[i].p == NULL) {
	    rmBlocks[i].p = mem;
	    rmBlocks[i].tag = tag;
	    return;
	    }
	}
    PM_fatalError("To many real mode memory block allocations!");
}

uint PMAPI _PM_findRealModeBlock(void *mem)
{
    int i;

    for (i = 0; i < MAX_RM_BLOCKS; i++) {
	if (rmBlocks[i].p == mem)
	    return rmBlocks[i].tag;
	}
    PM_fatalError("Could not find prior real mode memory block allocation!");
    return 0;
}

char * PMAPI PM_getCurrentPath(
    char *path,
    int maxLen)
{
    return getcwd(path,maxLen);
}

char PMAPI PM_getBootDrive(void)
{ return 'C'; }

const char * PMAPI PM_getVBEAFPath(void)
{ return "c:\\"; }

const char * PMAPI PM_getNucleusPath(void)
{
    static char path[256];
    char        *env;

    if ((env = getenv("NUCLEUS_PATH")) != NULL)
	return env;
    return "c:\\nucleus";
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
{ return "SMX"; }

const char * PMAPI PM_getMachineName(void)
{ return "SMX"; }

int PMAPI PM_kbhit(void)
{
    int     hit;
    event_t evt;

    hit = EVT_peekNext(&evt,EVT_KEYDOWN | EVT_KEYREPEAT);
    EVT_flush(~(EVT_KEYDOWN | EVT_KEYREPEAT));
    return hit;
}

int PMAPI PM_getch(void)
{
   event_t evt;

    EVT_halt(&evt,EVT_KEYDOWN);
   return EVT_asciiCode(evt.message);
}

PM_HWND PMAPI PM_openConsole(PM_HWND hwndUser,int device,int xRes,int yRes,int bpp,ibool fullScreen)
{
    /* Not used for SMX */
    (void)hwndUser;
    (void)device;
    (void)xRes;
    (void)yRes;
    (void)bpp;
    (void)fullScreen;
    return 0;
}

int PMAPI PM_getConsoleStateSize(void)
{
    return sizeof(DOS_stateBuf);
}

void PMAPI PM_saveConsoleState(void *stateBuf,PM_HWND hwndConsole)
{
    RMREGS          regs;
    DOS_stateBuf    *sb = stateBuf;

    /* Save the old video mode state */
    regs.h.ah = 0x0F;
    PM_int86(0x10,&regs,&regs);
    sb->oldMode = regs.h.al & 0x7F;
    sb->old50Lines = false;
    if (sb->oldMode == 0x3) {
	regs.x.ax = 0x1130;
	regs.x.bx = 0;
	regs.x.dx = 0;
	PM_int86(0x10,&regs,&regs);
	sb->old50Lines = (regs.h.dl == 42 || regs.h.dl == 49);
	}
    (void)hwndConsole;
}

void PMAPI PM_setSuspendAppCallback(int (_ASMAPIP saveState)(int flags))
{
    /* Not used for SMX */
    (void)saveState;
}

void PMAPI PM_restoreConsoleState(const void *stateBuf,PM_HWND hwndConsole)
{
    RMREGS              regs;
    const DOS_stateBuf  *sb = stateBuf;

    /* Retore 50 line mode if set */
    if (sb->old50Lines) {
	regs.x.ax = 0x1112;
	regs.x.bx = 0;
	PM_int86(0x10,&regs,&regs);
	}
    (void)hwndConsole;
}

void PMAPI PM_closeConsole(PM_HWND hwndConsole)
{
    /* Not used for SMX */
    (void)hwndConsole;
}

void PMAPI PM_setOSCursorLocation(int x,int y)
{
    uchar *_biosPtr = PM_getBIOSPointer();
    PM_setByte(_biosPtr+0x50,x);
    PM_setByte(_biosPtr+0x51,y);
}

void PMAPI PM_setOSScreenWidth(int width,int height)
{
    uchar *_biosPtr = PM_getBIOSPointer();
    PM_setWord(_biosPtr+0x4A,width);
    PM_setWord(_biosPtr+0x4C,width*2);
    PM_setByte(_biosPtr+0x84,height-1);
    if (height > 25) {
	PM_setWord(_biosPtr+0x60,0x0607);
	PM_setByte(_biosPtr+0x85,0x08);
	}
    else {
	PM_setWord(_biosPtr+0x60,0x0D0E);
	PM_setByte(_biosPtr+0x85,0x016);
	}
}

void * PMAPI PM_mallocShared(long size)
{
    return PM_malloc(size);
}

void PMAPI PM_freeShared(void *ptr)
{
    PM_free(ptr);
}

#define GetRMVect(intno,isr)    *(isr) = ((ulong*)rmZeroPtr)[intno]
#define SetRMVect(intno,isr)    ((ulong*)rmZeroPtr)[intno] = (isr)

ibool PMAPI PM_doBIOSPOST(
    ushort axVal,
    ulong BIOSPhysAddr,
    void *mappedBIOS,
    ulong BIOSLen)
{
    static int      firstTime = true;
    static uchar    *rmZeroPtr;
    long            Current10,Current6D,Current42;
    RMREGS          regs;
    RMSREGS         sregs;

    /* Create a zero memory mapping for us to use */
    if (firstTime) {
	rmZeroPtr = PM_mapPhysicalAddr(0,0x7FFF,true);
	firstTime = false;
	}

    /* Remap the secondary BIOS to 0xC0000 physical */
    if (BIOSPhysAddr != 0xC0000L || BIOSLen > 32768) {
	/* SMX cannot virtually remap the BIOS, so we can only work if all
	 * the secondary controllers are identical, and we then use the
	 * BIOS on the first controller for all the remaining controllers.
	 *
	 * For OS'es that do virtual memory, and remapping of 0xC0000
	 * physical (perhaps a copy on write mapping) should be all that
	 * is needed.
	 */
	return false;
	}

    /* Save current handlers of int 10h and 6Dh */
    GetRMVect(0x10,&Current10);
    GetRMVect(0x6D,&Current6D);

    /* POST the secondary BIOS */
    GetRMVect(0x42,&Current42);
    SetRMVect(0x10,Current42);  /* Restore int 10h to STD-BIOS */
    regs.x.ax = axVal;
    PM_callRealMode(0xC000,0x0003,&regs,&sregs);

    /* Restore current handlers */
    SetRMVect(0x10,Current10);
    SetRMVect(0x6D,Current6D);

    /* Second the primary BIOS mappin 1:1 for 0xC0000 physical */
    if (BIOSPhysAddr != 0xC0000L) {
	/* SMX does not support this */
	(void)mappedBIOS;
	}
    return true;
}

void PMAPI PM_sleep(ulong milliseconds)
{
    ulong           microseconds = milliseconds * 1000L;
    LZTimerObject   tm;

    LZTimerOnExt(&tm);
    while (LZTimerLapExt(&tm) < microseconds)
	;
    LZTimerOffExt(&tm);
}

int PMAPI PM_getCOMPort(int port)
{
    switch (port) {
	case 0: return 0x3F8;
	case 1: return 0x2F8;
	}
    return 0;
}

int PMAPI PM_getLPTPort(int port)
{
    switch (port) {
	case 0: return 0x3BC;
	case 1: return 0x378;
	case 2: return 0x278;
	}
    return 0;
}

PM_MODULE PMAPI PM_loadLibrary(
    const char *szDLLName)
{
    (void)szDLLName;
    return NULL;
}

void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    (void)hModule;
    (void)szProcName;
    return NULL;
}

void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    (void)hModule;
}

int PMAPI PM_setIOPL(
    int level)
{
    return level;
}

/****************************************************************************
REMARKS:
Internal function to convert the find data to the generic interface.
****************************************************************************/
static void convertFindData(
    PM_findData *findData,
    struct find_t *blk)
{
    ulong   dwSize = findData->dwSize;

    memset(findData,0,findData->dwSize);
    findData->dwSize = dwSize;
    if (blk->attrib & _A_RDONLY)
	findData->attrib |= PM_FILE_READONLY;
    if (blk->attrib & _A_SUBDIR)
	findData->attrib |= PM_FILE_DIRECTORY;
    if (blk->attrib & _A_ARCH)
	findData->attrib |= PM_FILE_ARCHIVE;
    if (blk->attrib & _A_HIDDEN)
	findData->attrib |= PM_FILE_HIDDEN;
    if (blk->attrib & _A_SYSTEM)
	findData->attrib |= PM_FILE_SYSTEM;
    findData->sizeLo = blk->size;
    strncpy(findData->name,blk->name,PM_MAX_PATH);
    findData->name[PM_MAX_PATH-1] = 0;
}

#define FIND_MASK   (_A_RDONLY | _A_ARCH | _A_SUBDIR | _A_HIDDEN | _A_SYSTEM)

/****************************************************************************
REMARKS:
Function to find the first file matching a search criteria in a directory.
****************************************************************************/
void * PMAPI PM_findFirstFile(
    const char *filename,
    PM_findData *findData)
{
    struct find_t *blk;

    if ((blk = PM_malloc(sizeof(*blk))) == NULL)
	return PM_FILE_INVALID;
    if (_dos_findfirst((char*)filename,FIND_MASK,blk) == 0) {
	convertFindData(findData,blk);
	return blk;
	}
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
    struct find_t *blk = handle;

    if (_dos_findnext(blk) == 0) {
	convertFindData(findData,blk);
	return true;
	}
    return false;
}

/****************************************************************************
REMARKS:
Function to close the find process
****************************************************************************/
void PMAPI PM_findClose(
    void *handle)
{
    PM_free(handle);
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
    RMREGS  regs;
    regs.h.dl = (uchar)(drive - 'A' + 1);
    regs.h.ah = 0x36;               /* Get disk information service */
    PM_int86(0x21,&regs,&regs);
    return regs.x.ax != 0xFFFF;     /* AX = 0xFFFF if disk is invalid */
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
    uint oldDrive,maxDrives;
    _dos_getdrive(&oldDrive);
    _dos_setdrive(drive,&maxDrives);
    getcwd(dir,len);
    _dos_setdrive(oldDrive,&maxDrives);
}

/****************************************************************************
REMARKS:
Function to change the file attributes for a specific file.
****************************************************************************/
void PMAPI PM_setFileAttr(
    const char *filename,
    uint attrib)
{
#if defined(TNT) && defined(_MSC_VER)
    DWORD attr = 0;

    if (attrib & PM_FILE_READONLY)
	attr |= FILE_ATTRIBUTE_READONLY;
    if (attrib & PM_FILE_ARCHIVE)
	attr |= FILE_ATTRIBUTE_ARCHIVE;
    if (attrib & PM_FILE_HIDDEN)
	attr |= FILE_ATTRIBUTE_HIDDEN;
    if (attrib & PM_FILE_SYSTEM)
	attr |= FILE_ATTRIBUTE_SYSTEM;
    SetFileAttributes((LPSTR)filename, attr);
#else
    uint attr = 0;

    if (attrib & PM_FILE_READONLY)
	attr |= _A_RDONLY;
    if (attrib & PM_FILE_ARCHIVE)
	attr |= _A_ARCH;
    if (attrib & PM_FILE_HIDDEN)
	attr |= _A_HIDDEN;
    if (attrib & PM_FILE_SYSTEM)
	attr |= _A_SYSTEM;
    _dos_setfileattr(filename,attr);
#endif
}

/****************************************************************************
REMARKS:
Function to create a directory.
****************************************************************************/
ibool PMAPI PM_mkdir(
    const char *filename)
{
#ifdef  __GNUC__
    return mkdir(filename,S_IRUSR) == 0;
#else
/*AM:   return mkdir(filename) == 0; */
    return(false);
#endif
}

/****************************************************************************
REMARKS:
Function to remove a directory.
****************************************************************************/
ibool PMAPI PM_rmdir(
    const char *filename)
{
/*AM:   return rmdir(filename) == 0; */
    return(false);
}

/****************************************************************************
REMARKS:
Allocates a block of locked, physically contiguous memory. The memory
may be required to be below the 16Meg boundary.
****************************************************************************/
void * PMAPI PM_allocLockedMem(
    uint size,
    ulong *physAddr,
    ibool contiguous,
    ibool below16M)
{
    void            *p;
    uint            r_seg,r_off;
    PM_lockHandle   lh;

    /* Under DOS the only way to know the physical memory address is to
     * allocate the memory below the 1Meg boundary as real mode memory.
     * We also allocate 4095 bytes more memory than we need, so we can
     * properly page align the start of the memory block for DMA operations.
     */
    if (size > 4096)
	return NULL;
    if ((p = PM_allocRealSeg((size + 0xFFF) & ~0xFFF,&r_seg,&r_off)) == NULL)
	return NULL;
    *physAddr = ((r_seg << 4) + r_off + 0xFFF) & ~0xFFF;
    PM_lockDataPages(p,size*2,&lh);
    return p;
}

void PMAPI PM_freeLockedMem(void *p,uint size,ibool contiguous)
{
    (void)size;
    PM_freeRealSeg(p);
}

/*-------------------------------------------------------------------------*/
/* Generic DPMI routines common to 16/32 bit code                          */
/*-------------------------------------------------------------------------*/

ulong PMAPI DPMI_mapPhysicalToLinear(ulong physAddr,ulong limit)
{
    PMREGS  r;
    ulong   physOfs;

    if (physAddr < 0x100000L) {
	/* We can't map memory below 1Mb, but the linear address are already
	 * mapped 1:1 for this memory anyway so we just return the base address.
	 */
	return physAddr;
	}

    /* Round the physical address to a 4Kb boundary and the limit to a
     * 4Kb-1 boundary before passing the values to DPMI as some extenders
     * will fail the calls unless this is the case. If we round the
     * physical address, then we also add an extra offset into the address
     * that we return.
     */
    physOfs = physAddr & 4095;
    physAddr = physAddr & ~4095;
    limit = ((limit+physOfs+1+4095) & ~4095)-1;

    r.x.ax = 0x800;                 /* DPMI map physical to linear      */
    r.x.bx = physAddr >> 16;
    r.x.cx = physAddr & 0xFFFF;
    r.x.si = limit >> 16;
    r.x.di = limit & 0xFFFF;
    PM_int386(0x31, &r, &r);
    if (r.x.cflag)
	return 0xFFFFFFFFUL;
    return ((ulong)r.x.bx << 16) + r.x.cx + physOfs;
}

int PMAPI DPMI_setSelectorBase(ushort sel,ulong linAddr)
{
    PMREGS  r;

    r.x.ax = 7;                     /* DPMI set selector base address   */
    r.x.bx = sel;
    r.x.cx = linAddr >> 16;
    r.x.dx = linAddr & 0xFFFF;
    PM_int386(0x31, &r, &r);
    if (r.x.cflag)
	return 0;
    return 1;
}

ulong PMAPI DPMI_getSelectorBase(ushort sel)
{
    PMREGS  r;

    r.x.ax = 6;                     /* DPMI get selector base address   */
    r.x.bx = sel;
    PM_int386(0x31, &r, &r);
    return ((ulong)r.x.cx << 16) + r.x.dx;
}

int PMAPI DPMI_setSelectorLimit(ushort sel,ulong limit)
{
    PMREGS  r;

    r.x.ax = 8;                     /* DPMI set selector limit          */
    r.x.bx = sel;
    r.x.cx = limit >> 16;
    r.x.dx = limit & 0xFFFF;
    PM_int386(0x31, &r, &r);
    if (r.x.cflag)
	return 0;
    return 1;
}

uint PMAPI DPMI_createSelector(ulong base,ulong limit)
{
    uint    sel;
    PMREGS  r;

    /* Allocate 1 descriptor */
    r.x.ax = 0;
    r.x.cx = 1;
    PM_int386(0x31, &r, &r);
    if (r.x.cflag) return 0;
    sel = r.x.ax;

    /* Set the descriptor access rights (for a 32 bit page granular
     * segment, ring 0).
     */
    r.x.ax = 9;
    r.x.bx = sel;
    r.x.cx = 0x4093;
    PM_int386(0x31, &r, &r);

    /* Map physical memory and create selector */
    if ((base = DPMI_mapPhysicalToLinear(base,limit)) == 0xFFFFFFFFUL)
	return 0;
    if (!DPMI_setSelectorBase(sel,base))
	return 0;
    if (!DPMI_setSelectorLimit(sel,limit))
	return 0;
    return sel;
}

void PMAPI DPMI_freeSelector(uint sel)
{
    PMREGS  r;

    r.x.ax = 1;
    r.x.bx = sel;
    PM_int386(0x31, &r, &r);
}

int PMAPI DPMI_lockLinearPages(ulong linear,ulong len)
{
    PMREGS  r;

    r.x.ax = 0x600;                     /* DPMI Lock Linear Region      */
    r.x.bx = (linear >> 16);            /* Linear address in BX:CX      */
    r.x.cx = (linear & 0xFFFF);
    r.x.si = (len >> 16);               /* Length in SI:DI              */
    r.x.di = (len & 0xFFFF);
    PM_int386(0x31, &r, &r);
    return (!r.x.cflag);
}

int PMAPI DPMI_unlockLinearPages(ulong linear,ulong len)
{
    PMREGS  r;

    r.x.ax = 0x601;                     /* DPMI Unlock Linear Region    */
    r.x.bx = (linear >> 16);            /* Linear address in BX:CX      */
    r.x.cx = (linear & 0xFFFF);
    r.x.si = (len >> 16);               /* Length in SI:DI              */
    r.x.di = (len & 0xFFFF);
    PM_int386(0x31, &r, &r);
    return (!r.x.cflag);
}

void * PMAPI DPMI_mapPhysicalAddr(ulong base,ulong limit,ibool isCached)
{
    PMSREGS sregs;
    ulong   linAddr;
    ulong   DSBaseAddr;

    /* Get the base address for the default DS selector */
    PM_segread(&sregs);
    DSBaseAddr = DPMI_getSelectorBase(sregs.ds);
    if ((base < 0x100000) && (DSBaseAddr == 0)) {
	/* DS is zero based, so we can directly access the first 1Mb of
	 * system memory (like under DOS4GW).
	 */
	return (void*)base;
	}

    /* Map the memory to a linear address using DPMI function 0x800 */
    if ((linAddr = DPMI_mapPhysicalToLinear(base,limit)) == 0) {
	if (base >= 0x100000)
	    return NULL;
	/* If the linear address mapping fails but we are trying to
	 * map an area in the first 1Mb of system memory, then we must
	 * be running under a Windows or OS/2 DOS box. Under these
	 * environments we can use the segment wrap around as a fallback
	 * measure, as this does work properly.
	 */
	linAddr = base;
	}

    /* Now expand the default DS selector to 4Gb so we can access it */
    if (!DPMI_setSelectorLimit(sregs.ds,0xFFFFFFFFUL))
	return NULL;

    /* Finally enable caching for the page tables that we just mapped in,
     * since DOS4GW and PMODE/W create the page table entries without
     * caching enabled which hurts the performance of the linear framebuffer
     * as it disables write combining on Pentium Pro and above processors.
     *
     * For those processors cache disabling is better handled through the
     * MTRR registers anyway (we can write combine a region but disable
     * caching) so that MMIO register regions do not screw up.
     */
    if (isCached) {
	if ((PDB = _PM_getPDB()) != 0 && DSBaseAddr == 0) {
	    int     startPDB,endPDB,iPDB,startPage,endPage,start,end,iPage;
	    ulong   pageTable,*pPageTable;
	    if (!pPDB) {
		if (PDB >= 0x100000)
		    pPDB = (ulong*)DPMI_mapPhysicalToLinear(PDB,0xFFF);
		else
		    pPDB = (ulong*)PDB;
		}
	    if (pPDB) {
		startPDB = (linAddr >> 22) & 0x3FF;
		startPage = (linAddr >> 12) & 0x3FF;
		endPDB = ((linAddr+limit) >> 22) & 0x3FF;
		endPage = ((linAddr+limit) >> 12) & 0x3FF;
		for (iPDB = startPDB; iPDB <= endPDB; iPDB++) {
		    pageTable = pPDB[iPDB] & ~0xFFF;
		    if (pageTable >= 0x100000)
			pPageTable = (ulong*)DPMI_mapPhysicalToLinear(pageTable,0xFFF);
		    else
			pPageTable = (ulong*)pageTable;
		    start = (iPDB == startPDB) ? startPage : 0;
		    end = (iPDB == endPDB) ? endPage : 0x3FF;
		    for (iPage = start; iPage <= end; iPage++)
			pPageTable[iPage] &= ~0x18;
		    }
		}
	    }
	}

    /* Now return the base address of the memory into the default DS */
    return (void*)(linAddr - DSBaseAddr);
}

/* Some DOS extender implementations do not directly support calling a
 * real mode procedure from protected mode. However we can simulate what
 * we need temporarily hooking the INT 6Ah vector with a small real mode
 * stub that will call our real mode code for us.
 */

static uchar int6AHandler[] = {
    0x00,0x00,0x00,0x00,        /*  __PMODE_callReal variable           */
    0xFB,                       /*  sti                                 */
    0x2E,0xFF,0x1E,0x00,0x00,   /*  call    [cs:__PMODE_callReal]       */
    0xCF,                       /*  iretf                               */
    };
static uchar *crPtr = NULL; /* Pointer to of int 6A handler         */
static uint crRSeg,crROff;  /* Real mode seg:offset of handler      */

void PMAPI PM_callRealMode(uint seg,uint off, RMREGS *in,
    RMSREGS *sregs)
{
    uchar   *p;
    uint    oldSeg,oldOff;

    if (!crPtr) {
	/* Allocate and copy the memory block only once */
	crPtr = PM_allocRealSeg(sizeof(int6AHandler), &crRSeg, &crROff);
	memcpy(crPtr,int6AHandler,sizeof(int6AHandler));
	}
    PM_setWord(crPtr,off);              /* Plug in address to call  */
    PM_setWord(crPtr+2,seg);
    p = PM_mapRealPointer(0,0x6A * 4);
    oldOff = PM_getWord(p);             /* Save old handler address */
    oldSeg = PM_getWord(p+2);
    PM_setWord(p,crROff+4);             /* Hook 6A handler          */
    PM_setWord(p+2,crRSeg);
    PM_int86x(0x6A, in, in, sregs);     /* Call real mode code      */
    PM_setWord(p,oldOff);               /* Restore old handler      */
    PM_setWord(p+2,oldSeg);
}

void * PMAPI PM_getBIOSPointer(void)
{ return PM_mapPhysicalAddr(0x400,0xFFFF,true); }

void * PMAPI PM_getA0000Pointer(void)
{ return PM_mapPhysicalAddr(0xA0000,0xFFFF,true); }

void * PMAPI PM_mapPhysicalAddr(ulong base,ulong limit,ibool isCached)
{ return DPMI_mapPhysicalAddr(base,limit,isCached); }

void PMAPI PM_freePhysicalAddr(void *ptr,ulong limit)
{
    /* Mapping cannot be free */
}

ulong PMAPI PM_getPhysicalAddr(void *p)
{
    /* TODO: This function should find the physical address of a linear */
    /*       address. */
    (void)p;
    return 0xFFFFFFFFUL;
}

void * PMAPI PM_mapToProcess(void *base,ulong limit)
{
    (void)limit;
    return (void*)base;
}

void * PMAPI PM_mapRealPointer(uint r_seg,uint r_off)
{
    static uchar *zeroPtr = NULL;

    if (!zeroPtr)
	zeroPtr = PM_mapPhysicalAddr(0,0xFFFFF,true);
    return (void*)(zeroPtr + MK_PHYS(r_seg,r_off));
}

void * PMAPI PM_allocRealSeg(uint size,uint *r_seg,uint *r_off)
{
    PMREGS      r;
    void        *p;

    r.x.ax = 0x100;                 /* DPMI allocate DOS memory         */
    r.x.bx = (size + 0xF) >> 4;     /* number of paragraphs             */
    PM_int386(0x31, &r, &r);
    if (r.x.cflag)
	return NULL;                /* DPMI call failed                 */
    *r_seg = r.x.ax;                /* Real mode segment                */
    *r_off = 0;
    p = PM_mapRealPointer(*r_seg,*r_off);
    _PM_addRealModeBlock(p,r.x.dx);
    return p;
}

void PMAPI PM_freeRealSeg(void *mem)
{
    PMREGS  r;

    r.x.ax = 0x101;                     /* DPMI free DOS memory         */
    r.x.dx = _PM_findRealModeBlock(mem);/* DX := selector from 0x100    */
    PM_int386(0x31, &r, &r);
}

static DPMI_handler_t   DPMI_int10 = NULL;

void PMAPI DPMI_setInt10Handler(DPMI_handler_t handler)
{
    DPMI_int10 = handler;
}

void PMAPI DPMI_int86(int intno, DPMI_regs *regs)
{
    PMREGS      r;
    PMSREGS     sr;

    if (intno == 0x10 && DPMI_int10) {
	if (DPMI_int10(regs))
	    return;
	}
    PM_segread(&sr);
    r.x.ax = 0x300;                 /* DPMI issue real interrupt    */
    r.h.bl = intno;
    r.h.bh = 0;
    r.x.cx = 0;
    sr.es = sr.ds;
    r.e.edi = (uint)regs;
    PM_int386x(0x31, &r, &r, &sr);  /* Issue the interrupt          */
}

#define IN(reg)     rmregs.reg = in->e.reg
#define OUT(reg)    out->e.reg = rmregs.reg

int PMAPI PM_int86(int intno, RMREGS *in, RMREGS *out)
{
    DPMI_regs   rmregs;

    memset(&rmregs, 0, sizeof(rmregs));
    IN(eax); IN(ebx); IN(ecx); IN(edx); IN(esi); IN(edi);

/* These real mode ints may cause crashes. */
/*AM:   DPMI_int86(intno,&rmregs);      /###* DPMI issue real interrupt    */

    OUT(eax); OUT(ebx); OUT(ecx); OUT(edx); OUT(esi); OUT(edi);
    out->x.cflag = rmregs.flags & 0x1;
    return out->x.ax;
}

int PMAPI PM_int86x(int intno, RMREGS *in, RMREGS *out,
    RMSREGS *sregs)
{
    DPMI_regs   rmregs;

    memset(&rmregs, 0, sizeof(rmregs));
    IN(eax); IN(ebx); IN(ecx); IN(edx); IN(esi); IN(edi);
    rmregs.es = sregs->es;
    rmregs.ds = sregs->ds;

/*AM:   DPMI_int86(intno,&rmregs);      /###* DPMI issue real interrupt    */

    OUT(eax); OUT(ebx); OUT(ecx); OUT(edx); OUT(esi); OUT(edi);
    sregs->es = rmregs.es;
    sregs->cs = rmregs.cs;
    sregs->ss = rmregs.ss;
    sregs->ds = rmregs.ds;
    out->x.cflag = rmregs.flags & 0x1;
    return out->x.ax;
}

#pragma pack(1)

typedef struct {
	uint    LargestBlockAvail;
	uint    MaxUnlockedPage;
	uint    LargestLockablePage;
	uint    LinAddrSpace;
	uint    NumFreePagesAvail;
	uint    NumPhysicalPagesFree;
	uint    TotalPhysicalPages;
	uint    FreeLinAddrSpace;
	uint    SizeOfPageFile;
	uint    res[3];
	} MemInfo;

#pragma pack()

void PMAPI PM_availableMemory(ulong *physical,ulong *total)
{
    PMREGS  r;
    PMSREGS sr;
    MemInfo memInfo;

    PM_segread(&sr);
    r.x.ax = 0x500;                 /* DPMI get free memory info */
    sr.es = sr.ds;
    r.e.edi = (uint)&memInfo;
    PM_int386x(0x31, &r, &r, &sr);  /* Issue the interrupt */
    *physical = memInfo.NumPhysicalPagesFree * 4096;
    *total = memInfo.LargestBlockAvail;
    if (*total < *physical)
	*physical = *total;
}

/****************************************************************************
REMARKS:
Function to get the file attributes for a specific file.
****************************************************************************/
uint PMAPI PM_getFileAttr(
    const char *filename)
{
    /* TODO: Implement this! */
    return 0;
}

/****************************************************************************
REMARKS:
Function to get the file time and date for a specific file.
****************************************************************************/
ibool PMAPI PM_getFileTime(
    const char *filename,
    ibool gmTime,
    PM_time *time)
{
    /* TODO: Implement this! */
    return false;
}

/****************************************************************************
REMARKS:
Function to set the file time and date for a specific file.
****************************************************************************/
ibool PMAPI PM_setFileTime(
    const char *filename,
    ibool gmTime,
    PM_time *time)
{
    /* TODO: Implement this! */
    return false;
}
