#############################################################################
#
#                   Copyright (C) 1996 SciTech Software.
#                           All rights reserved.
#
# Descripton:   Generic makefile for the PM library. Builds the library
#               file and all test programs.
#
#############################################################################

.IMPORT .IGNORE : DEBUG_AGP_DRIVER TEST_HARNESS DEBUG_SDDPMI

#----------------------------------------------------------------------------
# Add DOS extender dependant flags to command line
#----------------------------------------------------------------------------

CFLAGS          += $(DX_CFLAGS)
ASFLAGS         += $(DX_ASFLAGS)
NO_PMLIB        := 1

#----------------------------------------------------------------------------
# Include definitions specific for the target system
#----------------------------------------------------------------------------

.IF $(USE_VXD)

# Building for Win32 VxD (minimal PM library implementation)

LIBNAME         = pm
OBJECTS         = pm$O vflat$O ztimer$O cpuinfo$O mtrr$O fileio$O pcilib$O \
                  agp$O malloc$O vgastate$O gavxd$O _pm$O _mtrr$O _cpuinfo$O \
                  _int64$O _pcihelp$O
DEPEND_SRC      := vxd;common;codepage;tests
.SOURCE:           vxd common codepage tests

.ELIF $(USE_NTDRV)

# Building for NT device drivers (minimal PM library implementation)

LIBNAME         = pm
OBJECTS         = pm$O vflat$O ztimer$O cpuinfo$O mtrr$O mem$O irq$O int86$O \
                  stdio$O stdlib$O pcilib$O agp$O malloc$O vgastate$O gantdrv$O \
                  _pm$O _mtrr$O _cpuinfo$O _int64$O _pcihelp$O _irq$O
DEPEND_SRC      := ntdrv;common;codepage;tests
.SOURCE:           ntdrv common codepage tests

.ELIF $(USE_WIN32)

# Building for Win32

CFLAGS          += -DUSE_OS_JOYSTICK
LIBNAME         = pm
OBJECTS         = pm$O vflat$O event$O ddraw$O ztimer$O cpuinfo$O pcilib$O \
                  agp$O malloc$O vgastate$O gawin32$O ntservc$O _joy$O _cpuinfo$O \
                  _int64$O _pcihelp$O
DEPEND_SRC      := win32;common;codepage;tests
.SOURCE:           win32 common codepage tests

.ELIF $(USE_OS232)

# Building for OS/2

.IF $(USE_OS2GUI)
LIBNAME         = pm_pm
.ELSE
LIBNAME         = pm
.ENDIF
OBJECTS         = pm$O vflat$O event$O ztimer$O cpuinfo$O mtrr$O pcilib$O \
                  agp$O malloc$O vgastate$O gaos2$O _pmos2$O _joy$O _cpuinfo$O \
                  _int64$O _pcihelp$O dossctl$O
DEPEND_SRC      := os2;common;codepage;tests
.SOURCE:           os2 common codepage tests

.ELIF $(USE_QNX)

# Building for QNX

USE_BIOS        := 1
.IF $(USE_PHOTON)
LIBNAME         = pm_ph
.ELIF $(USE_X11)
LIBNAME         = pm_x11
.ELSE
LIBNAME         = pm
.ENDIF
OBJECTS         = pm$O vflat$O event$O ztimer$O cpuinfo$O mtrr$O pcilib$O \
                  agp$O malloc$O mtrrqnx$O unixio$O vgastate$O gaqnx$O _joy$O \
                  _mtrrqnx$O _cpuinfo$O _int64$O _pcihelp$O
DEPEND_SRC      := qnx;common;codepage;tests
.SOURCE:           qnx common codepage tests

# Indicate that this program uses Nucleus device drivers (so needs I/O access)
USE_NUCLEUS     := 1

.ELIF $(USE_LINUX)

# Building for Linux

CFLAGS          += -DENABLE_MTRR -DUSE_OS_JOYSTICK
.IF $(USE_X11)
LIBNAME         = pm_x11
.ELSE
LIBNAME         = pm
.ENDIF
OBJECTS         = pm$O vflat$O event$O ztimer$O cpuinfo$O pcilib$O \
                  agp$O malloc$O unixio$O vgastate$O galinux$O _cpuinfo$O \
                  _int64$O _pcihelp$O
DEPEND_SRC      := linux;common;codepage;tests;x11
.SOURCE:           linux common codepage tests x11

# Building a shared library
.IF $(SOFILE)
LIB             := ld
LIBFLAGS        := -r -o
CFLAGS          += -fPIC
.ENDIF

.ELIF $(USE_BEOS)

# Building for BeOS GUI

LIBNAME         = pm
OBJECTS         = pm$O vflat$O event$O ztimer$O cpuinfo$O pcilib$O \
                  agp$O malloc$O vgastate$O gabeos$O _joy$O _cpuinfo$O \
                  _int64$O _pcihelp$O
DEPEND_SRC      := beos;common;codepage;tests
.SOURCE:           beos common codepage tests

.ELIF $(USE_SMX32)

# Building for SMX

LIBNAME         = pm
OBJECTS         = pm$O pmsmx$O vflat$O event$O ztimer$O cpuinfo$O mtrr$O pcilib$O \
                  agp$O malloc$O vgastate$O gasmx$O _pm$O _pmsmx$O _mtrr$O _event$O \
                  _joy$O _cpuinfo$O _int64$O _pcihelp$O _lztimer$O
