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
* Environment:  Win32
*
* Description:  Implementation for the OS Portability Manager Library, which
*               contains functions to implement OS specific services in a
*               generic, cross platform API. Porting the OS Portability
*               Manager library is the first step to porting any SciTech
*               products to a new platform.
*
****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "pmapi.h"
#include "drvlib/os/os.h"
#include "pm_help.h"

/*--------------------------- Global variables ----------------------------*/

ibool           _PM_haveWinNT;      /* True if we are running on NT     */
static uint     VESABuf_len = 1024; /* Length of the VESABuf buffer     */
static void     *VESABuf_ptr = NULL;/* Near pointer to VESABuf          */
static uint     VESABuf_rseg;       /* Real mode segment of VESABuf     */
static uint     VESABuf_roff;       /* Real mode offset of VESABuf      */
HANDLE          _PM_hDevice = NULL; /* Handle to Win32 VxD              */
static ibool    inited = false;     /* Flags if we are initialised      */
static void     (PMAPIP fatalErrorCleanup)(void) = NULL;

static char *szMachineNameKey   = "System\\CurrentControlSet\\control\\ComputerName\\ComputerName";
static char *szMachineNameKeyNT = "System\\CurrentControlSet\\control\\ComputerName\\ActiveComputerName";
static char *szMachineName      = "ComputerName";

/*----------------------------- Implementation ----------------------------*/

/* Macro to check for a valid, loaded version of PMHELP. We check this
 * on demand when we need these services rather than when PM_init() is
 * called because if we are running on DirectDraw we don't need PMHELP.VXD.
 */

#define CHECK_FOR_PMHELP()                                                  \
{                                                                           \
    if (_PM_hDevice == INVALID_HANDLE_VALUE)                                \
	if (_PM_haveWinNT)                                                  \
	    PM_fatalError("Unable to connect to PMHELP.SYS or SDDHELP.SYS!"); \
	else                                                                  \
	    PM_fatalError("Unable to connect to PMHELP.VXD or SDDHELP.VXD!"); \
}

