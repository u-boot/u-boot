;****************************************************************************
;*
;*                  SciTech OS Portability Manager Library
;*
;*               Copyright (C) 1991-1998 SciTech Software, Inc.
;*                            All rights reserved.
;*
;*  ======================================================================
;*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
;*  |                                                                    |
;*  |This copyrighted computer code contains proprietary technology      |
;*  |owned by SciTech Software, Inc., located at 505 Wall Street,        |
;*  |Chico, CA 95928 USA (http://www.scitechsoft.com).                   |
;*  |                                                                    |
;*  |The contents of this file are subject to the SciTech Nucleus        |
;*  |License; you may *not* use this file or related software except in  |
;*  |compliance with the License. You may obtain a copy of the License   |
;*  |at http://www.scitechsoft.com/nucleus-license.txt                   |
;*  |                                                                    |
;*  |Software distributed under the License is distributed on an         |
;*  |"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or      |
;*  |implied. See the License for the specific language governing        |
;*  |rights and limitations under the License.                           |
;*  |                                                                    |
;*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
;*  ======================================================================
;*
;* Language:    TASM 4.0 or NASM
;* Environment: IBM PC 32 bit Protected Mode.
;*
;* Description: Module to implement the import stubs for all the PM
;*              API functions for Intel binary portable drivers.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"           ; Memory model macros

BEGIN_IMPORTS_DEF   _PM_imports
DECLARE_IMP PM_getModeType,0
DECLARE_IMP PM_getBIOSPointer,0
DECLARE_IMP PM_getA0000Pointer,0
DECLARE_IMP PM_mapPhysicalAddr,0
DECLARE_IMP PM_mallocShared,0
SKIP_IMP    _PM_reserved1,0
DECLARE_IMP PM_freeShared,0
DECLARE_IMP PM_mapToProcess,0
DECLARE_IMP PM_mapRealPointer,0
DECLARE_IMP PM_allocRealSeg,0
DECLARE_IMP PM_freeRealSeg,0
DECLARE_IMP PM_allocLockedMem,0
DECLARE_IMP PM_freeLockedMem,0
DECLARE_IMP PM_callRealMode,0
DECLARE_IMP PM_int86,0
DECLARE_IMP PM_int86x,0
DECLARE_IMP DPMI_int86,0
DECLARE_IMP PM_availableMemory,0
DECLARE_IMP PM_getVESABuf,0
DECLARE_IMP PM_getOSType,0
DECLARE_IMP PM_fatalError,0
DECLARE_IMP PM_setBankA,0
DECLARE_IMP PM_setBankAB,0
DECLARE_IMP PM_setCRTStart,0
DECLARE_IMP PM_getCurrentPat,0
DECLARE_IMP PM_getVBEAFPath,0
DECLARE_IMP PM_getNucleusPath,0
DECLARE_IMP PM_getNucleusConfigPath,0
DECLARE_IMP PM_getUniqueID,0
DECLARE_IMP PM_getMachineName,0
DECLARE_IMP VF_available,0
DECLARE_IMP VF_init,0
DECLARE_IMP VF_exit,0
DECLARE_IMP PM_openConsole,0
DECLARE_IMP PM_getConsoleStateSize,0
DECLARE_IMP PM_saveConsoleState,0
DECLARE_IMP PM_restoreConsoleState,0
DECLARE_IMP PM_closeConsole,0
DECLARE_IMP PM_setOSCursorLocation,0
DECLARE_IMP PM_setOSScreenWidth,0
DECLARE_IMP PM_enableWriteCombine,0
DECLARE_IMP PM_backslash,0
DECLARE_IMP PM_lockDataPages,0
DECLARE_IMP PM_unlockDataPages,0
DECLARE_IMP PM_lockCodePages,0
DECLARE_IMP PM_unlockCodePages,0
DECLARE_IMP PM_setRealTimeClockHandler,0
DECLARE_IMP PM_setRealTimeClockFrequency,0
DECLARE_IMP PM_restoreRealTimeClockHandler,0
DECLARE_IMP PM_doBIOSPOST,0
DECLARE_IMP PM_getBootDrive,0
DECLARE_IMP PM_freePhysicalAddr,0
DECLARE_IMP PM_inpb,0
DECLARE_IMP PM_inpw,0
DECLARE_IMP PM_inpd,0
DECLARE_IMP PM_outpb,0
DECLARE_IMP PM_outpw,0
DECLARE_IMP PM_outpd,0
SKIP_IMP    _PM_reserved2,0
DECLARE_IMP PM_setSuspendAppCallback,0
DECLARE_IMP PM_haveBIOSAccess,0
DECLARE_IMP PM_kbhit,0
DECLARE_IMP PM_getch,0
DECLARE_IMP PM_findBPD,0
DECLARE_IMP PM_getPhysicalAddr,0
DECLARE_IMP PM_sleep,0
DECLARE_IMP PM_getCOMPort,0
DECLARE_IMP PM_getLPTPort,0
DECLARE_IMP PM_loadLibrary,0
DECLARE_IMP PM_getProcAddress,0
DECLARE_IMP PM_freeLibrary,0
DECLARE_IMP PCI_enumerate,0
DECLARE_IMP PCI_accessReg,0
DECLARE_IMP PCI_setHardwareIRQ,0
DECLARE_IMP PCI_generateSpecialCyle,0
SKIP_IMP    _PM_reserved3,0
DECLARE_IMP PCIBIOS_getEntry,0
DECLARE_IMP CPU_getProcessorType,0
DECLARE_IMP CPU_haveMMX,0
DECLARE_IMP CPU_have3DNow,0
DECLARE_IMP CPU_haveSSE,0
DECLARE_IMP CPU_haveRDTSC,0
DECLARE_IMP CPU_getProcessorSpeed,0
DECLARE_IMP ZTimerInit,0
DECLARE_IMP LZTimerOn,0
DECLARE_IMP LZTimerLap,0
DECLARE_IMP LZTimerOff,0
DECLARE_IMP LZTimerCount,0
DECLARE_IMP LZTimerOnExt,0
DECLARE_IMP LZTimerLapExt,0
DECLARE_IMP LZTimerOffExt,0
DECLARE_IMP LZTimerCountExt,0
DECLARE_IMP ULZTimerOn,0
DECLARE_IMP ULZTimerLap,0
DECLARE_IMP ULZTimerOff,0
DECLARE_IMP ULZTimerCount,0
DECLARE_IMP ULZReadTime,0
DECLARE_IMP ULZElapsedTime,0
DECLARE_IMP ULZTimerResolution,0
DECLARE_IMP PM_findFirstFile,0
DECLARE_IMP PM_findNextFile,0
DECLARE_IMP PM_findClose,0
DECLARE_IMP PM_makepath,0
DECLARE_IMP PM_splitpath,0
DECLARE_IMP PM_driveValid,0
DECLARE_IMP PM_getdcwd,0
DECLARE_IMP PM_setFileAttr,0
DECLARE_IMP PM_mkdir,0
DECLARE_IMP PM_rmdir,0
DECLARE_IMP PM_getFileAttr,0
DECLARE_IMP PM_getFileTime,0
DECLARE_IMP PM_setFileTime,0
DECLARE_IMP CPU_getProcessorName,0
DECLARE_IMP PM_getVGAStateSize,0
DECLARE_IMP PM_saveVGAState,0
DECLARE_IMP PM_restoreVGAState,0
DECLARE_IMP PM_vgaBlankDisplay,0
DECLARE_IMP PM_vgaUnblankDisplay,0
DECLARE_IMP PM_blockUntilTimeout,0
DECLARE_IMP _PM_add64,0
DECLARE_IMP _PM_sub64,0
DECLARE_IMP _PM_mul64,0
DECLARE_IMP _PM_div64,0
DECLARE_IMP _PM_shr64,0
DECLARE_IMP _PM_sar64,0
DECLARE_IMP _PM_shl64,0
DECLARE_IMP _PM_neg64,0
DECLARE_IMP PCI_findBARSize,0
DECLARE_IMP PCI_readRegBlock,0
DECLARE_IMP PCI_writeRegBlock,0
DECLARE_IMP PM_flushTLB,0
DECLARE_IMP PM_useLocalMalloc,0
DECLARE_IMP PM_malloc,0
DECLARE_IMP PM_calloc,0
DECLARE_IMP PM_realloc,0
DECLARE_IMP PM_free,0
DECLARE_IMP PM_getPhysicalAddrRange,0
DECLARE_IMP PM_allocPage,0
DECLARE_IMP PM_freePage,0
DECLARE_IMP PM_agpInit,0
DECLARE_IMP PM_agpExit,0
DECLARE_IMP PM_agpReservePhysical,0
DECLARE_IMP PM_agpReleasePhysical,0
DECLARE_IMP PM_agpCommitPhysical,0
DECLARE_IMP PM_agpFreePhysical,0
DECLARE_IMP PCI_getNumDevices,0
DECLARE_IMP PM_setLocalBPDPath,0
DECLARE_IMP PM_loadDirectDraw,0
DECLARE_IMP PM_unloadDirectDraw,0
DECLARE_IMP PM_getDirectDrawWindow,0
DECLARE_IMP PM_doSuspendApp,0
END_IMPORTS_DEF

        END