DEPEND_SRC      := smx;common;codepage;tests
.SOURCE:           smx common codepage tests

.ELIF $(USE_RTTARGET)

# Building for RTTarget-32

LIBNAME         = pm
OBJECTS         = pm$O vflat$O event$O ztimer$O cpuinfo$O mtrr$O pcilib$O \
                  agp$O malloc$O vgastate$O gartt$O _mtrr$O _joy$O _cpuinfo$O \
                  _int64$O _pcihelp$O
DEPEND_SRC      := rttarget;common;codepage;tests
.SOURCE:           rttarget common codepage tests

.ELSE

# Building for MSDOS

LIBNAME         = pm
OBJECTS         = pm$O pmdos$O vflat$O event$O ztimer$O cpuinfo$O mtrr$O \
                  agp$O malloc$O pcilib$O vgastate$O gados$O \
                  _pm$O _pmdos$O _mtrr$O _vflat$O _event$O _joy$O _pcihelp$O \
                  _cpuinfo$O _int64$O _lztimer$O _dma$O
DEPEND_SRC      := dos;common;codepage;tests
.SOURCE:           dos common codepage tests

.ENDIF

# Object modules for keyboard code pages

OBJECTS         += us_eng$O

# Common object modules

OBJECTS         += common$O
.IF $(CHECKED)
OBJECTS         += debug$O
.ENDIF

# Nucleus loader library object modules. Note that when compiling a test harness
# library we need to exclude the Nucleus loader library.

.IF $(TEST_HARNESS)
CFLAGS          += -DTEST_HARNESS -DPMLIB
LIBNAME         = pm_test
.ELSE
OBJECTS         += galib$O _ga_imp$O
.ENDIF

.IF $(DEBUG_SDDPMI)
CFLAGS          += -DDEBUG_SDDPMI
.ENDIF

# AGP library object modules

.IF $(DEBUG_AGP_DRIVER)
CFLAGS          += -DDEBUG_AGP_DRIVER
OBJECTS         += agplib$O
.ELSE
OBJECTS         += agplib$O peloader$O libcimp$O _gatimer$O
.ENDIF

#----------------------------------------------------------------------------
# Name of library and generic object files required to build it
#----------------------------------------------------------------------------

.IF $(STKCALL)
LIBFILE         = s$(LP)$(LIBNAME)$L
.ELSE
LIBFILE         = $(LP)$(LIBNAME)$L
.ENDIF
LIBCLEAN        = *.lib *.a

#----------------------------------------------------------------------------
# Change destination for library file depending the extender being used. This
# is only necessary for DOS extender since the file go into a subdirectory
# in the normal library directory, one for each supported extender. Other
# OS'es put the file into the regular library directory, since there is
# only one per OS in this case.
#----------------------------------------------------------------------------

MK_PMODE        = 1

.IF $(TEST_HARNESS)
LIB_DEST        := $(LIB_BASE)
.ELIF $(USE_TNT)
LIB_DEST        := $(LIB_BASE)\tnt
.ELIF $(USE_DOS4GW)
LIB_DEST        := $(LIB_BASE)\dos4gw
.ELIF $(USE_X32)
LIB_DEST        := $(LIB_BASE)\x32
.ELIF $(USE_DPMI16)
LIB_DEST        := $(LIB_BASE)\dpmi16
.ELIF $(USE_DPMI32)
LIB_DEST        := $(LIB_BASE)\dpmi32
.ELIF $(USE_DOSX)
LIB_DEST        := $(LIB_BASE)\dosx
.END

#----------------------------------------------------------------------------
# Names of all executable files built
#----------------------------------------------------------------------------

.IF $(USE_REALDOS)
EXEFILES        = memtest$E biosptr$E video$E isvesa$E callreal$E       \
                  mouse$E tick$E key$E key15$E brk$E altbrk$E           \
                  critical$E altcrit$E vftest$E rtc$E getch$E           \
                  cpu$E timerc$E timercpp$E showpci$E uswc$E block$E
.ELSE
EXEFILES        = memtest$E video$E isvesa$E callreal$E vftest$E getch$E \
                  cpu$E timerc$E timercpp$E showpci$E uswc$E block$E \
                  save$E restore$E
.ENDIF

all: $(EXEFILES)

$(EXEFILES): $(LIBFILE)

memtest$E:  memtest$O
biosptr$E:  biosptr$O
video$E:    video$O
isvesa$E:   isvesa$O
mouse$E:    mouse$O
tick$E:     tick$O
key$E:      key$O
key15$E:    key15$O
brk$E:      brk$O
altbrk$E:   altbrk$O
critical$E: critical$O
altcrit$E:  altcrit$O
callreal$E: callreal$O
vftest$E:   vftest$O
rtc$E:      rtc$O
getch$E:    getch$O
cpu$E:      cpu$O
timerc$E:   timerc$O
timercpp$E: timercpp$O
showpci$E:  showpci$O
uswc$E:     uswc$O
block$E:    block$O
save$E:     save$O
restore$E:  restore$O
test$E:     test$O _test$O

#----------------------------------------------------------------------------
# Define the list of object files to create dependency information for
#----------------------------------------------------------------------------

DEPEND_OBJ      := $(OBJECTS) memtest$O biosptr$O video$O isvesa$O mouse$O  \
                   tick$O key$O key$O brk$O altbrk$O critical$O altcrit$O   \
                   callreal$O vftest$O getch$O timercpp$O

.INCLUDE: "$(SCITECH)/makedefs/common.mk"