/****************************************************************************
REMARKS:
Initialise the PM library and connect to our helper device driver. If we
cannot connect to our helper device driver, we bail out with an error
message. Our Windows 9x VxD is dynamically loadable, so it can be loaded
after the system has started.
****************************************************************************/
void PMAPI PM_init(void)
{
    DWORD   inBuf[1];   /* Buffer to receive data from VxD  */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */
    char    cntPath[PM_MAX_PATH];
    char    *env;

    /* Create a file handle for the static VxD if possible, otherwise
     * dynamically load the PMHELP helper VxD. Note that if an old version
     * of SDD is loaded, we use the PMHELP VxD instead.
     */
    if (!inited) {
	/* Determine if we are running under Windows NT or not and
	 * set the global OS type variable.
	 */
	_PM_haveWinNT = false;
	if ((GetVersion() & 0x80000000UL) == 0)
	    _PM_haveWinNT = true;
	___drv_os_type = (_PM_haveWinNT) ? _OS_WINNT : _OS_WIN95;

	/* Now try to connect to SDDHELP.VXD or SDDHELP.SYS */
	_PM_hDevice = CreateFile(SDDHELP_MODULE_PATH, 0,0,0, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);
	if (_PM_hDevice != INVALID_HANDLE_VALUE) {
	    if (!DeviceIoControl(_PM_hDevice, PMHELP_GETVER32, NULL, 0,
		    outBuf, sizeof(outBuf), &count, NULL) || outBuf[0] < PMHELP_VERSION) {
		/* Old version of SDDHELP loaded, so use PMHELP instead */
		CloseHandle(_PM_hDevice);
		_PM_hDevice = INVALID_HANDLE_VALUE;
		}
	    }
	if (_PM_hDevice == INVALID_HANDLE_VALUE) {
	    /* First try to see if there is a currently loaded PMHELP driver.
	     * This is usually the case when we are running under Windows NT/2K.
	     */
	    _PM_hDevice = CreateFile(PMHELP_MODULE_PATH, 0,0,0, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);
	    if (_PM_hDevice == INVALID_HANDLE_VALUE) {
		/* The driver was not staticly loaded, so try creating a file handle
		 * to a dynamic version of the VxD if possible. Note that on WinNT/2K we
		 * cannot support dynamically loading the drivers.
		 */
		_PM_hDevice = CreateFile(PMHELP_VXD_PATH, 0,0,0, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);
		}
	    }
	if (_PM_hDevice != INVALID_HANDLE_VALUE) {
	    /* Call the driver to determine the version number */
	    if (!DeviceIoControl(_PM_hDevice, PMHELP_GETVER32, inBuf, sizeof(inBuf),
		    outBuf, sizeof(outBuf), &count, NULL) || outBuf[0] < PMHELP_VERSION) {
		if (_PM_haveWinNT)
		    PM_fatalError("Older version of PMHELP.SYS found!");
		else
		    PM_fatalError("Older version of PMHELP.VXD found!");
		}

	    /* Now set the current path inside the VxD so it knows what the
	     * current directory is for loading Nucleus drivers.
	     */
	    inBuf[0] = (ulong)PM_getCurrentPath(cntPath,sizeof(cntPath));
	    if (!DeviceIoControl(_PM_hDevice, PMHELP_SETCNTPATH32, inBuf, sizeof(inBuf), outBuf, sizeof(outBuf), &count, NULL))
		PM_fatalError("Unable to set VxD current path!");

	    /* Now pass down the NUCLEUS_PATH environment variable to the device
	     * driver so it can use this value if it is found.
	     */
	    if ((env = getenv("NUCLEUS_PATH")) != NULL) {
		inBuf[0] = (ulong)env;
		if (!DeviceIoControl(_PM_hDevice, PMHELP_SETNUCLEUSPATH32, inBuf, sizeof(inBuf), outBuf, sizeof(outBuf), &count, NULL))
		    PM_fatalError("Unable to set VxD Nucleus path!");
		}

	    /* Enable IOPL for ring-3 code by default if driver is present */
	    if (_PM_haveWinNT)
		PM_setIOPL(3);
	    }

	/* Indicate that we have been initialised */
	inited = true;
	}
}

/****************************************************************************
REMARKS:
We do have BIOS access under Windows 9x, but not under Windows NT.
****************************************************************************/
int PMAPI PM_setIOPL(
    int iopl)
{
    DWORD       inBuf[1];   /* Buffer to receive data from VxD  */
    DWORD       outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD       count;      /* Count of bytes returned from VxD */
    static int  cntIOPL = 0;
    int         oldIOPL = cntIOPL;

    /* Enable I/O by adjusting the I/O permissions map on Windows NT */
    if (_PM_haveWinNT) {
	CHECK_FOR_PMHELP();
	if (iopl == 3)
	    DeviceIoControl(_PM_hDevice, PMHELP_ENABLERING3IOPL, inBuf, sizeof(inBuf),outBuf, sizeof(outBuf), &count, NULL);
	else
	    DeviceIoControl(_PM_hDevice, PMHELP_DISABLERING3IOPL, inBuf, sizeof(inBuf),outBuf, sizeof(outBuf), &count, NULL);
	cntIOPL = iopl;
	return oldIOPL;
	}

    /* We always have IOPL on Windows 9x */
    return 3;
}

/****************************************************************************
REMARKS:
We do have BIOS access under Windows 9x, but not under Windows NT.
****************************************************************************/
ibool PMAPI PM_haveBIOSAccess(void)
{
    if (PM_getOSType() == _OS_WINNT)
	return false;
    else
	return _PM_hDevice != INVALID_HANDLE_VALUE;
}

