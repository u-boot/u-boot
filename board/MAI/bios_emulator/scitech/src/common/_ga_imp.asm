;****************************************************************************
;*
;*                  SciTech Nucleus Graphics Architecture
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
;* Description: Module to implement the import stubs for all the Nucleus
;*              Graphics API functions for Intel binary compatible drivers.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"           ; Memory model macros

BEGIN_IMPORTS_DEF   __GA_exports
SKIP_IMP    GA_status,0                     ; Implemented in C code
SKIP_IMP    GA_errorMsg,1                   ; Implemented in C code
SKIP_IMP    GA_getDaysLeft,1                ; Implemented in C code
SKIP_IMP    GA_registerLicense,2            ; Implemented in C code
SKIP_IMP    GA_enumerateDevices,1           ; Implemented in C code
SKIP_IMP    GA_loadDriver,2                 ; Implemented in C code
DECLARE_IMP GA_setActiveDevice,1
SKIP_IMP    GA_reserved1,0                  ; Implemented in C code
DECLARE_IMP GA_unloadDriver,1
DECLARE_IMP REF2D_loadDriver,6
DECLARE_IMP REF2D_unloadDriver,2
DECLARE_IMP GA_loadRef2d,5
DECLARE_IMP GA_unloadRef2d,1
DECLARE_IMP GA_softStereoInit,1
DECLARE_IMP GA_softStereoOn,0
DECLARE_IMP GA_softStereoScheduleFlip,2
DECLARE_IMP GA_softStereoGetFlipStatus,0
DECLARE_IMP GA_softStereoWaitTillFlipped,0
DECLARE_IMP GA_softStereoOff,0
DECLARE_IMP GA_softStereoExit,0
DECLARE_IMP GA_saveModeProfile,2
DECLARE_IMP GA_saveOptions,2
DECLARE_IMP GA_saveCRTCTimings,1
DECLARE_IMP GA_restoreCRTCTimings,1
DECLARE_IMP DDC_init,1
DECLARE_IMP DDC_readEDID,5
DECLARE_IMP EDID_parse,3
DECLARE_IMP MCS_begin,1
DECLARE_IMP MCS_getCapabilitiesString,2
DECLARE_IMP MCS_isControlSupported,1
DECLARE_IMP MCS_enableControl,2
DECLARE_IMP MCS_getControlMax,2
DECLARE_IMP MCS_getControlValue,2
DECLARE_IMP MCS_getControlValues,3
DECLARE_IMP MCS_setControlValue,2
DECLARE_IMP MCS_setControlValues,3
DECLARE_IMP MCS_resetControl,1
DECLARE_IMP MCS_saveCurrentSettings,0
DECLARE_IMP MCS_getTimingReport,3
DECLARE_IMP MCS_getSelfTestReport,3
DECLARE_IMP MCS_end,0
SKIP_IMP    GA_loadInGUI,1                  ; Implemented in C code
DECLARE_IMP DDC_writeEDID,6
DECLARE_IMP GA_useDoubleScan,1
DECLARE_IMP GA_getMaxRefreshRate,4
DECLARE_IMP GA_computeCRTCTimings,6
DECLARE_IMP GA_addMode,5
DECLARE_IMP GA_addRefresh,5
DECLARE_IMP GA_delMode,5
DECLARE_IMP N_getLogName,0
SKIP_IMP2   N_log
DECLARE_IMP MDBX_getErrCode,0
DECLARE_IMP MDBX_getErrorMsg,0
DECLARE_IMP MDBX_open,1
DECLARE_IMP MDBX_close,0
DECLARE_IMP MDBX_first,1
DECLARE_IMP MDBX_last,1
DECLARE_IMP MDBX_next,1
DECLARE_IMP MDBX_prev,1
DECLARE_IMP MDBX_insert,1
DECLARE_IMP MDBX_update,1
DECLARE_IMP MDBX_flush,0
DECLARE_IMP MDBX_importINF,2
SKIP_IMP    GA_getGlobalOptions,2           ; Implemented in C code
DECLARE_IMP GA_setGlobalOptions,1
DECLARE_IMP GA_saveGlobalOptions,1
DECLARE_IMP GA_getInternalName,1
DECLARE_IMP GA_getNucleusConfigPath,0
DECLARE_IMP GA_getFakePCIID,0
SKIP_IMP    GA_loadLibrary,3                ; Implemented in C code
SKIP_IMP    GA_isOEMVersion,1               ; Implemented in C code
DECLARE_IMP GA_isLiteVersion,1
DECLARE_IMP GA_getDisplaySerialNo,1
DECLARE_IMP GA_getDisplayUserName,1
SKIP_IMP    GA_getCurrentDriver,1           ; Implemented in C code
SKIP_IMP    GA_getCurrentRef2d,1            ; Implemented in C code
SKIP_IMP    GA_getLicensedDevices,1         ; Implemented in C code
DECLARE_IMP DDC_initExt,2
DECLARE_IMP MCS_beginExt,2
DECLARE_IMP GA_loadRegionMgr,3
DECLARE_IMP GA_unloadRegionMgr,1
DECLARE_IMP GA_getProcAddress,2
DECLARE_IMP GA_enableVBEMode,5
DECLARE_IMP GA_disableVBEMode,5
DECLARE_IMP GA_loadModeProfile,2
DECLARE_IMP GA_getCRTCTimings,4
DECLARE_IMP GA_setCRTCTimings,4
DECLARE_IMP GA_setDefaultRefresh,6
DECLARE_IMP GA_saveMonitorInfo,2
DECLARE_IMP GA_detectPnPMonitor,3
SKIP_IMP3   GA_queryFunctions
SKIP_IMP3   REF2D_queryFunctions
END_IMPORTS_DEF

        END

