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
* Environment:  32-bit OS/2 VDD
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
#include "sdd/sddhelp.h"
#include "mtrr.h"

#define TRACE(a)

/*--------------------------- Global variables ----------------------------*/

#define MAX_MEMORY_SHARED           100
#define MAX_MEMORY_MAPPINGS         100

/* TODO: I think the global and linear members will be the same, but not sure yet. */
typedef struct {
    void    *linear;
    ulong   global;
    ulong   length;
    int     npages;
    } memshared;

typedef struct {
    ulong   physical;
    ulong   linear;
    ulong   length;
    int     npages;
    ibool   isCached;
    } mmapping;

static int          numMappings = 0;
static memshared    shared[MAX_MEMORY_MAPPINGS] = {0};
static mmapping     maps[MAX_MEMORY_MAPPINGS];
ibool               _PM_haveBIOS = TRUE;
char                _PM_cntPath[PM_MAX_PATH] = "";     /* there just isn't any */
uchar               *_PM_rmBufAddr = NULL;
ushort _VARAPI      PM_savedDS = 0;   /* why can't I use the underscore prefix? */

HVDHSEM             hevFarCallRet = NULL;
HVDHSEM             hevIRet       = NULL;
HHOOK               hhookUserReturnHook = NULL;
HHOOK               hhookUserIRetHook   = NULL;

static void (PMAPIP fatalErrorCleanup)(void) = NULL;

/*----------------------------- Implementation ----------------------------*/

/* Functions to read and write CMOS registers */

ulong   PMAPI _PM_getPDB(void);
uchar   PMAPI _PM_readCMOS(int index);
void    PMAPI _PM_writeCMOS(int index,uchar value);

VOID HOOKENTRY UserReturnHook(PVOID pRefData, PCRF pcrf);
VOID HOOKENTRY UserIRetHook(PVOID pRefData, PCRF pcrf);

void PMAPI PM_init(void)
{
    MTRR_init();

    /* Initialize VDD-specific data */
    /* Note: PM_init must be (obviously) called in VDM task context! */
    VDHCreateSem(&hevFarCallRet, VDH_EVENTSEM);
    VDHCreateSem(&hevIRet, VDH_EVENTSEM);
    hhookUserReturnHook = VDHAllocHook(VDH_RETURN_HOOK, (PFNARM)UserReturnHook, 0);
    hhookUserIRetHook   = VDHAllocHook(VDH_RETURN_HOOK, (PFNARM)UserIRetHook, 0);

    if ((hevIRet == NULL) || (hevFarCallRet == NULL) ||
	(hhookUserReturnHook == NULL) || (hhookUserIRetHook == NULL)) {
	/* something failed, we can't go on */
	/* TODO: take some action here! */
	}
}

/* Do some cleaning up */
void PMAPI PM_exit(void)
{
    /* Note: Hooks allocated during or after VDM creation are deallocated automatically */
    if (hevIRet != NULL)
	VDHDestroySem(hevIRet);

    if (hevFarCallRet != NULL)
	VDHDestroySem(hevFarCallRet);
}

ibool PMAPI PM_haveBIOSAccess(void)
{ return _PM_haveBIOS; }

long PMAPI PM_getOSType(void)
{ return /*_OS_OS2VDD*/ _OS_OS2; }  /*FIX!! */

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

void PMAPI PM_fatalError(const char *msg)
{
    if (fatalErrorCleanup)
	fatalErrorCleanup();
/*    Fatal_Error_Handler(msg,0);  TODO: implement somehow! */
}

/****************************************************************************
PARAMETERS:
len     - Place to store the length of the buffer
rseg    - Place to store the real mode segment of the buffer
roff    - Place to store the real mode offset of the buffer

REMARKS:
This function returns the address and length of the global VESA transfer
buffer.
****************************************************************************/
void * PMAPI PM_getVESABuf(
    uint *len,
    uint *rseg,
    uint *roff)
{
    if (_PM_rmBufAddr) {
	*len = 0; /*VESA_BUF_SIZE; */
	*rseg = (ulong)(_PM_rmBufAddr) >> 4;
	*roff = (ulong)(_PM_rmBufAddr) & 0xF;
	return _PM_rmBufAddr;
	}
    return NULL;
}