/****************************************************************************
REMARKS:
Return the operating system type identifier.
****************************************************************************/
long PMAPI PM_getOSType(void)
{
    if ((GetVersion() & 0x80000000UL) == 0)
	return ___drv_os_type = _OS_WINNT;
    else
	return ___drv_os_type = _OS_WIN95;
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
    MessageBox(NULL,msg,"Fatal Error!", MB_ICONEXCLAMATION);
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
    DWORD   outBuf[4];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    /* We require the helper VxD to be loaded staticly in order to support
     * the VESA transfer buffer. We do not support dynamically allocating
     * real mode memory buffers from Win32 programs (we need a 16-bit DLL
     * for this, and Windows 9x becomes very unstable if you free the
     * memory blocks out of order).
     */
    if (!inited)
	PM_init();
    if (!VESABuf_ptr) {
	CHECK_FOR_PMHELP();
	if (DeviceIoControl(_PM_hDevice, PMHELP_GETVESABUF32, NULL, 0,
		outBuf, sizeof(outBuf), &count, NULL)) {
	    if (!outBuf[0])
		return NULL;
	    VESABuf_ptr = (void*)outBuf[0];
	    VESABuf_len = outBuf[1];
	    VESABuf_rseg = outBuf[2];
	    VESABuf_roff = outBuf[3];
	    }
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
    /* Not used in Windows */
    return true;
}

/****************************************************************************
REMARKS:
Wait for and return the next keypress.
****************************************************************************/
int PMAPI PM_getch(void)
{
    /* Not used in Windows */
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
    /* Nothing to do for Windows */
    (void)x;
    (void)y;
}

/****************************************************************************
REMARKS:
Set the width of the OS console.
****************************************************************************/
void PM_setOSScreenWidth(
    int width,
    int height)
{
    /* Nothing to do for Windows */
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
    /* We do not support this from Win32 programs. Rather the VxD handles
     * this stuff it will take care of hooking the stereo flip functions at
     * the VxD level.
     */
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
    /* Not supported under Win32 */
    (void)frequency;
}

/****************************************************************************
REMARKS:
Restore the original real time clock handler.
****************************************************************************/
void PMAPI PM_restoreRealTimeClockHandler(void)
{
    /* Not supported under Win32 */
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
Query a string from the registry (extended version).
****************************************************************************/
static ibool REG_queryStringEx(
    HKEY hKey,
    const char *szValue,
    char *value,
    ulong size)
{
    DWORD   type;

    if (RegQueryValueEx(hKey,(PCHAR)szValue,(PDWORD)NULL,(PDWORD)&type,(LPBYTE)value,(PDWORD)&size) == ERROR_SUCCESS)
	return true;
    return false;
}

/****************************************************************************
REMARKS:
Query a string from the registry.
****************************************************************************/
static ibool REG_queryString(
    const char *szKey,
    const char *szValue,
    char *value,
    DWORD size)
{
    HKEY    hKey;
    ibool   status = false;

    memset(value,0,sizeof(value));
    if (RegOpenKey(HKEY_LOCAL_MACHINE,szKey,&hKey) == ERROR_SUCCESS) {
	status = REG_queryStringEx(hKey,szValue,value,size);
	RegCloseKey(hKey);
	}
    return status;
}

/****************************************************************************
REMARKS:
Return the drive letter for the boot drive.
****************************************************************************/
char PMAPI PM_getBootDrive(void)
{
    static char path[256];
    GetSystemDirectory(path,sizeof(path));
    return path[0];
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
    static char path[256];
    char        *env;

    if ((env = getenv("NUCLEUS_PATH")) != NULL)
	return env;
    GetSystemDirectory(path,sizeof(path));
    strcat(path,"\\nucleus");
    return path;
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
    static char name[256];

    if (REG_queryString(szMachineNameKey,szMachineName,name,sizeof(name)))
	return name;
    if (REG_queryString(szMachineNameKeyNT,szMachineName,name,sizeof(name)))
	return name;
    return "Unknown";
}

/****************************************************************************
REMARKS:
Return a pointer to the real mode BIOS data area.
****************************************************************************/
void * PMAPI PM_getBIOSPointer(void)
{
    if (_PM_haveWinNT) {
	/* On Windows NT we have to map it physically directly */
	    return PM_mapPhysicalAddr(0x400, 0x1000, true);
	}
    else {
	/* For Windows 9x we can access this memory directly */
	return (void*)0x400;
	}
}

/****************************************************************************
REMARKS:
Return a pointer to 0xA0000 physical VGA graphics framebuffer.
****************************************************************************/
void * PMAPI PM_getA0000Pointer(void)
{
    if (_PM_haveWinNT) {
	/* On Windows NT we have to map it physically directly */
	    return PM_mapPhysicalAddr(0xA0000, 0x0FFFF, false);
	}
    else {
	/* Always use the 0xA0000 linear address so that we will use
	 * whatever page table mappings are set up for us (ie: for virtual
	 * bank switching.
	 */
	return (void*)0xA0000;
	}
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
    DWORD   inBuf[3];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = base;
    inBuf[1] = limit;
    inBuf[2] = isCached;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_MAPPHYS32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return (void*)outBuf[0];
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
    /* We never free the mappings under Win32 (the VxD tracks them and
     * reissues the same mappings until the system is rebooted).
     */
    (void)ptr;
    (void)limit;
}

/****************************************************************************
REMARKS:
Find the physical address of a linear memory address in current process.
****************************************************************************/
ulong PMAPI PM_getPhysicalAddr(
    void *p)
{
    DWORD   inBuf[1];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = (ulong)p;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_GETPHYSICALADDR32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return 0xFFFFFFFFUL;
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
    DWORD   inBuf[3];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = (ulong)p;
    inBuf[1] = (ulong)length;
    inBuf[2] = (ulong)physAddress;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_GETPHYSICALADDRRANGE32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return false;
}

/****************************************************************************
REMARKS:
Sleep for the specified number of milliseconds.
****************************************************************************/
void PMAPI PM_sleep(
    ulong milliseconds)
{
    Sleep(milliseconds);
}

/****************************************************************************
REMARKS:
Return the base I/O port for the specified COM port.
****************************************************************************/
int PMAPI PM_getCOMPort(int port)
{
    /* TODO: Re-code this to determine real values using the Plug and Play */
    /*       manager for the OS. */
    switch (port) {
	case 0: return 0x3F8;
	case 1: return 0x2F8;
	case 2: return 0x3E8;
	case 3: return 0x2E8;
	}
    return 0;
}

/****************************************************************************
REMARKS:
Return the base I/O port for the specified LPT port.
****************************************************************************/
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
Allocate a block of shared memory. For Win9x we allocate shared memory
as locked, global memory that is accessible from any memory context
(including interrupt time context), which allows us to load our important
data structure and code such that we can access it directly from a ring
0 interrupt context.
****************************************************************************/
void * PMAPI PM_mallocShared(
    long size)
{
    DWORD   inBuf[1];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    inBuf[0] = size;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_MALLOCSHARED32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return (void*)outBuf[0];
    return NULL;
}

/****************************************************************************
REMARKS:
Free a block of shared memory.
****************************************************************************/
void PMAPI PM_freeShared(
    void *ptr)
{
    DWORD   inBuf[1];   /* Buffer to send data to VxD       */

    inBuf[0] = (ulong)ptr;
    CHECK_FOR_PMHELP();
    DeviceIoControl(_PM_hDevice, PMHELP_FREESHARED32, inBuf, sizeof(inBuf), NULL, 0, NULL, NULL);
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
    (void)base;
    (void)limit;
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
    return (void*)(MK_PHYS(r_seg,r_off));
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
    /* We do not support dynamically allocating real mode memory buffers
     * from Win32 programs (we need a 16-bit DLL for this, and Windows
     * 9x becomes very unstable if you free the memory blocks out of order).
     */
    (void)size;
    (void)r_seg;
    (void)r_off;
    return NULL;
}

/****************************************************************************
REMARKS:
Free a block of real mode memory.
****************************************************************************/
void PMAPI PM_freeRealSeg(
    void *mem)
{
    /* Not supported in Windows */
    (void)mem;
}

/****************************************************************************
REMARKS:
Issue a real mode interrupt (parameters in DPMI compatible structure)
****************************************************************************/
void PMAPI DPMI_int86(
    int intno,
    DPMI_regs *regs)
{
    DWORD   inBuf[2];   /* Buffer to send data to VxD       */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = intno;
    inBuf[1] = (ulong)regs;
    CHECK_FOR_PMHELP();
    DeviceIoControl(_PM_hDevice, PMHELP_DPMIINT8632, inBuf, sizeof(inBuf),
	NULL, 0, &count, NULL);
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
    DWORD   inBuf[3];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = intno;
    inBuf[1] = (ulong)in;
    inBuf[2] = (ulong)out;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_INT8632, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
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
    DWORD   inBuf[4];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = intno;
    inBuf[1] = (ulong)in;
    inBuf[2] = (ulong)out;
    inBuf[3] = (ulong)sregs;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_INT86X32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
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
    DWORD   inBuf[4];   /* Buffer to send data to VxD       */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = seg;
    inBuf[1] = off;
    inBuf[2] = (ulong)in;
    inBuf[3] = (ulong)sregs;
    CHECK_FOR_PMHELP();
    DeviceIoControl(_PM_hDevice, PMHELP_CALLREALMODE32, inBuf, sizeof(inBuf),
	NULL, 0, &count, NULL);
}

/****************************************************************************
REMARKS:
Return the amount of available memory.
****************************************************************************/
void PMAPI PM_availableMemory(
    ulong *physical,
    ulong *total)
{
    /* We don't support this under Win32 at the moment */
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
    DWORD   inBuf[4];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = size;
    inBuf[1] = (ulong)physAddr;
    inBuf[2] = (ulong)contiguous;
    inBuf[3] = (ulong)below16M;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_ALLOCLOCKED32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return (void*)outBuf[0];
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
    DWORD   inBuf[3];   /* Buffer to send data to VxD       */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = (ulong)p;
    inBuf[1] = size;
    inBuf[2] = contiguous;
    CHECK_FOR_PMHELP();
    DeviceIoControl(_PM_hDevice, PMHELP_FREELOCKED32, inBuf, sizeof(inBuf),
	NULL, 0, &count, NULL);
}

/****************************************************************************
REMARKS:
Allocates a page aligned and page sized block of memory
****************************************************************************/
void * PMAPI PM_allocPage(
    ibool locked)
{
    DWORD   inBuf[2];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = locked;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_ALLOCPAGE32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return (void*)outBuf[0];
    return NULL;
}

/****************************************************************************
REMARKS:
Free a page aligned and page sized block of memory
****************************************************************************/
void PMAPI PM_freePage(
    void *p)
{
    DWORD   inBuf[1];   /* Buffer to send data to VxD       */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = (ulong)p;
    CHECK_FOR_PMHELP();
    DeviceIoControl(_PM_hDevice, PMHELP_FREEPAGE32, inBuf, sizeof(inBuf),
	NULL, 0, &count, NULL);
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    DWORD   inBuf[2];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    inBuf[0] = (ulong)p;
    inBuf[1] = len;
    inBuf[2] = (ulong)lh;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_LOCKDATAPAGES32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return 0;
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    DWORD   inBuf[2];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    inBuf[0] = (ulong)p;
    inBuf[1] = len;
    inBuf[2] = (ulong)lh;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_UNLOCKDATAPAGES32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return 0;
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    DWORD   inBuf[2];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    inBuf[0] = (ulong)p;
    inBuf[1] = len;
    inBuf[2] = (ulong)lh;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_LOCKCODEPAGES32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return 0;
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    DWORD   inBuf[2];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    inBuf[0] = (ulong)p;
    inBuf[1] = len;
    inBuf[2] = (ulong)lh;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_UNLOCKCODEPAGES32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return 0;
}

/****************************************************************************
REMARKS:
Call the VBE/Core software interrupt to change display banks.
****************************************************************************/
void PMAPI PM_setBankA(
    int bank)
{
    RMREGS  regs;
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
    DWORD   inBuf[3];   /* Buffer to send data to VxD       */
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    if (!inited)
	PM_init();
    inBuf[0] = base;
    inBuf[1] = length;
    inBuf[2] = type;
    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_ENABLELFBCOMB32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return false;
}

/****************************************************************************
REMARKS:
Get the page directory base register value
****************************************************************************/
ulong PMAPI _PM_getPDB(void)
{
    DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    CHECK_FOR_PMHELP();
    if (DeviceIoControl(_PM_hDevice, PMHELP_GETPDB32, NULL, 0,
	    outBuf, sizeof(outBuf), &count, NULL))
	return outBuf[0];
    return 0;
}

/****************************************************************************
REMARKS:
Flush the translation lookaside buffer.
****************************************************************************/
void PMAPI PM_flushTLB(void)
{
    CHECK_FOR_PMHELP();
    DeviceIoControl(_PM_hDevice, PMHELP_FLUSHTLB32, NULL, 0, NULL, 0, NULL, NULL);
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
    /* This is never done by Win32 programs, but rather done by the VxD
     * when the system boots.
     */
    (void)axVal;
    (void)BIOSPhysAddr;
    (void)mappedBIOS;
    (void)BIOSLen;
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
    return (PM_MODULE)LoadLibrary(szDLLName);
}

/****************************************************************************
REMARKS:
Get the address of a named procedure from a shared library.
****************************************************************************/
void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    return (void*)GetProcAddress((HINSTANCE)hModule,szProcName);
}

/****************************************************************************
REMARKS:
Unload a shared library.
****************************************************************************/
void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    FreeLibrary((HINSTANCE)hModule);
}

