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
* Environment:  32-bit Windows NT device drivers.
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
#include "oshdr.h"

/*--------------------------- Global variables ----------------------------*/

char                _PM_cntPath[PM_MAX_PATH] = "";
char                _PM_nucleusPath[PM_MAX_PATH] = "";
static void (PMAPIP fatalErrorCleanup)(void) = NULL;

static char *szNTWindowsKey     = "\\REGISTRY\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion";
static char *szNTSystemRoot     = "SystemRoot";
static char *szMachineNameKey   = "\\REGISTRY\\Machine\\System\\CurrentControlSet\\control\\ComputerName\\ComputerName";
static char *szMachineNameKeyNT = "\\REGISTRY\\Machine\\System\\CurrentControlSet\\control\\ComputerName\\ActiveComputerName";
static char *szMachineName      = "ComputerName";

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Initialise the PM library.
****************************************************************************/
void PMAPI PM_init(void)
{
    /* Initialiase the MTRR module */
    MTRR_init();
}

/****************************************************************************
REMARKS:
Return the operating system type identifier.
****************************************************************************/
long PMAPI PM_getOSType(void)
{
    return _OS_WINNTDRV;
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
void PMAPI PM_backslash(char *s)
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
Handle fatal errors internally in the driver.
****************************************************************************/
void PMAPI PM_fatalError(
    const char *msg)
{
    ULONG   BugCheckCode = 0;
    ULONG   MoreBugCheckData[4] = {0};
    char    *p;
    ULONG   len;

    if (fatalErrorCleanup)
	fatalErrorCleanup();

#ifdef DBG	/* Send output to debugger, just return so as not to force a reboot */
#pragma message("INFO: building for debug, PM_fatalError() re-routed")
	DBGMSG2("SDDHELP> PM_fatalError(): ERROR: %s\n", msg);
	return ;
#endif
    /* KeBugCheckEx brings down the system in a controlled */
    /* manner when the caller discovers an unrecoverable */
    /* inconsistency that would corrupt the system if */
    /* the caller continued to run. */
    /* */
    /* hack - dump the first 20 chars in hex using the variables */
    /*      provided - Each ULONG is equal to four characters... */
    for(len = 0; len < 20; len++)
	if (msg[len] == (char)0)
	    break;

    /* This looks bad but it's quick and reliable... */
    p = (char *)&BugCheckCode;
    if(len > 0) p[3] = msg[0];
    if(len > 1) p[2] = msg[1];
    if(len > 2) p[1] = msg[2];
    if(len > 3) p[0] = msg[3];

    p = (char *)&MoreBugCheckData[0];
    if(len > 4) p[3] = msg[4];
    if(len > 5) p[2] = msg[5];
    if(len > 6) p[1] = msg[6];
    if(len > 7) p[0] = msg[7];

    p = (char *)&MoreBugCheckData[1];
    if(len > 8) p[3] = msg[8];
    if(len > 9) p[2] = msg[9];
    if(len > 10) p[1] = msg[10];
    if(len > 11) p[0] = msg[11];

    p = (char *)&MoreBugCheckData[2];
    if(len > 12) p[3] = msg[12];
    if(len > 13) p[2] = msg[13];
    if(len > 14) p[1] = msg[14];
    if(len > 15) p[0] = msg[15];

    p = (char *)&MoreBugCheckData[3];
    if(len > 16) p[3] = msg[16];
    if(len > 17) p[2] = msg[17];
    if(len > 18) p[1] = msg[18];
    if(len > 19) p[0] = msg[19];

    /* Halt the system! */
    KeBugCheckEx(BugCheckCode, MoreBugCheckData[0], MoreBugCheckData[1], MoreBugCheckData[2], MoreBugCheckData[3]);
}

/****************************************************************************
REMARKS:
Return the current operating system path or working directory.
****************************************************************************/
char * PMAPI PM_getCurrentPath(
    char *path,
    int maxLen)
{
    strncpy(path,_PM_cntPath,maxLen);
    path[maxLen-1] = 0;
    return path;
}

/****************************************************************************
PARAMETERS:
szKey       - Key to query (can contain version number formatting)
szValue     - Value to get information for
value       - Place to store the registry key data read
size        - Size of the string buffer to read into

RETURNS:
true if the key was found, false if not.
****************************************************************************/
static ibool REG_queryString(
    char *szKey,
    const char *szValue,
    char *value,
    DWORD size)
{
    ibool                           status;
    NTSTATUS                        rval;
    ULONG                           length;
    HANDLE                          Handle;
    OBJECT_ATTRIBUTES               keyAttributes;
    UNICODE_STRING                  *uniKey = NULL;
    UNICODE_STRING                  *uniValue = NULL;
    PKEY_VALUE_FULL_INFORMATION		fullInfo = NULL;
    STRING                          stringdata;
    UNICODE_STRING                  unidata;

    /* Convert strings to UniCode */
    status = false;
    if ((uniKey = _PM_CStringToUnicodeString(szKey)) == NULL)
	goto Exit;
    if ((uniValue = _PM_CStringToUnicodeString(szValue)) == NULL)
	goto Exit;

    /* Open the key */
    InitializeObjectAttributes( &keyAttributes,
				uniKey,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL );
    rval = ZwOpenKey( &Handle,
		      KEY_ALL_ACCESS,
		      &keyAttributes );
    if (!NT_SUCCESS(rval))
	goto Exit;

    /* Query the value */
    length = sizeof (KEY_VALUE_FULL_INFORMATION)
	   + size * sizeof(WCHAR);
    if ((fullInfo = ExAllocatePool (PagedPool, length)) == NULL)
	goto Exit;
    RtlZeroMemory(fullInfo, length);
    rval = ZwQueryValueKey (Handle,
			    uniValue,
			    KeyValueFullInformation,
			    fullInfo,
			    length,
			    &length);
    if (NT_SUCCESS (rval)) {
	/* Create the UniCode string so we can convert it */
	unidata.Buffer = (PWCHAR)(((PCHAR)fullInfo) + fullInfo->DataOffset);
	unidata.Length = (USHORT)fullInfo->DataLength;
	unidata.MaximumLength = (USHORT)fullInfo->DataLength + sizeof(WCHAR);

	/* Convert unicode univalue to ansi string. */
	rval = RtlUnicodeStringToAnsiString(&stringdata, &unidata, TRUE);
	if (NT_SUCCESS(rval)) {
	    strcpy(value,stringdata.Buffer);
	    status = true;
	    }
	}

Exit:
    if (fullInfo) ExFreePool(fullInfo);
    if (uniKey) _PM_FreeUnicodeString(uniKey);
    if (uniValue) _PM_FreeUnicodeString(uniValue);
    return status;
}

/****************************************************************************
REMARKS:
Return the drive letter for the boot drive.
****************************************************************************/
char PMAPI PM_getBootDrive(void)
{
    char path[256];
    if (REG_queryString(szNTWindowsKey,szNTSystemRoot,path,sizeof(path)))
	return 'c';
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

    if (strlen(_PM_nucleusPath) > 0) {
	strcpy(path,_PM_nucleusPath);
	PM_backslash(path);
	return path;
	}
    if (!REG_queryString(szNTWindowsKey,szNTSystemRoot,path,sizeof(path)))
	strcpy(path,"c:\\winnt");
    PM_backslash(path);
    strcat(path,"system32\\nucleus");
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
Check if a key has been pressed.
****************************************************************************/
int PMAPI PM_kbhit(void)
{
    /* Not used in NT drivers */
    return true;
}

/****************************************************************************
REMARKS:
Wait for and return the next keypress.
****************************************************************************/
int PMAPI PM_getch(void)
{
    /* Not used in NT drivers */
    return 0xD;
}

/****************************************************************************
REMARKS:
Open a console for output to the screen, creating the main event handling
window if necessary.
****************************************************************************/
PM_HWND PMAPI PM_openConsole(
    PM_HWND hwndUser,
    int device,
    int xRes,
    int yRes,
    int bpp,
    ibool fullScreen)
{
    /* Not used in NT drivers */
    (void)hwndUser;
    (void)device;
    (void)xRes;
    (void)yRes;
    (void)bpp;
    (void)fullScreen;
    return NULL;
}

/****************************************************************************
REMARKS:
Find the size of the console state buffer.
****************************************************************************/
int PMAPI PM_getConsoleStateSize(void)
{
    /* Not used in NT drivers */
    return 1;
}

/****************************************************************************
REMARKS:
Save the state of the console.
****************************************************************************/
void PMAPI PM_saveConsoleState(
    void *stateBuf,
    PM_HWND hwndConsole)
{
    /* Not used in NT drivers */
    (void)stateBuf;
    (void)hwndConsole;
}

/****************************************************************************
REMARKS:
Set the suspend application callback for the fullscreen console.
****************************************************************************/
void PMAPI PM_setSuspendAppCallback(
    PM_saveState_cb saveState)
{
    /* Not used in NT drivers */
    (void)saveState;
}

/****************************************************************************
REMARKS:
Restore the console state.
****************************************************************************/
void PMAPI PM_restoreConsoleState(
    const void *stateBuf,
    PM_HWND hwndConsole)
{
    /* Not used in NT drivers */
    (void)stateBuf;
    (void)hwndConsole;
}

/****************************************************************************
REMARKS:
Close the fullscreen console.
****************************************************************************/
void PMAPI PM_closeConsole(
    PM_HWND hwndConsole)
{
    /* Not used in NT drivers */
    (void)hwndConsole;
}

/****************************************************************************
REMARKS:
Set the location of the OS console cursor.
****************************************************************************/
void PMAPI PM_setOSCursorLocation(
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
void PMAPI PM_setOSScreenWidth(
    int width,
    int height)
{
    /* Nothing to do for Windows */
    (void)width;
    (void)height;
}

/****************************************************************************
REMARKS:
Maps a shared memory block into process address space. Does nothing since
the memory blocks are already globally mapped into all processes.
****************************************************************************/
void * PMAPI PM_mapToProcess(
    void *base,
    ulong limit)
{
    /* Not used anymore */
    (void)base;
    (void)limit;
    return NULL;
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
    /* This may not be possible in NT and should be done by the OS anyway */
    (void)axVal;
    (void)BIOSPhysAddr;
    (void)mappedBIOS;
    (void)BIOSLen;
    return false;
}

/****************************************************************************
REMARKS:
Return a pointer to the real mode BIOS data area.
****************************************************************************/
void * PMAPI PM_getBIOSPointer(void)
{
    /* Note that on NT this probably does not do what we expect! */
    return PM_mapPhysicalAddr(0x400, 0x1000, true);
}

/****************************************************************************
REMARKS:
Return a pointer to 0xA0000 physical VGA graphics framebuffer.
****************************************************************************/
void * PMAPI PM_getA0000Pointer(void)
{
    return PM_mapPhysicalAddr(0xA0000,0xFFFF,false);
}

/****************************************************************************
REMARKS:
Sleep for the specified number of milliseconds.
****************************************************************************/
void PMAPI PM_sleep(
    ulong milliseconds)
{
    /* We never use this in NT drivers */
    (void)milliseconds;
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
Returns available memory. Not possible under Windows.
****************************************************************************/
void PMAPI PM_availableMemory(
    ulong *physical,
    ulong *total)
{
    *physical = *total = 0;
}

/****************************************************************************
REMARKS:
OS specific shared libraries not supported inside a VxD
****************************************************************************/
PM_MODULE PMAPI PM_loadLibrary(
    const char *szDLLName)
{
    /* Not used in NT drivers */
    (void)szDLLName;
    return NULL;
}

/****************************************************************************
REMARKS:
OS specific shared libraries not supported inside a VxD
****************************************************************************/
void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    /* Not used in NT drivers */
    (void)hModule;
    (void)szProcName;
    return NULL;
}

/****************************************************************************
REMARKS:
OS specific shared libraries not supported inside a VxD
****************************************************************************/
void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    /* Not used in NT drivers */
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
    /* Not supported in NT drivers */
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
    /* Not supported in NT drivers */
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
    NTSTATUS                status;
    ACCESS_MASK             DesiredAccess = GENERIC_READ | GENERIC_WRITE;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    ULONG                   ShareAccess = FILE_SHARE_READ;
    ULONG                   CreateDisposition = FILE_OPEN;
    HANDLE                  FileHandle = NULL;
    UNICODE_STRING          *uniFile = NULL;
    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_BASIC_INFORMATION  FileBasic;
    char                    kernelFilename[PM_MAX_PATH+5];
    ULONG                   FileAttributes = 0;

    /* Convert file attribute flags */
    if (attrib & PM_FILE_READONLY)
	FileAttributes |= FILE_ATTRIBUTE_READONLY;
    if (attrib & PM_FILE_ARCHIVE)
	FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
    if (attrib & PM_FILE_HIDDEN)
	FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
    if (attrib & PM_FILE_SYSTEM)
	FileAttributes |= FILE_ATTRIBUTE_SYSTEM;

    /* Add prefix for addressing the file system. "\??\" is short for "\DosDevices\" */
    strcpy(kernelFilename, "\\??\\");
    strcat(kernelFilename, filename);

    /* Convert filename string to ansi string */
    if ((uniFile = _PM_CStringToUnicodeString(kernelFilename)) == NULL)
	goto Exit;

    /* Must open a file to query it's attributes */
    InitializeObjectAttributes (&ObjectAttributes,
				uniFile,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL );
    status = ZwCreateFile( &FileHandle,
			    DesiredAccess | SYNCHRONIZE,
			    &ObjectAttributes,
			    &IoStatusBlock,
			    NULL,                  /*AllocationSize  OPTIONAL, */
			    FILE_ATTRIBUTE_NORMAL,
			    ShareAccess,
			    CreateDisposition,
			    FILE_RANDOM_ACCESS,        /*CreateOptions, */
			    NULL,                  /*EaBuffer  OPTIONAL, */
			    0                      /*EaLength (required if EaBuffer) */
			    );
    if (!NT_SUCCESS (status))
	goto Exit;

    /* Query timestamps */
    status = ZwQueryInformationFile(FileHandle,
				    &IoStatusBlock,
				    &FileBasic,
				    sizeof(FILE_BASIC_INFORMATION),
				    FileBasicInformation
				    );
    if (!NT_SUCCESS (status))
	goto Exit;

    /* Change the four bits we change */
    FileBasic.FileAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE
				  | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    FileBasic.FileAttributes |= FileAttributes;

    /* Set timestamps */
    ZwSetInformationFile(   FileHandle,
			    &IoStatusBlock,
			    &FileBasic,
			    sizeof(FILE_BASIC_INFORMATION),
			    FileBasicInformation
			    );

Exit:
    if (FileHandle) ZwClose(FileHandle);
    if (uniFile) _PM_FreeUnicodeString(uniFile);
    return;
}

/****************************************************************************
REMARKS:
Function to get the file attributes for a specific file.
****************************************************************************/
uint PMAPI PM_getFileAttr(
    const char *filename)
{
    NTSTATUS                status;
    ACCESS_MASK             DesiredAccess = GENERIC_READ | GENERIC_WRITE;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    ULONG                   ShareAccess = FILE_SHARE_READ;
    ULONG                   CreateDisposition = FILE_OPEN;
    HANDLE                  FileHandle = NULL;
    UNICODE_STRING          *uniFile = NULL;
    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_BASIC_INFORMATION  FileBasic;
    char                    kernelFilename[PM_MAX_PATH+5];
    ULONG                   FileAttributes = 0;
    uint                    retval = 0;

    /* Add prefix for addressing the file system. "\??\" is short for "\DosDevices\" */
    strcpy(kernelFilename, "\\??\\");
    strcat(kernelFilename, filename);

    /* Convert filename string to ansi string */
    if ((uniFile = _PM_CStringToUnicodeString(kernelFilename)) == NULL)
	goto Exit;

    /* Must open a file to query it's attributes */
    InitializeObjectAttributes (&ObjectAttributes,
				uniFile,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL );
    status = ZwCreateFile( &FileHandle,
			   DesiredAccess | SYNCHRONIZE,
			   &ObjectAttributes,
			   &IoStatusBlock,
			   NULL,                  /*AllocationSize  OPTIONAL, */
			   FILE_ATTRIBUTE_NORMAL,
			   ShareAccess,
			   CreateDisposition,
			   FILE_RANDOM_ACCESS,        /*CreateOptions, */
			   NULL,                  /*EaBuffer  OPTIONAL, */
			   0                      /*EaLength (required if EaBuffer) */
			   );
    if (!NT_SUCCESS (status))
	goto Exit;

    /* Query timestamps */
    status = ZwQueryInformationFile(FileHandle,
				    &IoStatusBlock,
				    &FileBasic,
				    sizeof(FILE_BASIC_INFORMATION),
				    FileBasicInformation
				    );
    if (!NT_SUCCESS (status))
	goto Exit;

    /* Translate the file attributes */
    if (FileBasic.FileAttributes & FILE_ATTRIBUTE_READONLY)
	retval |= PM_FILE_READONLY;
    if (FileBasic.FileAttributes & FILE_ATTRIBUTE_ARCHIVE)
	retval |= PM_FILE_ARCHIVE;
    if (FileBasic.FileAttributes & FILE_ATTRIBUTE_HIDDEN)
	retval |= PM_FILE_HIDDEN;
    if (FileBasic.FileAttributes & FILE_ATTRIBUTE_SYSTEM)
	retval |= PM_FILE_SYSTEM;

Exit:
    if (FileHandle) ZwClose(FileHandle);
    if (uniFile) _PM_FreeUnicodeString(uniFile);
    return retval;
}

/****************************************************************************
REMARKS:
Function to create a directory.
****************************************************************************/
ibool PMAPI PM_mkdir(
    const char *filename)
{
    /* Not supported in NT drivers */
    (void)filename;
    return false;
}

/****************************************************************************
REMARKS:
Function to remove a directory.
****************************************************************************/
ibool PMAPI PM_rmdir(
    const char *filename)
{
    /* Not supported in NT drivers */
    (void)filename;
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
    /* Not supported in NT drivers */
    (void)filename;
    (void)gmTime;
    (void)time;
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
    /* Not supported in NT drivers */
    (void)filename;
    (void)gmTime;
    (void)time;
    return false;
}
