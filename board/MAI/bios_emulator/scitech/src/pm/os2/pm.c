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
* Environment:  32-bit OS/2
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
#include "pm_help.h"
#include "mtrr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#ifndef __EMX__
#include <direct.h>
#endif
#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_SUB
#define INCL_VIO
#define INCL_KBD
#include <os2.h>

/* Semaphore for communication with our background daemon */
#define SHAREDSEM   ((PSZ)"\\SEM32\\SDD\\DAEMON")
#define DAEMON_NAME "SDDDAEMN.EXE"

/*--------------------------- Global variables ----------------------------*/

/* Public structures used to communicate with VIDEOPMI for implementing
 * the ability to call the real mode BIOS functions.
 */

typedef struct _VIDEOMODEINFO {
    ULONG   miModeId;
    USHORT  usType;
    USHORT  usInt10ModeSet;
    USHORT  usXResolution;
    USHORT  usYResolution;
    ULONG   ulBufferAddress;
    ULONG   ulApertureSize;
    BYTE    bBitsPerPixel;
    BYTE    bBitPlanes;
    BYTE    bXCharSize;
    BYTE    bYCharSize;
    USHORT  usBytesPerScanLine;
    USHORT  usTextRows;
    ULONG   ulPageLength;
    ULONG   ulSaveSize;
    BYTE    bVrtRefresh;
    BYTE    bHrtRefresh;
    BYTE    bVrtPolPos;
    BYTE    bHrtPolPos;
    CHAR    bRedMaskSize;
    CHAR    bRedFieldPosition;
    CHAR    bGreenMaskSize;
    CHAR    bGreenFieldPosition;
    CHAR    bBlueMaskSize;
    CHAR    bBlueFieldPosition;
    CHAR    bRsvdMaskSize;
    CHAR    bRsvdFieldPosition;
    ULONG   ulColors;
    ULONG   ulReserved[3];
    } VIDEOMODEINFO, FAR *PVIDEOMODEINFO;

typedef struct _ADAPTERINFO {
    ULONG       ulAdapterID;
    CHAR        szOEMString[128];
    CHAR        szDACString[128];
    CHAR        szRevision[128];
    ULONG       ulTotalMemory;
    ULONG       ulMMIOBaseAddress;
    ULONG       ulPIOBaseAddress;
    BYTE        bBusType;
    BYTE        bEndian;
    USHORT      usDeviceBusID;
    USHORT      usVendorBusID;
    USHORT      SlotID;
    } ADAPTERINFO, FAR *PADAPTERINFO;

typedef struct _VIDEO_ADAPTER {
    void            *hvideo;
    ADAPTERINFO     Adapter;
    VIDEOMODEINFO   ModeInfo;
    } VIDEO_ADAPTER, FAR *PVIDEO_ADAPTER;

/* PMIREQUEST_SOFTWAREINT structures from OS/2 DDK */

typedef struct {
    ULONG ulFlags;                              /* VDM initialization type */
#define VDM_POSTLOAD                    0x1     /* adapter just loaded, used internally for initialization */
#define VDM_INITIALIZE                  0x2     /* force initialization of a permanently open VDM, even if previously initialized */
#define VDM_TERMINATE_POSTINITIALIZE    0x6     /*start VDM with initialization, but close it afterwards (includes VDM_INITIALIZE) */
#define VDM_QUERY_CAPABILITY            0x10    /* query the current int 10 capability */
#define VDM_FULL_VDM_CREATED            0x20    /* a full VDM is created */
#define VDM_MINI_VDM_CREATED            0x40    /* a mini VDM is created */
#define VDM_MINI_VDM_SUPPORTED          0x80    /* mini VDM support is available */
    PCHAR szName;                               /* VDM initialization program */
    PCHAR szArgs;                               /* VDM initialization arguments */
    }INITVDM;

typedef struct {
    BYTE bBufferType;
#define BUFFER_NONE     0
#define INPUT_BUFFER    1
#define OUTPUT_BUFFER   2
    BYTE bReserved;
    BYTE bSelCRF;
    BYTE bOffCRF;
    PVOID pAddress;
    ULONG ulSize;
    } BUFFER, *PBUFFER;

typedef struct vcrf_s {
    ULONG reg_eax;
    ULONG reg_ebx;
    ULONG reg_ecx;
    ULONG reg_edx;
    ULONG reg_ebp;
    ULONG reg_esi;
    ULONG reg_edi;
    ULONG reg_ds;
    ULONG reg_es;
    ULONG reg_fs;
    ULONG reg_gs;
    ULONG reg_cs;
    ULONG reg_eip;
    ULONG reg_eflag;
    ULONG reg_ss;
    ULONG reg_esp;
    } VCRF;

typedef struct {
    ULONG   ulBIOSIntNo;
    VCRF    aCRF;
    BUFFER  pB[2];
    } INTCRF;

#define PMIREQUEST_LOADPMIFILE          21
#define PMIREQUEST_IDENTIFYADAPTER      22
#define PMIREQUEST_SOFTWAREINT          23

#ifdef  PTR_DECL_IN_FRONT
#define EXPENTRYP   * EXPENTRY
#else
#define EXPENTRYP   EXPENTRY *
#endif

/* Entry point to VIDEOPMI32Request. This may be overridden by external
 * code that has already loaded VIDEOPMI to avoid loading it twice.
 */

APIRET (EXPENTRYP PM_VIDEOPMI32Request)(PVIDEO_ADAPTER, ULONG, PVOID, PVOID) = NULL;
static ibool        haveInt10 = -1; /* True if we have Int 10 support   */
static ibool        useVPMI = true; /* False if VIDEOPMI unavailable    */
static VIDEO_ADAPTER Adapter;       /* Video adapter for VIDEOPMI       */
static uchar        RMBuf[1024];    /* Fake real mode transfer buffer   */
static uint         VESABuf_len = 1024;/* Length of the VESABuf buffer  */
static void         *VESABuf_ptr = NULL;/* Near pointer to VESABuf      */
static uint         VESABuf_rseg;   /* Real mode segment of VESABuf     */
static uint         VESABuf_roff;   /* Real mode offset of VESABuf      */
static uchar *      lowMem = NULL;
static ibool        isSessionSwitching = false;
static ulong        parmsIn[4];     /* Must not cross 64Kb boundary!    */
static ulong        parmsOut[4];    /* Must not cross 64Kb boundary!    */
extern ushort       _PM_gdt;
static void (PMAPIP fatalErrorCleanup)(void) = NULL;

/* DosSysCtl prototype. It is not declared in the headers but it is in the
 * standard import libraries (DOSCALLS.876). Funny.
 */
APIRET APIENTRY DosSysCtl(ULONG ulFunction, PVOID pvData);

/* This is the stack size for the threads that track the session switch event */
#define SESSION_SWITCH_STACK_SIZE   32768

typedef struct {
    VIOMODEINFO     vmi;
    USHORT          CursorX;
    USHORT          CursorY;
    UCHAR           FrameBuffer[1];
    } CONSOLE_SAVE;

typedef struct _SESWITCHREC {
    /* The following variable is volatile because of PM_SUSPEND_APP         */
    volatile int    Flags;          /* -1 or PM_DEACTIVATE or PM_REACTIVATE */
    PM_saveState_cb Callback;       /* Save/restore context callback        */
    HMTX            Mutex;          /* Exclusive access mutex               */
    HEV             Event;          /* Posted after callback is called      */
    } SESWITCHREC;