/****************************************************************************
REMARKS:
Internal function to convert the find data to the generic interface.
****************************************************************************/
static void convertFindData(
    PM_findData *findData,
    WIN32_FIND_DATA *blk)
{
    ulong   dwSize = findData->dwSize;

    memset(findData,0,findData->dwSize);
    findData->dwSize = dwSize;
    if (blk->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
	findData->attrib |= PM_FILE_READONLY;
    if (blk->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	findData->attrib |= PM_FILE_DIRECTORY;
    if (blk->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
	findData->attrib |= PM_FILE_ARCHIVE;
    if (blk->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
	findData->attrib |= PM_FILE_HIDDEN;
    if (blk->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
	findData->attrib |= PM_FILE_SYSTEM;
    findData->sizeLo = blk->nFileSizeLow;
    findData->sizeHi = blk->nFileSizeHigh;
    strncpy(findData->name,blk->cFileName,PM_MAX_PATH);
    findData->name[PM_MAX_PATH-1] = 0;
}

/****************************************************************************
REMARKS:
Function to find the first file matching a search criteria in a directory.
****************************************************************************/
void *PMAPI PM_findFirstFile(
    const char *filename,
    PM_findData *findData)
{
    WIN32_FIND_DATA blk;
    HANDLE          hfile;

    if ((hfile = FindFirstFile(filename,&blk)) != INVALID_HANDLE_VALUE) {
	convertFindData(findData,&blk);
	return (void*)hfile;
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
    WIN32_FIND_DATA blk;

    if (FindNextFile((HANDLE)handle,&blk)) {
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
    FindClose((HANDLE)handle);
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
    char    buf[5];
    int     type;

    sprintf(buf,"%c:\\", drive);
    return ((type = GetDriveType(buf)) != 0 && type != 1);
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
    /* NT stores the current directory for drive N in the magic environment */
    /* variable =N: so we simply look for that environment variable. */
    char envname[4];

    envname[0] = '=';
    envname[1] = drive - 1 + 'A';
    envname[2] = ':';
    envname[3] = '\0';
    if (GetEnvironmentVariable(envname,dir,len) == 0) {
	/* The current directory or the drive has not been set yet, so */
	/* simply set it to the root. */
	dir[0] = envname[1];
	dir[1] = ':';
	dir[2] = '\\';
	dir[3] = '\0';
	SetEnvironmentVariable(envname,dir);
	}
}

/****************************************************************************
REMARKS:
Function to change the file attributes for a specific file.
****************************************************************************/
void PMAPI PM_setFileAttr(
    const char *filename,
    uint attrib)
{
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
}

/****************************************************************************
REMARKS:
Function to get the file attributes for a specific file.
****************************************************************************/
uint PMAPI PM_getFileAttr(
    const char *filename)
{
    DWORD   attr = GetFileAttributes(filename);
    uint    attrib = 0;

    if (attr & FILE_ATTRIBUTE_READONLY)
	attrib |= PM_FILE_READONLY;
    if (attr & FILE_ATTRIBUTE_ARCHIVE)
	attrib |= PM_FILE_ARCHIVE;
    if (attr & FILE_ATTRIBUTE_HIDDEN)
	attrib |= PM_FILE_HIDDEN;
    if (attr & FILE_ATTRIBUTE_SYSTEM)
	attrib |= PM_FILE_SYSTEM;
    return attrib;
}

/****************************************************************************
REMARKS:
Function to create a directory.
****************************************************************************/
ibool PMAPI PM_mkdir(
    const char *filename)
{
    return CreateDirectory(filename,NULL);
}

/****************************************************************************
REMARKS:
Function to remove a directory.
****************************************************************************/
ibool PMAPI PM_rmdir(
    const char *filename)
{
    return RemoveDirectory(filename);
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
    HFILE       f;
    OFSTRUCT    of;
    FILETIME    utcTime,localTime;
    SYSTEMTIME  sysTime;
    ibool       status = false;

    of.cBytes = sizeof(of);
    if ((f = OpenFile(filename,&of,OF_READ)) == HFILE_ERROR)
	return false;
    if (!GetFileTime((HANDLE)f,NULL,NULL,&utcTime))
	goto Exit;
    if (!gmTime) {
	if (!FileTimeToLocalFileTime(&utcTime,&localTime))
	    goto Exit;
	}
    else
	localTime = utcTime;
    if (!FileTimeToSystemTime(&localTime,&sysTime))
	goto Exit;
    time->year = sysTime.wYear;
    time->mon = sysTime.wMonth-1;
    time->day = sysTime.wYear;
    time->hour = sysTime.wHour;
    time->min = sysTime.wMinute;
    time->sec = sysTime.wSecond;
    status = true;

Exit:
    CloseHandle((HANDLE)f);
    return status;
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
    HFILE       f;
    OFSTRUCT    of;
    FILETIME    utcTime,localTime;
    SYSTEMTIME  sysTime;
    ibool       status = false;

    of.cBytes = sizeof(of);
    if ((f = OpenFile(filename,&of,OF_WRITE)) == HFILE_ERROR)
	return false;
    sysTime.wYear = time->year;
    sysTime.wMonth = time->mon+1;
    sysTime.wYear = time->day;
    sysTime.wHour = time->hour;
    sysTime.wMinute = time->min;
    sysTime.wSecond = time->sec;
    if (!SystemTimeToFileTime(&sysTime,&localTime))
	goto Exit;
    if (!gmTime) {
	if (!LocalFileTimeToFileTime(&localTime,&utcTime))
	    goto Exit;
	}
    else
	utcTime = localTime;
    if (!SetFileTime((HANDLE)f,NULL,NULL,&utcTime))
	goto Exit;
    status = true;

Exit:
    CloseHandle((HANDLE)f);
    return status;
}