int PMAPI PM_int386(int intno, PMREGS *in, PMREGS *out)
{
    /* Unused in VDDs */
    return 0;
}

char * PMAPI PM_getCurrentPath(char *path,int maxLen)
{
    strncpy(path, _PM_cntPath, maxLen);
    path[maxLen - 1] = 0;
    return path;
}

char PMAPI PM_getBootDrive(void)
{
    ulong   boot = 3;
    boot = VDHQuerySysValue(0, VDHGSV_BOOTDRV);
    return (char)('a' + boot - 1);
}

const char * PMAPI PM_getVBEAFPath(void)
{
    static char path[CCHMAXPATH];
    strcpy(path,"x:\\");
    path[0] = PM_getBootDrive();
    return path;
}

const char * PMAPI PM_getNucleusPath(void)
{
	static char path[CCHMAXPATH];
	strcpy(path,"x:\\os2\\drivers");
	path[0] = PM_getBootDrive();
	PM_backslash(path);
	strcat(path,"nucleus");
	return path;
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
{ return PM_getMachineName(); }

const char * PMAPI PM_getMachineName(void)
{
    return "Unknown";
}

int PMAPI PM_kbhit(void)
{ return 1; }

int PMAPI PM_getch(void)
{ return 0; }

PM_HWND PMAPI PM_openConsole(PM_HWND hwndUser,int device,int xRes,int yRes,int bpp,ibool fullScreen)
{
    /* Unused in VDDs */
    return NULL;
}

int PMAPI PM_getConsoleStateSize(void)
{
    /* Unused in VDDs */
    return 1;
}

void PMAPI PM_saveConsoleState(void *stateBuf,PM_HWND hwndConsole)
{
    /* Unused in VDDs */
}

void PMAPI PM_setSuspendAppCallback(int (_ASMAPIP saveState)(int flags))
{
    /* Unused in VDDs */
}

void PMAPI PM_restoreConsoleState(const void *stateBuf,PM_HWND hwndConsole)
{
    /* Unused in VDDs */
}

void PMAPI PM_closeConsole(PM_HWND hwndConsole)
{
    /* Unused in VDDs */
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
    PM_setByte(_biosPtr+0x4A,width);
    PM_setByte(_biosPtr+0x84,height-1);
}

/****************************************************************************
REMARKS:
Allocate a block of shared memory. For OS/2 VDD we allocate shared memory
as locked, global memory that is accessible from any memory context
(including interrupt time context), which allows us to load our important
data structure and code such that we can access it directly from a ring
0 interrupt context.
****************************************************************************/
void * PMAPI PM_mallocShared(long size)
{
    ULONG       nPages = (size + 0xFFF) >> 12;
    int         i;

    /* First find a free slot in our shared memory table */
    for (i = 0; i < MAX_MEMORY_SHARED; i++) {
	if (shared[i].linear == 0)
	    break;
	}
    if (i < MAX_MEMORY_SHARED) {
	shared[i].linear = VDHAllocPages(NULL, nPages, VDHAP_SYSTEM | VDHAP_FIXED);
	shared[i].npages = nPages;
	shared[i].global = (ULONG)shared[i].linear;
	return (void*)shared[i].global;
	}
    return NULL;
}

/****************************************************************************
REMARKS:
Free a block of shared memory
****************************************************************************/
void PMAPI PM_freeShared(void *p)
{
    int i;

    /* Find a shared memory block in our table and free it */
    for (i = 0; i < MAX_MEMORY_SHARED; i++) {
	if (shared[i].global == (ulong)p) {
	    VDHFreePages(shared[i].linear);
	    shared[i].linear = 0;
	    break;
	    }
	}
}

void * PMAPI PM_mapToProcess(void *base,ulong limit)
{ return (void*)base; }

ibool PMAPI PM_doBIOSPOST(
    ushort axVal,
    ulong BIOSPhysAddr,
    void *mappedBIOS,
    ulong BIOSLen)
{
    /* TODO: Figure out how to do this */
    return false;
}

void * PMAPI PM_getBIOSPointer(void)
{ return (void*)0x400; }

void * PMAPI PM_getA0000Pointer(void)
{ return PM_mapPhysicalAddr(0xA0000,0xFFFF,true); }

/****************************************************************************
PARAMETERS:
base        - Physical base address of the memory to maps in
limit       - Limit of physical memory to region to maps in

RETURNS:
Linear address of the newly mapped memory.

REMARKS:
Maps a physical memory range to a linear memory range.
****************************************************************************/
ulong MapPhysicalToLinear(
    ulong base,
    ulong limit,
    int *npages)
{
    ulong   linear,length = limit+1;
    int     i,ppage,flags;
#if 0
    ppage = base >> 12;
    *npages = (length + (base & 0xFFF) + 4095) >> 12;
    flags = PR_FIXED | PR_STATIC;
    if (base == 0xA0000) {
	/* We require the linear address to be aligned to a 64Kb boundary
	 * for mapping the banked framebuffer (so we can do efficient
	 * carry checking for bank changes in the assembler code). The only
	 * way to ensure this is to force the linear address to be aligned
	 * to a 4Mb boundary.
	 */
	flags |= PR_4MEG;
	}
    if ((linear = (ulong)PageReserve(PR_SYSTEM,*npages,flags)) == (ulong)-1)
	return 0;
    if (!PageCommitPhys(linear >> 12,*npages,ppage,PC_INCR | PC_USER | PC_WRITEABLE))
	return 0;
#endif
    return linear + (base & 0xFFF);
}

/****************************************************************************
PARAMETERS:
base        - Physical base address of the memory to map in
limit       - Limit of physical memory to region to map in
isCached    - True if the memory should be cached, false if not

RETURNS:
Linear address of the newly mapped memory.

REMARKS:
This function maps physical memory to linear memory, which can then be used
to create a selector or used directly from 32-bit protected mode programs.
This is better than DPMI 0x800, since it allows you to maps physical
memory below 1Mb, which gets this memory out of the way of the Windows VxD's
sticky paws.

NOTE:   If the memory is not expected to be cached, this function will
	directly re-program the PCD (Page Cache Disable) bit in the
	page tables. There does not appear to be a mechanism in the VMM
	to control this bit via the regular interface.
****************************************************************************/
void * PMAPI PM_mapPhysicalAddr(
    ulong base,
    ulong limit,
    ibool isCached)
{
    ulong   linear,length = limit+1;
    int     i,npages;
    ulong   PDB,*pPDB;

    /* Search table of existing mappings to see if we have already mapped
     * a region of memory that will serve this purpose.
     */
    for (i = 0; i < numMappings; i++) {
	if (maps[i].physical == base && maps[i].length == length && maps[i].isCached == isCached)
	    return (void*)maps[i].linear;
	}
    if (numMappings == MAX_MEMORY_MAPPINGS)
	return NULL;

    /* We did not find any previously mapped memory region, so map it in.
     * Note that we do not use MapPhysToLinear, since this function appears
     * to have problems mapping memory in the 1Mb physical address space.
     * Hence we use PageReserve and PageCommitPhys.
     */
    if ((linear = MapPhysicalToLinear(base,limit,&npages)) == 0)
	return NULL;
    maps[numMappings].physical = base;
    maps[numMappings].length = length;
    maps[numMappings].linear = linear;
    maps[numMappings].npages = npages;
    maps[numMappings].isCached = isCached;
    numMappings++;

#if 0
    /* Finally disable caching where necessary */
    if (!isCached && (PDB = _PM_getPDB()) != 0) {
	int     startPDB,endPDB,iPDB,startPage,endPage,start,end,iPage;
	ulong   pageTable,*pPageTable;

	if (PDB >= 0x100000)
	    pPDB = (ulong*)MapPhysicalToLinear(PDB,0xFFF,&npages);
	else
	    pPDB = (ulong*)PDB;
	if (pPDB) {
	    startPDB = (linear >> 22) & 0x3FF;
	    startPage = (linear >> 12) & 0x3FF;
	    endPDB = ((linear+limit) >> 22) & 0x3FF;
	    endPage = ((linear+limit) >> 12) & 0x3FF;
	    for (iPDB = startPDB; iPDB <= endPDB; iPDB++) {
		pageTable = pPDB[iPDB] & ~0xFFF;
		if (pageTable >= 0x100000)
		    pPageTable = (ulong*)MapPhysicalToLinear(pageTable,0xFFF,&npages);
		else
		    pPageTable = (ulong*)pageTable;
		start = (iPDB == startPDB) ? startPage : 0;
		end = (iPDB == endPDB) ? endPage : 0x3FF;
		for (iPage = start; iPage <= end; iPage++)
		    pPageTable[iPage] |= 0x10;
		PageFree((ulong)pPageTable,PR_STATIC);
		}
	    PageFree((ulong)pPDB,PR_STATIC);
	    }
	}
#endif
    return (void*)linear;
}

void PMAPI PM_freePhysicalAddr(void *ptr,ulong limit)
{
    /* We never free the mappings */
}

void PMAPI PM_sleep(ulong milliseconds)
{
    /* We never sleep in a VDD */
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

ulong PMAPI PM_getPhysicalAddr(void *p)
{
	/* TODO: This function should find the physical address of a linear */
	/*               address. */
	return 0xFFFFFFFFUL;
}

void PMAPI _PM_freeMemoryMappings(void)
{
    int i;
/*    for (i = 0; i < numMappings; i++) */
/*        PageFree(maps[i].linear,PR_STATIC); */
}

void * PMAPI PM_mapRealPointer(uint r_seg,uint r_off)
{ return (void*)MK_PHYS(r_seg,r_off); }

void * PMAPI PM_allocRealSeg(uint size,uint *r_seg,uint *r_off)
{ return NULL; }

void PMAPI PM_freeRealSeg(void *mem)
{ }

void PMAPI DPMI_int86(int intno, DPMI_regs *regs)
{
    /* Unsed in VDDs */
}

/****************************************************************************
REMARKS:
Load the V86 registers in the client state, and save the original state
before loading the registers.
****************************************************************************/
static void LoadV86Registers(
    PCRF saveRegs,
    RMREGS *in,
    RMSREGS *sregs)
{
    PCRF            pcrf;     /* current client register frame */

    /* get pointer to registers */
    pcrf = (PCRF)VDHQuerySysValue(CURRENT_VDM, VDHLSV_PCRF);

    /* Note: We could do VDHPushRegs instead but this should be safer as it */
    /* doesn't rely on the VDM session having enough free stack space. */
    *saveRegs = *pcrf;        /* save all registers */

    pcrf->crf_eax = in->e.eax;    /* load new values */
    pcrf->crf_ebx = in->e.ebx;
    pcrf->crf_ecx = in->e.ecx;
    pcrf->crf_edx = in->e.edx;
    pcrf->crf_esi = in->e.esi;
    pcrf->crf_edi = in->e.edi;
    pcrf->crf_es  = sregs->es;
    pcrf->crf_ds  = sregs->ds;

}

/****************************************************************************
REMARKS:
Read the V86 registers from the client state and restore the original state.
****************************************************************************/
static void ReadV86Registers(
    PCRF saveRegs,
    RMREGS *out,
    RMSREGS *sregs)
{
    PCRF            pcrf;     /* current client register frame */

    /* get pointer to registers */
    pcrf = (PCRF)VDHQuerySysValue(CURRENT_VDM, VDHLSV_PCRF);

    /* read new register values */
    out->e.eax = pcrf->crf_eax;
    out->e.ebx = pcrf->crf_ebx;
    out->e.ecx = pcrf->crf_ecx;
    out->e.edx = pcrf->crf_edx;
    out->e.esi = pcrf->crf_esi;
    out->e.edi = pcrf->crf_edi;
    sregs->es  = pcrf->crf_es;
    sregs->ds  = pcrf->crf_ds;

    /* restore original client registers */
    *pcrf = *saveRegs;
}

/****************************************************************************
REMARKS: Used for far calls into V86 code
****************************************************************************/
VOID HOOKENTRY UserReturnHook(
  PVOID pRefData,
  PCRF pcrf )
{
    VDHPostEventSem(hevFarCallRet);
}

/****************************************************************************
REMARKS: Used for calling BIOS interrupts
****************************************************************************/
VOID HOOKENTRY UserIRetHook(
  PVOID pRefData,
  PCRF pcrf )
{
    VDHPostEventSem(hevIRet);
}

/****************************************************************************
REMARKS:
Call a V86 real mode function with the specified register values
loaded before the call. The call returns with a far ret.
Must be called from within a DOS session context!
****************************************************************************/
void PMAPI PM_callRealMode(
    uint seg,
    uint off,
    RMREGS *regs,
    RMSREGS *sregs)
{
    CRF   saveRegs;
    FPFN  fnAddress;
    ULONG rc;

    TRACE("SDDHELP: Entering PM_callRealMode()\n");
    LoadV86Registers(SSToDS(&saveRegs),regs,sregs);

    /* set up return hook for call */
    rc = VDHArmReturnHook(hhookUserReturnHook, VDHARH_CSEIP_HOOK);

    VDHResetEventSem(hevFarCallRet);

    /* the address is a 16:32 pointer */
    OFFSETOF32(fnAddress)  = off;
    SEGMENTOF32(fnAddress) = seg;
    rc = VDHPushFarCall(fnAddress);
    VDHYield(0);

    /* wait until the V86 call returns - our return hook posts the semaphore */
    rc = VDHWaitEventSem(hevFarCallRet, SEM_INDEFINITE_WAIT);

    ReadV86Registers(SSToDS(&saveRegs),regs,sregs);
    TRACE("SDDHELP: Exiting PM_callRealMode()\n");
}

/****************************************************************************
REMARKS:
Issue a V86 real mode interrupt with the specified register values
loaded before the interrupt.
Must be called from within a DOS session context!
****************************************************************************/
int PMAPI PM_int86(
    int intno,
    RMREGS *in,
    RMREGS *out)
{
    RMSREGS    sregs = {0};
    CRF        saveRegs;
    ushort     oldDisable;
    ULONG      rc;

    memset(SSToDS(&sregs), 0, sizeof(sregs));

#if 0   /* do we need this?? */
    /* Disable pass-up to our VDD handler so we directly call BIOS */
    TRACE("SDDHELP: Entering PM_int86()\n");
    if (disableTSRFlag) {
	oldDisable = *disableTSRFlag;
	*disableTSRFlag = 0;
	}
#endif

    LoadV86Registers(SSToDS(&saveRegs), in, SSToDS(&sregs));

    VDHResetEventSem(hevIRet);
    rc = VDHPushInt(intno);

    /* set up return hook for interrupt */
    rc = VDHArmReturnHook(hhookUserIRetHook, VDHARH_NORMAL_IRET);

    VDHYield(0);

    /* wait until the V86 IRETs - our return hook posts the semaphore */
    rc = VDHWaitEventSem(hevIRet, 5000); /*SEM_INDEFINITE_WAIT); */

    ReadV86Registers(SSToDS(&saveRegs), out, SSToDS(&sregs));

#if 0
    /* Re-enable pass-up to our VDD handler if previously enabled */
    if (disableTSRFlag)
	*disableTSRFlag = oldDisable;
#endif

    TRACE("SDDHELP: Exiting PM_int86()\n");
    return out->x.ax;

}

/****************************************************************************
REMARKS:
Issue a V86 real mode interrupt with the specified register values
loaded before the interrupt.
****************************************************************************/
int PMAPI PM_int86x(
    int intno,
    RMREGS *in,
    RMREGS *out,
    RMSREGS *sregs)
{
    CRF             saveRegs;
    ushort          oldDisable;
    ULONG       rc;

#if 0
    /* Disable pass-up to our VxD handler so we directly call BIOS */
    TRACE("SDDHELP: Entering PM_int86x()\n");
    if (disableTSRFlag) {
	oldDisable = *disableTSRFlag;
	*disableTSRFlag = 0;
	}
#endif
    LoadV86Registers(SSToDS(&saveRegs), in, sregs);

    VDHResetEventSem(hevIRet);
    rc = VDHPushInt(intno);

    /* set up return hook for interrupt */
    rc = VDHArmReturnHook(hhookUserIRetHook, VDHARH_NORMAL_IRET);

    VDHYield(0);

    /* wait until the V86 IRETs - our return hook posts the semaphore */
    rc = VDHWaitEventSem(hevIRet, 5000); /*SEM_INDEFINITE_WAIT); */

    ReadV86Registers(SSToDS(&saveRegs), out, sregs);

#if 0
    /* Re-enable pass-up to our VxD handler if previously enabled */
    if (disableTSRFlag)
	*disableTSRFlag = oldDisable;
#endif

    TRACE("SDDHELP: Exiting PM_int86x()\n");
    return out->x.ax;
}

void PMAPI PM_availableMemory(ulong *physical,ulong *total)
{ *physical = *total = 0; }

/****************************************************************************
REMARKS:
Allocates a block of locked physical memory.
****************************************************************************/
void * PMAPI PM_allocLockedMem(
    uint size,
    ulong *physAddr,
    ibool contiguous,
    ibool below16M)
{
    ULONG       flags = VDHAP_SYSTEM;
    ULONG       nPages = (size + 0xFFF) >> 12;

    flags |= (physAddr != NULL) ? VDHAP_PHYSICAL : VDHAP_FIXED;

    return VDHAllocPages(physAddr, nPages, VDHAP_SYSTEM | VDHAP_PHYSICAL);
}

/****************************************************************************
REMARKS:
Frees a block of locked physical memory.
****************************************************************************/
void PMAPI PM_freeLockedMem(
    void *p,
    uint size,
    ibool contiguous)
{
    if (p)
	VDHFreePages((PVOID)p);
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    ULONG  lockHandle;

    /* TODO: the lock handle is essential for the unlock operation!! */
    lockHandle = VDHLockMem(p, len, 0, (PVOID)VDHLM_NO_ADDR, NULL);

    if (lockHandle != NULL)
       return 0;
    else
       return 1;
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    /* TODO: implement - use a table of lock handles? */
    /* VDHUnlockPages(lockHandle); */
    return 0;
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    return PM_lockDataPages((void*)p,len,lh);
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    return PM_unlockDataPages((void*)p,len,lh);
}

/****************************************************************************
REMARKS:
OS specific shared libraries not supported inside a VDD
****************************************************************************/
PM_MODULE PMAPI PM_loadLibrary(
    const char *szDLLName)
{
    (void)szDLLName;
    return NULL;
}

/****************************************************************************
REMARKS:
OS specific shared libraries not supported inside a VDD
****************************************************************************/
void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    (void)hModule;
    (void)szProcName;
    return NULL;
}

/****************************************************************************
REMARKS:
OS specific shared libraries not supported inside a VDD
****************************************************************************/
void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    (void)hModule;
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
    /* Not applicable in a VDD */
    (void)drive;
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
    /* Not applicable in a VDD */
    (void)drive;
    (void)dir;
    (void)len;
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
    return MTRR_enableWriteCombine(base,size,type);
}