/* Page sized block cache */

#define PAGES_PER_BLOCK     32
#define PAGE_BLOCK_SIZE     (PAGES_PER_BLOCK * PM_PAGE_SIZE + (PM_PAGE_SIZE-1) + sizeof(pageblock))
#define FREELIST_NEXT(p)    (*(void**)(p))
typedef struct pageblock {
    struct pageblock    *next;
    struct pageblock    *prev;
    void                *freeListStart;
    void                *freeList;
    void                *freeListEnd;
    int                 freeCount;
    PM_lockHandle       lockHandle;
    } pageblock;

static pageblock    *pageBlocks = NULL;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
PARAMETERS:
func        - Helper device driver function to call

RETURNS:
First return value from the device driver in parmsOut[0]

REMARKS:
Function to open our helper device driver, call it and close the file
handle. Note that we have to open the device driver for every call because
of two problems:

 1. We cannot open a single file handle in a DLL that is shared amongst
    programs, since every process must have it's own open file handle.

 2. For some reason there appears to be a limit of about 12 open file
    handles on a device driver in the system. Hence when we open more
    than about 12 file handles things start to go very strange.

Hence we simply open the file handle every time that we need to call the
device driver to work around these problems.
****************************************************************************/
static ulong CallSDDHelp(
    int func)
{
    static ulong    inLen;          /* Must not cross 64Kb boundary!    */
    static ulong    outLen;         /* Must not cross 64Kb boundary!    */
    HFILE           hSDDHelp;
    ULONG           rc;
    ulong           result;

    if ((rc = DosOpen(PMHELP_NAME,&hSDDHelp,&result,0,0,
	    FILE_OPEN, OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
	    NULL)) != 0) {
	if (rc == 4) {  /* Did we run out of file handles? */
	    ULONG   ulNewFHs;
	    LONG    lAddFHs = 5;

	    if (DosSetRelMaxFH(&lAddFHs, &ulNewFHs) != 0)
		PM_fatalError("Failed to raise the file handles limit!");
	    else {
		if ((rc = DosOpen(PMHELP_NAME,&hSDDHelp,&result,0,0,
			FILE_OPEN, OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
			NULL)) != 0) {
		    PM_fatalError("Unable to open SDDHELP$ helper device driver! (#2)");
		    }
		}
	    }
	else
	    PM_fatalError("Unable to open SDDHELP$ helper device driver!");
	}
    if (DosDevIOCtl(hSDDHelp,PMHELP_IOCTL,func,
	    &parmsIn, inLen = sizeof(parmsIn), &inLen,
	    &parmsOut, outLen = sizeof(parmsOut), &outLen) != 0)
	PM_fatalError("Failure calling SDDHELP$ helper device driver!");
    DosClose(hSDDHelp);
    return parmsOut[0];
}

/****************************************************************************
REMARKS:
Determine if we're running on a DBCS system.
****************************************************************************/
ibool __IsDBCSSystem(void)
{
    CHAR        achDBCSInfo[12];
    COUNTRYCODE ccStruct = {0, 0};

    memset(achDBCSInfo, 0, 12);

    /* Get the DBCS vector - if it's not empty, we're on DBCS */
    DosQueryDBCSEnv(sizeof(achDBCSInfo), &ccStruct, achDBCSInfo);
    if (achDBCSInfo[0] != 0)
	return true;
    else
	return false;
}

/****************************************************************************
REMARKS:
Determine if PMSHELL is running - if it isn't, we can't use certain calls
****************************************************************************/
ibool __isShellLoaded(void)
{
    PVOID   ptr;

    if (DosGetNamedSharedMem(&ptr, (PSZ)"\\SHAREMEM\\PMGLOBAL.MEM", PAG_READ) == NO_ERROR) {
	DosFreeMem(ptr);
	return true;
	}
    return false;
}

/****************************************************************************
REMARKS:
Initialise the PM library and connect to our helper device driver. If we
cannot connect to our helper device driver, we bail out with an error
message.
****************************************************************************/
void PMAPI PM_init(void)
{
    if (!lowMem) {
	/* Obtain the 32->16 callgate from the device driver to enable IOPL */
	if ((_PM_gdt = CallSDDHelp(PMHELP_GETGDT32)) == 0)
	    PM_fatalError("Unable to obtain call gate selector!");

	PM_setIOPL(3);

	/* Map the first Mb of physical memory into lowMem */
	if ((lowMem = PM_mapPhysicalAddr(0,0xFFFFF,true)) == NULL)
	    PM_fatalError("Unable to map first Mb physical memory!");

	/* Initialise the MTRR interface functions */
	MTRR_init();
	}
}

/****************************************************************************
REMARKS:
Initialise the PM library for BIOS access via VIDEOPMI. This should work
with any GRADD driver, including SDD/2.
****************************************************************************/
static ibool InitInt10(void)
{
    HMODULE     hModGENPMI,hModSDDPMI,hModVideoPMI;
    CHAR        buf[80],path[_MAX_PATH];
    HEV         hevDaemon = NULLHANDLE;
    RESULTCODES resCodes;

    if (haveInt10 == -1) {
	/* Connect to VIDEOPMI and get entry point. Note that we only
	 * do this if GENPMI or SDDPMI are already loaded, since we need
	 * a GRADD based driver for this to work.
	 */
	PM_init();
	haveInt10 = false;
	if (DosQueryModuleHandle((PSZ)"GENPMI.DLL",&hModGENPMI) != 0)
	    hModGENPMI = NULLHANDLE;
	if (DosQueryModuleHandle((PSZ)"SDDPMI.DLL",&hModSDDPMI) != 0)
	    hModSDDPMI = NULLHANDLE;
	if (hModGENPMI || hModSDDPMI) {
	    if (DosLoadModule((PSZ)buf,sizeof(buf),(PSZ)"VIDEOPMI.DLL",&hModVideoPMI) == 0) {
		if (DosQueryProcAddr(hModVideoPMI,0,(PSZ)"VIDEOPMI32Request",(void*)&PM_VIDEOPMI32Request) != 0)
		    PM_fatalError("Unable to get VIDEOPMI32Request entry point!");
		strcpy(path,"X:\\OS2\\SVGADATA.PMI");
		path[0] = PM_getBootDrive();
		if (PM_VIDEOPMI32Request(&Adapter,PMIREQUEST_LOADPMIFILE,path,NULL) != 0) {
		    DosFreeModule(hModVideoPMI);
		    PM_VIDEOPMI32Request = NULL;
		    haveInt10 = false;
		    }
		else {
		    /* Attempt to initialise the full VDM in the system. This will only
		     * work if VPRPMI.SYS is loaded, but it provides support for passing
		     * values in ES/DS/ESI/EDI between the BIOS which does not work with
		     * kernel VDM's in fixpacks earlier than FP15. FP15 and later and
		     * the new Warp 4.51 and Warp Server convenience packs should work
		     * fine with the kernel mini-VDM.
		     *
		     * Also the full VDM is the only solution for really old kernels
		     * (but GRADD won't run on them so this is superfluous ;-).
		     */
		    INITVDM InitVDM = {VDM_INITIALIZE,NULL,NULL};
		    PM_VIDEOPMI32Request(&Adapter,PMIREQUEST_SOFTWAREINT,&InitVDM,NULL);
		    haveInt10 = true;
		    }
		}
	    }
	else {
	    /* A GRADD driver isn't loaded, hence we can't use VIDEOPMI. But we will try
	     * to access the mini-VDM directly, first verifying that the support is
	     * available in the kernel (it should be for kernels that support GRADD).
	     * This may be needed in a command line boot or if non-GRADD driver is
	     * used (Matrox or classic VGA).
	     * Note: because of problems with mini-VDM support in the kernel, we have to
	     * spawn a daemon process that will do the actual mini-VDM access for us.
	     */
	     /* Try to open shared semaphore to see if our daemon is already up */
	    if (DosOpenEventSem(SHAREDSEM, &hevDaemon) == NO_ERROR) {
		if (DosWaitEventSem(hevDaemon, 1) == NO_ERROR) {
		    /* If semaphore is posted, all is well */
		    useVPMI   = false;
		    haveInt10 = true;
		    }
		}
	    else {
		/* Create shared event semaphore */
		if (DosCreateEventSem(SHAREDSEM, &hevDaemon, DC_SEM_SHARED, FALSE) == NO_ERROR) {
		    PM_findBPD(DAEMON_NAME, path);
		    strcat(path, DAEMON_NAME);
		    if (DosExecPgm(buf, sizeof(buf), EXEC_BACKGROUND, (PSZ)DAEMON_NAME,
			NULL, &resCodes, (PSZ)path) == NO_ERROR) {
			/* The daemon was successfully spawned, now give it a sec to come up */
			if (DosWaitEventSem(hevDaemon, 2000) == NO_ERROR) {
			    /* It's up! */
			    useVPMI   = false;
			    haveInt10 = true;
			    }
			}
		    }
		}
	    }
	}
    return haveInt10;
}

