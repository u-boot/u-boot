;****************************************************************************
;*
;*                     SciTech Nucleus Audio Architecture
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
;*              Audio API functions for Intel binary compatible drivers.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"           ; Memory model macros

BEGIN_IMPORTS_DEF   _AA_exports
SKIP_IMP    AA_status                     ; Implemented in C code
SKIP_IMP    AA_errorMsg                   ; Implemented in C code
SKIP_IMP    AA_getDaysLeft                ; Implemented in C code
SKIP_IMP    AA_registerLicense            ; Implemented in C code
SKIP_IMP    AA_enumerateDevices           ; Implemented in C code
SKIP_IMP    AA_loadDriver                 ; Implemented in C code
DECLARE_IMP AA_unloadDriver
DECLARE_IMP AA_saveOptions
END_IMPORTS_DEF

        END