/****************************************************************************
REMARKS:
Function to change the file attributes for a specific file.
****************************************************************************/
void PMAPI PM_setFileAttr(
    const char *filename,
    uint attrib)
{
    /* TODO: Implement this ? */
    (void)filename;
    (void)attrib;
    PM_fatalError("PM_setFileAttr not implemented!");
}

/****************************************************************************
REMARKS:
Function to get the file attributes for a specific file.
****************************************************************************/
uint PMAPI PM_getFileAttr(
    const char *filename)
{
    /* TODO: Implement this ? */
    (void)filename;
    PM_fatalError("PM_getFileAttr not implemented!");
    return 0;
}

/****************************************************************************
REMARKS:
Function to create a directory.
****************************************************************************/
ibool PMAPI PM_mkdir(
    const char *filename)
{
    /* TODO: Implement this ? */
    (void)filename;
    PM_fatalError("PM_mkdir not implemented!");
    return false;
}

/****************************************************************************
REMARKS:
Function to remove a directory.
****************************************************************************/
ibool PMAPI PM_rmdir(
    const char *filename)
{
    /* TODO: Implement this ? */
    (void)filename;
    PM_fatalError("PM_rmdir not implemented!");
    return false;
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
    /* TODO: Implement this ? */
    (void)filename;
    (void)gmTime;
    (void)time;
    PM_fatalError("PM_getFileTime not implemented!");
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
    /* TODO: Implement this ? */
    (void)filename;
    (void)gmTime;
    (void)time;
    PM_fatalError("PM_setFileTime not implemented!");
    return false;
}