/****************************************************************************
REMARKS:
We "probably" have BIOS access under OS/2 but we have to verify/initialize it
first.
****************************************************************************/
ibool PMAPI PM_haveBIOSAccess(void)
{
    return InitInt10();
}

/****************************************************************************
REMARKS:
Return the operating system type identifier.
****************************************************************************/
long PMAPI PM_getOSType(void)
{
    return _OS_OS2;
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
    /* Be prepare to be called recursively (failed to fail situation :-) */
    static int fatalErrorCount = 0;
    if (fatalErrorCount++ == 0) {
	if (fatalErrorCleanup)
	    fatalErrorCleanup();
	}
    fprintf(stderr,"%s\n", msg);
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
    if (!VESABuf_ptr) {
	/* Allocate a global buffer for communicating with the VESA VBE */
	if ((VESABuf_ptr = PM_allocRealSeg(VESABuf_len, &VESABuf_rseg, &VESABuf_roff)) == NULL)
	    return NULL;
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
    KBDKEYINFO   key;            /* Must not cross a 64K boundary    */

    KbdPeek(&key, 0);
    return (key.fbStatus & KBDTRF_FINAL_CHAR_IN);
}

/****************************************************************************
REMARKS:
Wait for and return the next keypress.
****************************************************************************/
int PMAPI PM_getch(void)
{
    KBDKEYINFO   key;            /* Must not cross a 64K boundary    */

    KbdCharIn(&key,IO_WAIT,0);
    return key.chChar;
}

/****************************************************************************
REMARKS:
Open a fullscreen console for output to the screen. This requires that
the application be a fullscreen VIO program.
****************************************************************************/
PM_HWND PMAPI PM_openConsole(
    PM_HWND hwndUser,
    int device,
    int xRes,
    int yRes,
    int bpp,
    ibool fullScreen)
{
    (void)hwndUser;
    (void)device;
    (void)xRes;
    (void)yRes;
    (void)bpp;
    (void)fullScreen;
    return 0;
}

/****************************************************************************
REMARKS:
Find the size of the console state buffer.
****************************************************************************/
int PMAPI PM_getConsoleStateSize(void)
{
    VIOMODEINFO vmi;
    vmi.cb = sizeof (VIOMODEINFO);
    VioGetMode (&vmi, (HVIO)0);
    return sizeof (CONSOLE_SAVE) - 1 + vmi.col * vmi.row * 2;
}

/****************************************************************************
REMARKS:
Save the state of the console.
****************************************************************************/
void PMAPI PM_saveConsoleState(
    void *stateBuf,
    PM_HWND hwndConsole)
{
    USHORT          fblen;
    CONSOLE_SAVE    *cs = (CONSOLE_SAVE*)stateBuf;
    VIOMODEINFO     vmi;

    /* The reason for the VIOMODEINFO juggling is 16-bit code. Because the user
     * allocates the state buffer, cd->vmi might be crossing the 64K boundary and
     * the 16-bit API would fail. If we create another copy on stack, the compiler
     * should ensure that the 64K boundary will not be crossed (it adjusts the stack
     * if it should cross).
     */
    vmi.cb = sizeof(VIOMODEINFO);
    VioGetMode(&vmi,(HVIO)0);
    memcpy(&cs->vmi, &vmi, sizeof(VIOMODEINFO));
    VioGetCurPos(&cs->CursorY, &cs->CursorX, (HVIO)0);
    fblen = cs->vmi.col * cs->vmi.row * 2;
    VioReadCellStr((PCH)cs->FrameBuffer, &fblen, 0, 0, (HVIO)0);
}

/* Global variable to communicate between threads */
static SESWITCHREC SesSwitchRec = { -1 };

/****************************************************************************
REMARKS:
Called by external routines at least once per frame to check whenever a
session save/restore should be performed. Since we receive such notifications
asyncronously, we can't perform all required operations at that time.
****************************************************************************/
void __PM_checkConsoleSwitch(void)
{
    int             Flags, Mode;
    PM_saveState_cb Callback;

    /* Quick optimized path for most common case */
    if (SesSwitchRec.Flags == -1)
	return;

again:
    if (DosRequestMutexSem(SesSwitchRec.Mutex, 100))
	return;
    Flags = SesSwitchRec.Flags;
    Callback = SesSwitchRec.Callback;
    SesSwitchRec.Flags = -1;
    DosReleaseMutexSem(SesSwitchRec.Mutex);

    isSessionSwitching = true;            /* Prevent VIO calls */
    Mode = Callback(Flags);
    isSessionSwitching = false;
    DosPostEventSem(SesSwitchRec.Event);
    if (Flags == PM_DEACTIVATE && Mode == PM_SUSPEND_APP)
	/* Suspend application until we switch back to our application */
	for (;;) {
	    DosSleep (500);
	    /* SesSwitchRec.Flags is volatile so optimizer
	     * won't load it into a register
	     */
	    if (SesSwitchRec.Flags != -1)
		goto again;
	    }
}

/****************************************************************************
REMARKS:
Waits until main thread processes the session switch event.
****************************************************************************/
static void _PM_SessionSwitchEvent(
    PM_saveState_cb saveState,
    int flags)
{
    ULONG Count;

    if (DosRequestMutexSem(SesSwitchRec.Mutex, 10000))
	return;

    /* We're going to wait on that semaphore */
    DosResetEventSem(SesSwitchRec.Event, &Count);
    SesSwitchRec.Callback = saveState;
    SesSwitchRec.Flags = flags;
    DosReleaseMutexSem(SesSwitchRec.Mutex);

    /* Now wait until all required operations are complete */
    DosWaitEventSem (SesSwitchRec.Event, 10000);
}

/****************************************************************************
REMARKS:
This is the thread responsible for tracking switches back to our
fullscreen session.
****************************************************************************/
static void _PM_ConsoleSwitch(
    PM_saveState_cb saveState)
{
    USHORT NotifyType;

    for (;;) {
	if (VioModeWait(VMWR_POPUP, &NotifyType, 0) != 0)
	    break;
	_PM_SessionSwitchEvent(saveState, PM_REACTIVATE);
	}
    VioModeUndo(UNDOI_RELEASEOWNER, UNDOK_ERRORCODE, (HVIO)0);
}

/****************************************************************************
REMARKS:
This is the thread responsible for tracking screen popups (usually fatal
error handler uses them).
****************************************************************************/
static void _PM_ConsolePopup(
    PM_saveState_cb saveState)
{
    USHORT NotifyType;
    for (;;) {
	if (VioSavRedrawWait(VSRWI_SAVEANDREDRAW, &NotifyType, 0) != 0)
	    break;
	if (NotifyType == VSRWN_SAVE)
	    _PM_SessionSwitchEvent(saveState, PM_DEACTIVATE);
	else if (NotifyType == VSRWN_REDRAW)
	    _PM_SessionSwitchEvent(saveState, PM_REACTIVATE);
	}
    VioSavRedrawUndo(UNDOI_RELEASEOWNER, UNDOK_ERRORCODE, (HVIO)0);
}

/****************************************************************************
REMARKS:
Set the suspend application callback for the fullscreen console.
****************************************************************************/
void PMAPI PM_setSuspendAppCallback(
    PM_saveState_cb saveState)
{
    /* If PM isn't loaded, this stuff will cause crashes! */
    if (__isShellLoaded()) {
	if (saveState) {
	    /* Create the threads responsible for tracking console switches */
	    SesSwitchRec.Flags = -1;
	    DosCreateMutexSem(NULL, &SesSwitchRec.Mutex, 0, FALSE);
	    DosCreateEventSem(NULL, &SesSwitchRec.Event, 0, FALSE);
	    _beginthread ((void(*)(void*))_PM_ConsoleSwitch,NULL,SESSION_SWITCH_STACK_SIZE, (void*)saveState);
	    _beginthread ((void(*)(void*))_PM_ConsolePopup,NULL,SESSION_SWITCH_STACK_SIZE, (void*)saveState);
	    }
	else {
	    /* Kill the threads responsible for tracking console switches */
	    VioModeUndo(UNDOI_RELEASEOWNER, UNDOK_TERMINATE, (HVIO)0);
	    VioSavRedrawUndo(UNDOI_RELEASEOWNER, UNDOK_TERMINATE, (HVIO)0);
	    DosCloseEventSem(SesSwitchRec.Event);
	    DosCloseMutexSem(SesSwitchRec.Mutex);
	    }
	}
}

/****************************************************************************
REMARKS:
Restore the console state.
****************************************************************************/
void PMAPI PM_restoreConsoleState(
    const void *stateBuf,
    PM_HWND hwndConsole)
{
    CONSOLE_SAVE *cs = (CONSOLE_SAVE *)stateBuf;
    VIOMODEINFO  vmi;

    if (!cs)
	return;

    memcpy(&vmi, &cs->vmi, sizeof (VIOMODEINFO));
    VioSetMode(&vmi, (HVIO)0);
    VioSetCurPos(cs->CursorY, cs->CursorX, (HVIO)0);
    VioWrtCellStr((PCH)cs->FrameBuffer, cs->vmi.col * cs->vmi.row * 2,0, 0, (HVIO)0);
}

/****************************************************************************
REMARKS:
Close the fullscreen console.
****************************************************************************/
void PMAPI PM_closeConsole(
    PM_HWND hwndConsole)
{
    /* Kill the threads responsible for tracking console switches */
    PM_setSuspendAppCallback(NULL);
    (void)hwndConsole;
}

/****************************************************************************
REMARKS:
Set the location of the OS console cursor.
****************************************************************************/
void PM_setOSCursorLocation(
    int x,
    int y)
{
    /* If session switch is in progress, calling into VIO causes deadlocks! */
    /* Also this call to VIO screws up our console library on DBCS boxes... */
    if (!isSessionSwitching && !__IsDBCSSystem())
	VioSetCurPos(y,x,0);
}

/****************************************************************************
REMARKS:
Set the width of the OS console.
****************************************************************************/
void PM_setOSScreenWidth(
    int width,
    int height)
{
    /* Nothing to do in here */
    (void)width;
    (void)height;
}

/****************************************************************************
REMARKS:
Set the real time clock handler (used for software stereo modes).
****************************************************************************/
ibool PMAPI PM_setRealTimeClockHandler(
    PM_intHandler ih,
    int frequency)
{
    /* TODO: Implement this! */
    (void)ih;
    (void)frequency;
    return false;
}

/****************************************************************************
REMARKS:
Set the real time clock frequency (for stereo modes).
****************************************************************************/
void PMAPI PM_setRealTimeClockFrequency(
    int frequency)
{
    /* TODO: Implement this! */
    (void)frequency;
}

/****************************************************************************
REMARKS:
Restore the original real time clock handler.
****************************************************************************/
void PMAPI PM_restoreRealTimeClockHandler(void)
{
    /* TODO: Implement this! */
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
    ulong   boot = 3;
    DosQuerySysInfo(QSV_BOOT_DRIVE,QSV_BOOT_DRIVE,&boot,sizeof(boot));
    return (char)('a' + boot - 1);
}

/****************************************************************************
REMARKS:
Return the path to the VBE/AF driver files.
****************************************************************************/
const char * PMAPI PM_getVBEAFPath(void)
{
    static char path[CCHMAXPATH];
    strcpy(path,"x:\\");
    path[0] = PM_getBootDrive();
    return path;
}

/****************************************************************************
REMARKS:
Return the path to the Nucleus driver files.
****************************************************************************/
const char * PMAPI PM_getNucleusPath(void)
{
    static char path[CCHMAXPATH];
    if (getenv("NUCLEUS_PATH") != NULL)
	return getenv("NUCLEUS_PATH");
    strcpy(path,"x:\\os2\\drivers");
    path[0] = PM_getBootDrive();
    PM_backslash(path);
    strcat(path,"nucleus");
    return path;
}

/****************************************************************************
REMARKS:
Return the path to the Nucleus configuration files.
****************************************************************************/
const char * PMAPI PM_getNucleusConfigPath(void)
{
    static char path[CCHMAXPATH];
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
    static char name[40],*env;

    if ((env = getenv("HOSTNAME")) != NULL) {
	strncpy(name,env,sizeof(name));
	name[sizeof(name)-1] = 0;
	return name;
	}
    return "OS2";
}

/****************************************************************************
REMARKS:
Return a pointer to the real mode BIOS data area.
****************************************************************************/
void * PMAPI PM_getBIOSPointer(void)
{
    PM_init();
    return lowMem + 0x400;
}

/****************************************************************************
REMARKS:
Return a pointer to 0xA0000 physical VGA graphics framebuffer.
****************************************************************************/
void * PMAPI PM_getA0000Pointer(void)
{
    PM_init();
    return lowMem + 0xA0000;
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
    ulong   baseAddr,baseOfs,linear;

    /* Round the physical address to a 4Kb boundary and the limit to a
     * 4Kb-1 boundary before passing the values to mmap. If we round the
     * physical address, then we also add an extra offset into the address
     * that we return.
     */
    baseOfs = base & 4095;
    baseAddr = base & ~4095;
    limit = ((limit+baseOfs+1+4095) & ~4095)-1;
    parmsIn[0] = baseAddr;
    parmsIn[1] = limit;
    parmsIn[2] = isCached;
    if ((linear = CallSDDHelp(PMHELP_MAPPHYS)) == 0)
	return NULL;
    return (void*)(linear + baseOfs);
}

/****************************************************************************
REMARKS:
Free a physical address mapping allocated by PM_mapPhysicalAddr.
****************************************************************************/
void PMAPI PM_freePhysicalAddr(
    void *ptr,
    ulong limit)
{
    parmsIn[0] = (ulong)ptr;
    parmsIn[1] = limit;
    CallSDDHelp(PMHELP_FREEPHYS);
}

/****************************************************************************
REMARKS:
Find the physical address of a linear memory address in current process.
****************************************************************************/
ulong PMAPI PM_getPhysicalAddr(
    void *p)
{
    parmsIn[0] = (ulong)p;
    return CallSDDHelp(PMHELP_GETPHYSICALADDR);
}

/****************************************************************************
REMARKS:
Find the physical address of a linear memory address in current process.
****************************************************************************/
ibool PMAPI PM_getPhysicalAddrRange(
    void *p,
    ulong length,
    ulong *physAddress)
{
    parmsIn[0] = (ulong)p;
    parmsIn[1] = (ulong)length;
    parmsIn[2] = (ulong)physAddress;
    return CallSDDHelp(PMHELP_GETPHYSICALADDRRANGE);
}

/****************************************************************************
REMARKS:
Sleep for the specified number of milliseconds.
****************************************************************************/
void PMAPI PM_sleep(
    ulong milliseconds)
{
    DosSleep(milliseconds);
}

/****************************************************************************
REMARKS:
Return the base I/O port for the specified COM port.
****************************************************************************/
int PMAPI PM_getCOMPort(
    int port)
{
    switch (port) {
	case 0: return 0x3F8;
	case 1: return 0x2F8;
	}
    return 0;
}

/****************************************************************************
REMARKS:
Return the base I/O port for the specified LPT port.
****************************************************************************/
int PMAPI PM_getLPTPort(
    int port)
{
    switch (port) {
	case 0: return 0x3BC;
	case 1: return 0x378;
	case 2: return 0x278;
	}
    return 0;
}

/****************************************************************************
REMARKS:
Allocate a block of shared memory. For Win9x we allocate shared memory
as locked, global memory that is accessible from any memory context
(including interrupt time context), which allows us to load our important
data structure and code such that we can access it directly from a ring
0 interrupt context.
****************************************************************************/
void * PMAPI PM_mallocShared(
    long size)
{
    parmsIn[0] = size;
    return (void*)CallSDDHelp(PMHELP_MALLOCSHARED);
}

/****************************************************************************
REMARKS:
Free a block of shared memory.
****************************************************************************/
void PMAPI PM_freeShared(
    void *ptr)
{
    parmsIn[0] = (ulong)ptr;
    CallSDDHelp(PMHELP_FREESHARED);
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
    ulong   baseAddr,baseOfs;

    /* Round the physical address to a 4Kb boundary and the limit to a
     * 4Kb-1 boundary before passing the values to mmap. If we round the
     * physical address, then we also add an extra offset into the address
     * that we return.
     */
    baseOfs = (ulong)base & 4095;
    baseAddr = (ulong)base & ~4095;
    limit = ((limit+baseOfs+1+4095) & ~4095)-1;
    parmsIn[0] = (ulong)baseAddr;
    parmsIn[1] = limit;
    return (void*)(CallSDDHelp(PMHELP_MAPTOPROCESS)+baseOfs);
}

/****************************************************************************
REMARKS:
Map a real mode pointer to a protected mode pointer.
****************************************************************************/
void * PMAPI PM_mapRealPointer(
    uint r_seg,
    uint r_off)
{
    if (r_seg == 0xFFFF)
	return &RMBuf[r_off];
    return lowMem + MK_PHYS(r_seg,r_off);
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
    if (size > sizeof(RMBuf))
	return NULL;
    *r_seg = 0xFFFF;
    *r_off = 0x0000;
    return &RMBuf;
}

/****************************************************************************
REMARKS:
Free a block of real mode memory.
****************************************************************************/
void PMAPI PM_freeRealSeg(
    void *mem)
{
    /* Nothing to do in here */
    (void)mem;
}

#define INDPMI(reg)     rmregs.aCRF.reg_##reg = regs->reg
#define OUTDPMI(reg)    regs->reg = rmregs.aCRF.reg_##reg

#define REG_OFFSET(field)  (((ULONG)&(((VCRF*)0)->field))  / sizeof(ULONG))

/****************************************************************************
REMARKS:
Issue a real mode interrupt (parameters in DPMI compatible structure)
****************************************************************************/
void PMAPI DPMI_int86(
    int intno,
    DPMI_regs *regs)
{
    INTCRF  rmregs;
    ulong   eax = 0;

    if (!InitInt10())
	return;
    memset(&rmregs, 0, sizeof(rmregs));
    rmregs.ulBIOSIntNo = intno;
    INDPMI(eax); INDPMI(ebx); INDPMI(ecx); INDPMI(edx); INDPMI(esi); INDPMI(edi);
    rmregs.aCRF.reg_ds = regs->ds;
    rmregs.aCRF.reg_es = regs->es;
    if (intno == 0x10) {
	eax = rmregs.aCRF.reg_eax;
	switch (eax & 0xFFFF) {
	    case 0x4F00:
		/* We have to hack the way this function works, due to
		 * some bugs in the IBM mini-VDM BIOS support. Specifically
		 * we need to make the input buffer and output buffer the
		 * 'same' buffer, and that ES:SI points to the output
		 * buffer (ignored by the BIOS). The data will end up
		 * being returned in the input buffer, except for the
		 * first four bytes ('VESA') that will not be returned.
		 */
		rmregs.pB[0].bBufferType = INPUT_BUFFER;
		rmregs.pB[0].bSelCRF = REG_OFFSET(reg_es);
		rmregs.pB[0].bOffCRF = REG_OFFSET(reg_edi);
		rmregs.pB[0].pAddress = RMBuf;
		rmregs.pB[0].ulSize = 4;
		rmregs.pB[1].bBufferType = OUTPUT_BUFFER;
		rmregs.pB[1].bSelCRF = REG_OFFSET(reg_es);
		rmregs.pB[1].bOffCRF = REG_OFFSET(reg_esi);
		rmregs.pB[1].pAddress = ((PBYTE)RMBuf)+4;
		rmregs.pB[1].ulSize = 512-4;
		break;
	    case 0x4F01:
		rmregs.pB[0].bBufferType = OUTPUT_BUFFER;
		rmregs.pB[0].bSelCRF = REG_OFFSET(reg_es);
		rmregs.pB[0].bOffCRF = REG_OFFSET(reg_edi);
		rmregs.pB[0].pAddress = RMBuf;
		rmregs.pB[0].ulSize = 256;
		break;
	    case 0x4F02:
		rmregs.pB[0].bBufferType = INPUT_BUFFER;
		rmregs.pB[0].bSelCRF = REG_OFFSET(reg_es);
		rmregs.pB[0].bOffCRF = REG_OFFSET(reg_edi);
		rmregs.pB[0].pAddress = RMBuf;
		rmregs.pB[0].ulSize = 256;
		break;
	    case 0x4F09:
		rmregs.pB[0].bBufferType = INPUT_BUFFER;
		rmregs.pB[0].bSelCRF = REG_OFFSET(reg_es);
		rmregs.pB[0].bOffCRF = REG_OFFSET(reg_edi);
		rmregs.pB[0].pAddress = RMBuf;
		rmregs.pB[0].ulSize = 1024;
		break;
	    case 0x4F0A:
		/* Due to bugs in the mini-VDM in OS/2, the 0x4F0A protected
		 * mode interface functions will not work (we never get any
		 * selectors returned), so we fail this function here. The
		 * rest of the VBE/Core driver will work properly if this
		 * function is failed, because the VBE 2.0 and 3.0 specs
		 * allow for this.
		 */
		regs->eax = 0x014F;
		return;
	    }
	}
    if (useVPMI)
	PM_VIDEOPMI32Request(&Adapter,PMIREQUEST_SOFTWAREINT,NULL,&rmregs);
    else {
	DosSysCtl(6, &rmregs);
	}

    OUTDPMI(eax); OUTDPMI(ebx); OUTDPMI(ecx); OUTDPMI(edx); OUTDPMI(esi); OUTDPMI(edi);
    if (((regs->eax & 0xFFFF) == 0x004F) && ((eax & 0xFFFF) == 0x4F00)) {
	/* Hack to fix up the missing 'VESA' string for mini-VDM */
	memcpy(RMBuf,"VESA",4);
	}
    regs->ds = rmregs.aCRF.reg_ds;
    regs->es = rmregs.aCRF.reg_es;
    regs->flags = rmregs.aCRF.reg_eflag;
}

#define IN(reg)     rmregs.reg = in->e.reg
#define OUT(reg)    out->e.reg = rmregs.reg

/****************************************************************************
REMARKS:
Issue a real mode interrupt.
****************************************************************************/
int PMAPI PM_int86(
    int intno,
    RMREGS *in,
    RMREGS *out)
{
    DPMI_regs   rmregs;

    memset(&rmregs, 0, sizeof(rmregs));
    IN(eax); IN(ebx); IN(ecx); IN(edx); IN(esi); IN(edi);
    DPMI_int86(intno,&rmregs);
    OUT(eax); OUT(ebx); OUT(ecx); OUT(edx); OUT(esi); OUT(edi);
    out->x.cflag = rmregs.flags & 0x1;
    return out->x.ax;
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
    DPMI_regs   rmregs;

    memset(&rmregs, 0, sizeof(rmregs));
    IN(eax); IN(ebx); IN(ecx); IN(edx); IN(esi); IN(edi);
    rmregs.es = sregs->es;
    rmregs.ds = sregs->ds;
    DPMI_int86(intno,&rmregs);
    OUT(eax); OUT(ebx); OUT(ecx); OUT(edx); OUT(esi); OUT(edi);
    sregs->es = rmregs.es;
    sregs->cs = rmregs.cs;
    sregs->ss = rmregs.ss;
    sregs->ds = rmregs.ds;
    out->x.cflag = rmregs.flags & 0x1;
    return out->x.ax;
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
    PM_fatalError("PM_callRealMode not supported on OS/2!");
}

/****************************************************************************
REMARKS:
Return the amount of available memory.
****************************************************************************/
void PMAPI PM_availableMemory(
    ulong *physical,
    ulong *total)
{
    /* Unable to get reliable values from OS/2 for this */
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
    parmsIn[0] = size;
    parmsIn[1] = contiguous;
    parmsIn[2] = below16M;
    CallSDDHelp(PMHELP_ALLOCLOCKED);
    *physAddr = parmsOut[1];
    return (void*)parmsOut[0];
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
    parmsIn[0] = (ulong)p;
    CallSDDHelp(PMHELP_FREELOCKED);
}

/****************************************************************************
REMARKS:
Allocates a new block of pages for the page block manager.
****************************************************************************/
static pageblock *PM_addNewPageBlock(void)
{
    int         i;
    pageblock   *newBlock;
    char        *p,*next;

    /* Allocate memory for the new page block, and add to head of list */
    if (DosAllocSharedMem((void**)&newBlock,NULL,PAGE_BLOCK_SIZE,OBJ_GETTABLE | PAG_READ | PAG_WRITE | PAG_COMMIT))
	return NULL;
    if (!PM_lockDataPages(newBlock,PAGE_BLOCK_SIZE,&newBlock->lockHandle))
	return NULL;
    newBlock->prev = NULL;
    newBlock->next = pageBlocks;
    if (pageBlocks)
	pageBlocks->prev = newBlock;
    pageBlocks = newBlock;

    /* Initialise the page aligned free list for the page block */
    newBlock->freeCount = PAGES_PER_BLOCK;
    newBlock->freeList = p = (char*)(((ulong)(newBlock + 1) + (PM_PAGE_SIZE-1)) & ~(PM_PAGE_SIZE-1));
    newBlock->freeListStart = newBlock->freeList;
    newBlock->freeListEnd = p + (PAGES_PER_BLOCK-1) * PM_PAGE_SIZE;
    for (i = 0; i < PAGES_PER_BLOCK; i++,p = next)
	FREELIST_NEXT(p) = next = p + PM_PAGE_SIZE;
    FREELIST_NEXT(p - PM_PAGE_SIZE) = NULL;
    return newBlock;
}

/****************************************************************************
REMARKS:
Allocates a page aligned and page sized block of memory
****************************************************************************/
void * PMAPI PM_allocPage(
    ibool locked)
{
    pageblock   *block;
    void        *p;

    /* Scan the block list looking for any free blocks. Allocate a new
     * page block if no free blocks are found.
     */
    for (block = pageBlocks; block != NULL; block = block->next) {
	if (block->freeCount)
	    break;
	}
    if (block == NULL && (block = PM_addNewPageBlock()) == NULL)
	return NULL;
    block->freeCount--;
    p = block->freeList;
    block->freeList = FREELIST_NEXT(p);
    (void)locked;
    return p;
}

/****************************************************************************
REMARKS:
Free a page aligned and page sized block of memory
****************************************************************************/
void PMAPI PM_freePage(
    void *p)
{
    pageblock   *block;

    /* First find the page block that this page belongs to */
    for (block = pageBlocks; block != NULL; block = block->next) {
	if (p >= block->freeListStart && p <= block->freeListEnd)
	    break;
	}
    CHECK(block != NULL);

    /* Now free the block by adding it to the free list */
    FREELIST_NEXT(p) = block->freeList;
    block->freeList = p;
    if (++block->freeCount == PAGES_PER_BLOCK) {
	/* If all pages in the page block are now free, free the entire
	 * page block itself.
	 */
	if (block == pageBlocks) {
	    /* Delete from head */
	    pageBlocks = block->next;
	    if (block->next)
		block->next->prev = NULL;
	    }
	else {
	    /* Delete from middle of list */
	    CHECK(block->prev != NULL);
	    block->prev->next = block->next;
	    if (block->next)
		block->next->prev = block->prev;
	    }

	/* Unlock the memory and free it */
	PM_unlockDataPages(block,PAGE_BLOCK_SIZE,&block->lockHandle);
	DosFreeMem(block);
	}
}

/****************************************************************************
REMARKS:
Map in all the shared memory blocks for managing the memory pages above.
****************************************************************************/
void PMAPI PM_mapSharedPages(void)
{
    pageblock   *block;

    /* Map all the page blocks above into the shared memory for process */
    for (block = pageBlocks; block != NULL; block = block->next) {
	DosGetSharedMem(block, PAG_READ | PAG_WRITE);
	}
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockDataPages(
    void *p,
    uint len,
    PM_lockHandle *lockHandle)
{
    parmsIn[0] = (ulong)p;
    parmsIn[1] = len;
    CallSDDHelp(PMHELP_LOCKPAGES);
    lockHandle->h[0] = parmsOut[1];
    lockHandle->h[1] = parmsOut[2];
    lockHandle->h[2] = parmsOut[3];
    return parmsOut[0];
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockDataPages(
    void *p,
    uint len,
    PM_lockHandle *lockHandle)
{
    parmsIn[0] = lockHandle->h[0];
    parmsIn[1] = lockHandle->h[1];
    parmsIn[2] = lockHandle->h[2];
    return CallSDDHelp(PMHELP_UNLOCKPAGES);
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockCodePages(
    void (*p)(),
    uint len,
    PM_lockHandle *lockHandle)
{
    parmsIn[0] = (ulong)p;
    parmsIn[1] = len;
    CallSDDHelp(PMHELP_LOCKPAGES);
    lockHandle->h[0] = parmsOut[1];
    lockHandle->h[1] = parmsOut[2];
    lockHandle->h[2] = parmsOut[3];
    return parmsOut[0];
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockCodePages(
    void (*p)(),
    uint len,
    PM_lockHandle *lockHandle)
{
    parmsIn[0] = lockHandle->h[0];
    parmsIn[1] = lockHandle->h[1];
    parmsIn[2] = lockHandle->h[2];
    return CallSDDHelp(PMHELP_UNLOCKPAGES);
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display banks.
****************************************************************************/
void PMAPI PM_setBankA(
    int bank)
{
    INTCRF  rmregs;

    if (!InitInt10())
	return;
    memset(&rmregs, 0, sizeof(rmregs));
    rmregs.ulBIOSIntNo = 0x10;
    rmregs.aCRF.reg_eax = 0x4F05;
    rmregs.aCRF.reg_ebx = 0x0000;
    rmregs.aCRF.reg_edx = bank;
    PM_VIDEOPMI32Request(&Adapter,PMIREQUEST_SOFTWAREINT,&rmregs,NULL);
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display banks.
****************************************************************************/
void PMAPI PM_setBankAB(
    int bank)
{
    INTCRF  rmregs;

    if (!InitInt10())
	return;
    memset(&rmregs, 0, sizeof(rmregs));
    rmregs.ulBIOSIntNo = 0x10;
    rmregs.aCRF.reg_eax = 0x4F05;
    rmregs.aCRF.reg_ebx = 0x0000;
    rmregs.aCRF.reg_edx = bank;
    PM_VIDEOPMI32Request(&Adapter,PMIREQUEST_SOFTWAREINT,&rmregs,NULL);
    rmregs.ulBIOSIntNo = 0x10;
    rmregs.aCRF.reg_eax = 0x4F05;
    rmregs.aCRF.reg_ebx = 0x0001;
    rmregs.aCRF.reg_edx = bank;
    PM_VIDEOPMI32Request(&Adapter,PMIREQUEST_SOFTWAREINT,&rmregs,NULL);
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
    INTCRF  rmregs;

    if (!InitInt10())
	return;
    memset(&rmregs, 0, sizeof(rmregs));
    rmregs.ulBIOSIntNo = 0x10;
    rmregs.aCRF.reg_eax = 0x4F07;
    rmregs.aCRF.reg_ebx = waitVRT;
    rmregs.aCRF.reg_ecx = x;
    rmregs.aCRF.reg_edx = y;
    PM_VIDEOPMI32Request(&Adapter,PMIREQUEST_SOFTWAREINT,&rmregs,NULL);
}

/****************************************************************************
REMARKS:
Execute the POST on the secondary BIOS for a controller.
****************************************************************************/
ibool PMAPI PM_doBIOSPOST(
    ushort axVal,
    ulong BIOSPhysAddr,
    void *mappedBIOS,
    ulong BIOSLen)
{
    (void)axVal;
    (void)BIOSPhysAddr;
    (void)mappedBIOS;
    (void)BIOSLen;
    return false;
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

/* TODO: Move the MTRR helper stuff into the call gate, or better yet */
/*       entirely into the ring 0 helper driver!! */

/* MTRR helper functions. To make it easier to implement the MTRR support
 * under OS/2, we simply put our ring 0 helper functions into the
 * helper device driver rather than the entire MTRR module. This makes
 * it easier to maintain the MTRR support since we don't need to deal
 * with 16-bit ring 0 code in the MTRR library.
 */

/****************************************************************************
REMARKS:
Flush the translation lookaside buffer.
****************************************************************************/
void PMAPI PM_flushTLB(void)
{
    CallSDDHelp(PMHELP_FLUSHTLB);
}

/****************************************************************************
REMARKS:
Return true if ring 0 (or if we can call the helpers functions at ring 0)
****************************************************************************/
ibool _ASMAPI _MTRR_isRing0(void)
{
    return true;
}

/****************************************************************************
REMARKS:
Read and return the value of the CR4 register
****************************************************************************/
ulong _ASMAPI _MTRR_saveCR4(void)
{
    return CallSDDHelp(PMHELP_SAVECR4);
}

/****************************************************************************
REMARKS:
Restore the value of the CR4 register
****************************************************************************/
void _ASMAPI _MTRR_restoreCR4(ulong cr4Val)
{
    parmsIn[0] = cr4Val;
    CallSDDHelp(PMHELP_RESTORECR4);
}

/****************************************************************************
REMARKS:
Read a machine status register for the CPU.
****************************************************************************/
void _ASMAPI _MTRR_readMSR(
    ulong reg,
    ulong *eax,
    ulong *edx)
{
    parmsIn[0] = reg;
    CallSDDHelp(PMHELP_READMSR);
    *eax = parmsOut[0];
    *edx = parmsOut[1];
}

/****************************************************************************
REMARKS:
Write a machine status register for the CPU.
****************************************************************************/
void _ASMAPI _MTRR_writeMSR(
    ulong reg,
    ulong eax,
    ulong edx)
{
    parmsIn[0] = reg;
    parmsIn[1] = eax;
    parmsIn[2] = edx;
    CallSDDHelp(PMHELP_WRITEMSR);
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
Internal function to convert the find data to the generic interface.
****************************************************************************/
static void convertFindData(
    PM_findData *findData,
    FILEFINDBUF3 *blk)
{
    ulong   dwSize = findData->dwSize;

    memset(findData,0,findData->dwSize);
    findData->dwSize = dwSize;
    if (blk->attrFile & FILE_READONLY)
	findData->attrib |= PM_FILE_READONLY;
    if (blk->attrFile & FILE_DIRECTORY)
	findData->attrib |= PM_FILE_DIRECTORY;
    if (blk->attrFile & FILE_ARCHIVED)
	findData->attrib |= PM_FILE_ARCHIVE;
    if (blk->attrFile & FILE_HIDDEN)
	findData->attrib |= PM_FILE_HIDDEN;
    if (blk->attrFile & FILE_SYSTEM)
	findData->attrib |= PM_FILE_SYSTEM;
    findData->sizeLo = blk->cbFile;
    findData->sizeHi = 0;
    strncpy(findData->name,blk->achName,PM_MAX_PATH);
    findData->name[PM_MAX_PATH-1] = 0;
}

#define FIND_MASK   (FILE_ARCHIVED | FILE_DIRECTORY | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY)

/****************************************************************************
REMARKS:
Function to find the first file matching a search criteria in a directory.
****************************************************************************/
void *PMAPI PM_findFirstFile(
    const char *filename,
    PM_findData *findData)
{
    FILEFINDBUF3    blk;
    HDIR            hdir = HDIR_CREATE;
    ulong           count = 1;

    if (DosFindFirst((PSZ)filename,&hdir,FIND_MASK,&blk,sizeof(blk),&count,FIL_STANDARD) == NO_ERROR) {
	convertFindData(findData,&blk);
	return (void*)hdir;
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
    FILEFINDBUF3    blk;
    ulong           count = 1;

    if (DosFindNext((HDIR)handle,&blk,sizeof(blk),&count) == NO_ERROR) {
	convertFindData(findData,&blk);
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
    DosFindClose((HDIR)handle);
}

/****************************************************************************
REMARKS:
Function to determine if a drive is a valid drive or not. Under Unix this
function will return false for anything except a value of 3 (considered
the root drive, and equivalent to C: for non-Unix systems). The drive
numbering is:

    0   - Current drive
    1   - Drive A:
    2   - Drive B:
    3   - Drive C:
    etc

****************************************************************************/
ibool PMAPI PM_driveValid(
    char drive)
{
    ulong   cntDisk,cntDriveMap;
    ibool   valid;

    DosQueryCurrentDisk(&cntDisk,&cntDriveMap);
    valid = (DosSetDefaultDisk(drive) == NO_ERROR);
    DosSetDefaultDisk(cntDisk);
    return valid;
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
    ulong length = len;

    DosQueryCurrentDir(drive, (PSZ)dir, &length);
}

/****************************************************************************
REMARKS:
Function to change the file attributes for a specific file.
****************************************************************************/
void PMAPI PM_setFileAttr(
    const char *filename,
    uint attrib)
{
    FILESTATUS3 s;

    if (DosQueryPathInfo((PSZ)filename,FIL_STANDARD,(PVOID)&s,sizeof(s)))
	return;
    s.attrFile = 0;
    if (attrib & PM_FILE_READONLY)
	s.attrFile |= FILE_READONLY;
    if (attrib & PM_FILE_ARCHIVE)
	s.attrFile |= FILE_ARCHIVED;
    if (attrib & PM_FILE_HIDDEN)
	s.attrFile |= FILE_HIDDEN;
    if (attrib & PM_FILE_SYSTEM)
	s.attrFile |= FILE_SYSTEM;
    DosSetPathInfo((PSZ)filename,FIL_STANDARD,(PVOID)&s,sizeof(s),0L);
}

/****************************************************************************
REMARKS:
Function to get the file attributes for a specific file.
****************************************************************************/
uint PMAPI PM_getFileAttr(
    const char *filename)
{
    FILESTATUS3 fs3;
    uint        retval = 0;

    if (DosQueryPathInfo((PSZ)filename, FIL_STANDARD, &fs3, sizeof(FILESTATUS3)))
	return 0;
    if (fs3.attrFile & FILE_READONLY)
	retval |= PM_FILE_READONLY;
    if (fs3.attrFile & FILE_ARCHIVED)
	retval |= PM_FILE_ARCHIVE;
    if (fs3.attrFile & FILE_HIDDEN)
	retval |= PM_FILE_HIDDEN;
    if (fs3.attrFile & FILE_SYSTEM)
	retval |= PM_FILE_SYSTEM;
    return retval;
}

/****************************************************************************
REMARKS:
Function to create a directory.
****************************************************************************/
ibool PMAPI PM_mkdir(
    const char *filename)
{
    return DosCreateDir((PSZ)filename,NULL) == NO_ERROR;
}

/****************************************************************************
REMARKS:
Function to remove a directory.
****************************************************************************/
ibool PMAPI PM_rmdir(
    const char *filename)
{
    return DosDeleteDir((PSZ)filename) == NO_ERROR;
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
    FILESTATUS3 fs3;
    struct tm   tc;
    struct tm   *ret;
    time_t      tt;

    if (DosQueryPathInfo((PSZ)filename, FIL_STANDARD, &fs3, sizeof(FILESTATUS3)))
	return false;
    if (gmTime) {
	tc.tm_year = fs3.fdateLastWrite.year + 80;
	tc.tm_mon = fs3.fdateLastWrite.month - 1;
	tc.tm_mday = fs3.fdateLastWrite.day;
	tc.tm_hour = fs3.ftimeLastWrite.hours;
	tc.tm_min = fs3.ftimeLastWrite.minutes;
	tc.tm_sec = fs3.ftimeLastWrite.twosecs * 2;
	if((tt = mktime(&tc)) == -1)
	    return false;
	if(!(ret = gmtime(&tt)))
	    return false;
	time->sec = ret->tm_sec;
	time->day = ret->tm_mday;
	time->mon = ret->tm_mon + 1;
	time->year = ret->tm_year - 80;
	time->min = ret->tm_min;
	time->hour = ret->tm_hour;
	}
    else {
	time->sec = fs3.ftimeLastWrite.twosecs * 2;
	time->day = fs3.fdateLastWrite.day;
	time->mon = fs3.fdateLastWrite.month;
	time->year = fs3.fdateLastWrite.year;
	time->min = fs3.ftimeLastWrite.minutes;
	time->hour = fs3.ftimeLastWrite.hours;
	}
    return true;
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
    FILESTATUS3 fs3;
    struct tm   tc;
    struct tm   *ret;
    time_t      tt;

    if (DosQueryPathInfo((PSZ)filename,FIL_STANDARD,(PVOID)&fs3,sizeof(fs3)))
	return false;
    if (gmTime) {
	tc.tm_year = time->year + 80;
	tc.tm_mon = time->mon - 1;
	tc.tm_mday = time->day;
	tc.tm_hour = time->hour;
	tc.tm_min = time->min;
	tc.tm_sec = time->sec;
	if((tt = mktime(&tc)) == -1)
	    return false;
	ret = localtime(&tt);
	fs3.ftimeLastWrite.twosecs = ret->tm_sec / 2;
	fs3.fdateLastWrite.day = ret->tm_mday;
	fs3.fdateLastWrite.month = ret->tm_mon + 1;
	fs3.fdateLastWrite.year = ret->tm_year - 80;
	fs3.ftimeLastWrite.minutes = ret->tm_min;
	fs3.ftimeLastWrite.hours = ret->tm_hour;
	}
    else {
	fs3.ftimeLastWrite.twosecs = time->sec / 2;
	fs3.fdateLastWrite.day = time->day;
	fs3.fdateLastWrite.month = time->mon;
	fs3.fdateLastWrite.year = time->year;
	fs3.ftimeLastWrite.minutes = time->min;
	fs3.ftimeLastWrite.hours = time->hour;
	}
    memcpy(&fs3.fdateLastAccess, &fs3.fdateLastWrite, sizeof(FDATE));
    memcpy(&fs3.fdateCreation, &fs3.fdateLastWrite, sizeof(FDATE));
    memcpy(&fs3.ftimeLastAccess, &fs3.ftimeLastWrite, sizeof(FTIME));
    memcpy(&fs3.ftimeCreation, &fs3.ftimeLastWrite, sizeof(FTIME));
    DosSetPathInfo((PSZ)filename,FIL_STANDARD,(PVOID)&fs3,sizeof(FILESTATUS3),0L);
    return true;
}
