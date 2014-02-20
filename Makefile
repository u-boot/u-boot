#
# (C) Copyright 2000-2013
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

VERSION = 2014
PATCHLEVEL = 04
SUBLEVEL =
EXTRAVERSION = -rc1
NAME =

# *DOCUMENTATION*
# To see a list of typical targets execute "make help"
# More info can be located in ./README
# Comments in this file are targeted only to the developer, do not
# expect to learn how to build the kernel reading this file.

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";
MAKEFLAGS += -rR --no-print-directory

# Avoid funny character set dependencies
unexport LC_ALL
LC_COLLATE=C
LC_NUMERIC=C
export LC_COLLATE LC_NUMERIC

# We are using a recursive build, so we need to do a little thinking
# to get the ordering right.
#
# Most importantly: sub-Makefiles should only ever modify files in
# their own directory. If in some directory we have a dependency on
# a file in another dir (which doesn't happen often, but it's often
# unavoidable when linking the built-in.o targets which finally
# turn into vmlinux), we will call a sub make in that other dir, and
# after that we are sure that everything which is in that other dir
# is now up to date.
#
# The only cases where we need to modify files which have global
# effects are thus separated out and done before the recursive
# descending is started. They are now explicitly listed as the
# prepare rule.

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

# Call a source code checker (by default, "sparse") as part of the
# C compilation.
#
# Use 'make C=1' to enable checking of only re-compiled files.
# Use 'make C=2' to enable checking of *all* source files, regardless
# of whether they are re-compiled or not.
#
# See the file "Documentation/sparse.txt" for more details, including
# where to get the "sparse" utility.

ifeq ("$(origin C)", "command line")
  KBUILD_CHECKSRC = $(C)
endif
ifndef KBUILD_CHECKSRC
  KBUILD_CHECKSRC = 0
endif

# Use make M=dir to specify directory of external module to build
# Old syntax make ... SUBDIRS=$PWD is still supported
# Setting the environment variable KBUILD_EXTMOD take precedence
ifdef SUBDIRS
  KBUILD_EXTMOD ?= $(SUBDIRS)
endif

ifeq ("$(origin M)", "command line")
  KBUILD_EXTMOD := $(M)
endif

# kbuild supports saving output files in a separate directory.
# To locate output files in a separate directory two syntaxes are supported.
# In both cases the working directory must be the root of the kernel src.
# 1) O=
# Use "make O=dir/to/store/output/files/"
#
# 2) Set KBUILD_OUTPUT
# Set the environment variable KBUILD_OUTPUT to point to the directory
# where the output files shall be placed.
# export KBUILD_OUTPUT=dir/to/store/output/files/
# make
#
# The O= assignment takes precedence over the KBUILD_OUTPUT environment
# variable.


# KBUILD_SRC is set on invocation of make in OBJ directory
# KBUILD_SRC is not intended to be used by the regular user (for now)
ifeq ($(KBUILD_SRC),)

# OK, Make called in directory where kernel src resides
# Do we want to locate output files in a separate directory?
ifeq ("$(origin O)", "command line")
  KBUILD_OUTPUT := $(O)
endif

ifeq ("$(origin W)", "command line")
  export KBUILD_ENABLE_EXTRA_GCC_CHECKS := $(W)
endif

# That's our default target when none is given on the command line
PHONY := _all
_all:

# Cancel implicit rules on top Makefile
$(CURDIR)/Makefile Makefile: ;

ifneq ($(KBUILD_OUTPUT),)
# Invoke a second make in the output directory, passing relevant variables
# check that the output directory actually exists
saved-output := $(KBUILD_OUTPUT)
KBUILD_OUTPUT := $(shell cd $(KBUILD_OUTPUT) && /bin/pwd)
$(if $(KBUILD_OUTPUT),, \
     $(error output directory "$(saved-output)" does not exist))

PHONY += $(MAKECMDGOALS) sub-make

$(filter-out _all sub-make $(CURDIR)/Makefile, $(MAKECMDGOALS)) _all: sub-make
	@:

sub-make: FORCE
	$(if $(KBUILD_VERBOSE:1=),@)$(MAKE) -C $(KBUILD_OUTPUT) \
	KBUILD_SRC=$(CURDIR) \
	KBUILD_EXTMOD="$(KBUILD_EXTMOD)" -f $(CURDIR)/Makefile \
	$(filter-out _all sub-make,$(MAKECMDGOALS))

# Leave processing to above invocation of make
skip-makefile := 1
endif # ifneq ($(KBUILD_OUTPUT),)
endif # ifeq ($(KBUILD_SRC),)

# We process the rest of the Makefile if this is the final invocation of make
ifeq ($(skip-makefile),)

# If building an external module we do not care about the all: rule
# but instead _all depend on modules
PHONY += all
ifeq ($(KBUILD_EXTMOD),)
_all: all
else
_all: modules
endif

srctree		:= $(if $(KBUILD_SRC),$(KBUILD_SRC),$(CURDIR))
objtree		:= $(CURDIR)
src		:= $(srctree)
obj		:= $(objtree)

VPATH		:= $(srctree)$(if $(KBUILD_EXTMOD),:$(KBUILD_EXTMOD))

export srctree objtree VPATH

OBJTREE		:= $(objtree)
SPLTREE		:= $(OBJTREE)/spl
TPLTREE		:= $(OBJTREE)/tpl
SRCTREE		:= $(srctree)
TOPDIR		:= $(SRCTREE)
export	TOPDIR SRCTREE OBJTREE SPLTREE TPLTREE

MKCONFIG	:= $(SRCTREE)/mkconfig
export MKCONFIG

# Make sure CDPATH settings don't interfere
unexport CDPATH

#########################################################################

TIMESTAMP_FILE = include/generated/timestamp_autogenerated.h
VERSION_FILE = include/generated/version_autogenerated.h

HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/x86/ \
	    -e s/sun4u/sparc64/ \
	    -e s/arm.*/arm/ \
	    -e s/sa110/arm/ \
	    -e s/ppc64/powerpc/ \
	    -e s/ppc/powerpc/ \
	    -e s/macppc/powerpc/\
	    -e s/sh.*/sh/)

HOSTOS := $(shell uname -s | tr '[:upper:]' '[:lower:]' | \
	    sed -e 's/\(cygwin\).*/cygwin/')

export	HOSTARCH HOSTOS

# Deal with colliding definitions from tcsh etc.
VENDOR=

#########################################################################

# set default to nothing for native builds
ifeq ($(HOSTARCH),$(ARCH))
CROSS_COMPILE ?=
endif

# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

HOSTCC       = gcc
HOSTCFLAGS   = -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer

ifeq ($(HOSTOS),cygwin)
HOSTCFLAGS	+= -ansi
endif

# Mac OS X / Darwin's C preprocessor is Apple specific.  It
# generates numerous errors and warnings.  We want to bypass it
# and use GNU C's cpp.	To do this we pass the -traditional-cpp
# option to the compiler.  Note that the -traditional-cpp flag
# DOES NOT have the same semantics as GNU C's flag, all it does
# is invoke the GNU preprocessor in stock ANSI/ISO C fashion.
#
# Apple's linker is similar, thanks to the new 2 stage linking
# multiple symbol definitions are treated as errors, hence the
# -multiply_defined suppress option to turn off this error.
#
ifeq ($(HOSTOS),darwin)
# get major and minor product version (e.g. '10' and '6' for Snow Leopard)
DARWIN_MAJOR_VERSION	= $(shell sw_vers -productVersion | cut -f 1 -d '.')
DARWIN_MINOR_VERSION	= $(shell sw_vers -productVersion | cut -f 2 -d '.')

os_x_before	= $(shell if [ $(DARWIN_MAJOR_VERSION) -le $(1) -a \
	$(DARWIN_MINOR_VERSION) -le $(2) ] ; then echo "$(3)"; else echo "$(4)"; fi ;)

# Snow Leopards build environment has no longer restrictions as described above
HOSTCC       = $(call os_x_before, 10, 5, "cc", "gcc")
HOSTCFLAGS  += $(call os_x_before, 10, 4, "-traditional-cpp")
HOSTLDFLAGS += $(call os_x_before, 10, 5, "-multiply_defined suppress")
endif

# Decide whether to build built-in, modular, or both.
# Normally, just do built-in.

KBUILD_MODULES :=
KBUILD_BUILTIN := 1

#	If we have only "make modules", don't compile built-in objects.
#	When we're building modules with modversions, we need to consider
#	the built-in objects during the descend as well, in order to
#	make sure the checksums are up to date before we record them.

ifeq ($(MAKECMDGOALS),modules)
  KBUILD_BUILTIN := $(if $(CONFIG_MODVERSIONS),1)
endif

#	If we have "make <whatever> modules", compile modules
#	in addition to whatever we do anyway.
#	Just "make" or "make all" shall build modules as well

# U-Boot does not need modules
#ifneq ($(filter all _all modules,$(MAKECMDGOALS)),)
#  KBUILD_MODULES := 1
#endif

#ifeq ($(MAKECMDGOALS),)
#  KBUILD_MODULES := 1
#endif

export KBUILD_MODULES KBUILD_BUILTIN
export KBUILD_CHECKSRC KBUILD_SRC KBUILD_EXTMOD

# Beautify output
# ---------------------------------------------------------------------------
#
# Normally, we echo the whole command before executing it. By making
# that echo $($(quiet)$(cmd)), we now have the possibility to set
# $(quiet) to choose other forms of output instead, e.g.
#
#         quiet_cmd_cc_o_c = Compiling $(RELDIR)/$@
#         cmd_cc_o_c       = $(CC) $(c_flags) -c -o $@ $<
#
# If $(quiet) is empty, the whole command will be printed.
# If it is set to "quiet_", only the short version will be printed. 
# If it is set to "silent_", nothing will be printed at all, since
# the variable $(silent_cmd_cc_o_c) doesn't exist.
#
# A simple variant is to prefix commands with $(Q) - that's useful
# for commands that shall be hidden in non-verbose mode.
#
#	$(Q)ln $@ :<
#
# If KBUILD_VERBOSE equals 0 then the above command will be hidden.
# If KBUILD_VERBOSE equals 1 then the above command is displayed.

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands

ifneq ($(filter s% -s%,$(MAKEFLAGS)),)
  quiet=silent_
endif

export quiet Q KBUILD_VERBOSE


# Look for make include files relative to root of kernel src
MAKEFLAGS += --include-dir=$(srctree)

# We need some generic definitions (do not try to remake the file).
$(srctree)/scripts/Kbuild.include: ;
include $(srctree)/scripts/Kbuild.include

# Make variables (CC, etc...)

AS		= $(CROSS_COMPILE)as
# Always use GNU ld
ifneq ($(shell $(CROSS_COMPILE)ld.bfd -v 2> /dev/null),)
LD		= $(CROSS_COMPILE)ld.bfd
else
LD		= $(CROSS_COMPILE)ld
endif
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
LDR		= $(CROSS_COMPILE)ldr
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
AWK		= awk
RANLIB		= $(CROSS_COMPILE)RANLIB
DTC		= dtc
CHECK		= sparse

CHECKFLAGS     := -D__linux__ -Dlinux -D__STDC__ -Dunix -D__unix__ \
		  -Wbitwise -Wno-return-void -D__CHECK_ENDIAN__ $(CF)

KBUILD_CPPFLAGS := -D__KERNEL__

KBUILD_CFLAGS   := -Wall -Wstrict-prototypes \
		   -Wno-format-security \
		   -fno-builtin -ffreestanding
KBUILD_AFLAGS   := -D__ASSEMBLY__

U_BOOT_VERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(EXTRAVERSION)

export VERSION PATCHLEVEL SUBLEVEL U_BOOT_VERSION
export ARCH CPU BOARD VENDOR SOC
export CONFIG_SHELL HOSTCC HOSTCFLAGS HOSTLDFLAGS CROSS_COMPILE AS LD CC
export CPP AR NM LDR STRIP OBJCOPY OBJDUMP
export MAKE AWK
export DTC CHECK CHECKFLAGS

export KBUILD_CPPFLAGS NOSTDINC_FLAGS UBOOTINCLUDE
export KBUILD_CFLAGS KBUILD_AFLAGS

# When compiling out-of-tree modules, put MODVERDIR in the module
# tree rather than in the kernel tree. The kernel tree might
# even be read-only.
export MODVERDIR := $(if $(KBUILD_EXTMOD),$(firstword $(KBUILD_EXTMOD))/).tmp_versions

# Files to ignore in find ... statements

RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o -name CVS \
		   -o -name .pc -o -name .hg -o -name .git \) -prune -o
export RCS_TAR_IGNORE := --exclude SCCS --exclude BitKeeper --exclude .svn \
			 --exclude CVS --exclude .pc --exclude .hg --exclude .git

# ===========================================================================
# Rules shared between *config targets and build targets

# Basic helpers built in scripts/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic
	$(Q)rm -f .tmp_quiet_recordmcount

# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;

PHONY += outputmakefile
# outputmakefile generates a Makefile in the output directory, if using a
# separate output directory. This allows convenient use of make in the
# output directory.
outputmakefile:
ifneq ($(KBUILD_SRC),)
	$(Q)ln -fsn $(srctree) source
	$(Q)$(CONFIG_SHELL) $(srctree)/scripts/mkmakefile \
	    $(srctree) $(objtree) $(VERSION) $(PATCHLEVEL)
endif

# To make sure we do not include .config for any of the *config targets
# catch them early, and hand them over to scripts/kconfig/Makefile
# It is allowed to specify more targets when calling make, including
# mixing *config targets and build targets.
# For example 'make oldconfig all'.
# Detect when mixed targets is specified, and make a second invocation
# of make so .config is not included in this case either (for *config).

no-dot-config-targets := clean clobber mrproper distclean \
			 cscope TAGS %tags help %docs check% coccicheck \
			 backup

config-targets := 0
mixed-targets  := 0
dot-config     := 1

ifneq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-dot-config-targets), $(MAKECMDGOALS)),)
		dot-config := 0
	endif
endif

ifeq ($(KBUILD_EXTMOD),)
        ifneq ($(filter config %config,$(MAKECMDGOALS)),)
                config-targets := 1
                ifneq ($(filter-out config %config,$(MAKECMDGOALS)),)
                        mixed-targets := 1
                endif
        endif
endif

ifeq ($(mixed-targets),1)
# ===========================================================================
# We're called with mixed targets (*config and build targets).
# Handle them one by one.

PHONY += $(MAKECMDGOALS) build-one-by-one

$(MAKECMDGOALS): build-one-by-one
	@:

build-one-by-one:
	$(Q)set -e; \
	for i in $(MAKECMDGOALS); do \
		$(MAKE) -f $(srctree)/Makefile $$i; \
	done

else
ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

# Read arch specific Makefile to set KBUILD_DEFCONFIG as needed.
# KBUILD_DEFCONFIG may point out an alternative default configuration
# used for 'make defconfig'

%_config:: outputmakefile
	@$(MKCONFIG) -A $(@:_config=)

else
# ===========================================================================
# Build targets only - this includes vmlinux, arch specific targets, clean
# targets and others. In general all targets except *config targets.

# load ARCH, BOARD, and CPU configuration
-include include/config.mk

ifeq ($(dot-config),1)
# Read in config
-include include/autoconf.mk
-include include/autoconf.mk.dep

# load other configuration
include $(srctree)/config.mk

ifeq ($(wildcard include/config.mk),)
$(error "System not configured - see README")
endif

ifeq ($(__HAVE_ARCH_GENERIC_BOARD),)
ifneq ($(CONFIG_SYS_GENERIC_BOARD),)
$(error Your architecture does not support generic board. \
Please undefined CONFIG_SYS_GENERIC_BOARD in your board config file)
endif
endif

# If board code explicitly specified LDSCRIPT or CONFIG_SYS_LDSCRIPT, use
# that (or fail if absent).  Otherwise, search for a linker script in a
# standard location.

LDSCRIPT_MAKEFILE_DIR = $(dir $(LDSCRIPT))

ifndef LDSCRIPT
	#LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds.debug
	ifdef CONFIG_SYS_LDSCRIPT
		# need to strip off double quotes
		LDSCRIPT := $(CONFIG_SYS_LDSCRIPT:"%"=%)
	endif
endif

# If there is no specified link script, we look in a number of places for it
ifndef LDSCRIPT
	ifeq ($(CONFIG_NAND_U_BOOT),y)
		LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot-nand.lds
		ifeq ($(wildcard $(LDSCRIPT)),)
			LDSCRIPT := $(TOPDIR)/$(CPUDIR)/u-boot-nand.lds
		endif
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		LDSCRIPT := $(TOPDIR)/$(CPUDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		LDSCRIPT := $(TOPDIR)/arch/$(ARCH)/cpu/u-boot.lds
		# We don't expect a Makefile here
		LDSCRIPT_MAKEFILE_DIR =
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
$(error could not find linker script)
	endif
endif

else


endif # $(dot-config)

KBUILD_CFLAGS += -Os #-fomit-frame-pointer

ifdef BUILD_TAG
KBUILD_CFLAGS += -DBUILD_TAG='"$(BUILD_TAG)"'
endif

KBUILD_CFLAGS += $(call cc-option,-fno-stack-protector)

KBUILD_CFLAGS	+= -g
# $(KBUILD_AFLAGS) sets -g, which causes gcc to pass a suitable -g<format>
# option to the assembler.
KBUILD_AFLAGS	+= -g

# Report stack usage if supported
KBUILD_CFLAGS += $(call cc-option,-fstack-usage)

KBUILD_CFLAGS += $(call cc-option,-Wno-format-nonliteral)

# turn jbsr into jsr for m68k
ifeq ($(ARCH),m68k)
ifeq ($(findstring 3.4,$(shell $(CC) --version)),3.4)
KBUILD_AFLAGS += -Wa,-gstabs,-S
endif
endif

ifneq ($(CONFIG_SYS_TEXT_BASE),)
KBUILD_CPPFLAGS += -DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE)
endif

export CONFIG_SYS_TEXT_BASE

# Use UBOOTINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
UBOOTINCLUDE    :=
ifneq ($(OBJTREE),$(SRCTREE))
UBOOTINCLUDE	+= -I$(OBJTREE)/include
endif
UBOOTINCLUDE	+= -I$(srctree)/include \
		-I$(srctree)/arch/$(ARCH)/include

NOSTDINC_FLAGS += -nostdinc -isystem $(shell $(CC) -print-file-name=include)
CHECKFLAGS     += $(NOSTDINC_FLAGS)

# FIX ME
cpp_flags := $(KBUILD_CPPFLAGS) $(CPPFLAGS) $(UBOOTINCLUDE) $(NOSTDINC_FLAGS)
c_flags := $(KBUILD_CFLAGS) $(cpp_flags)

.PHONY : $(VERSION_FILE) $(TIMESTAMP_FILE)

#########################################################################
# U-Boot objects....order is important (i.e. start must be first)

head-y := $(CPUDIR)/start.o
head-$(CONFIG_4xx) += arch/powerpc/cpu/ppc4xx/resetvec.o
head-$(CONFIG_MPC85xx) += arch/powerpc/cpu/mpc85xx/resetvec.o

HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)

libs-y += lib/
libs-$(HAVE_VENDOR_COMMON_LIB) += board/$(VENDOR)/common/
libs-y += $(CPUDIR)/
ifdef SOC
libs-y += $(CPUDIR)/$(SOC)/
endif
libs-$(CONFIG_OF_EMBED) += dts/
libs-y += arch/$(ARCH)/lib/
libs-y += fs/
libs-y += net/
libs-y += disk/
libs-y += drivers/
libs-y += drivers/dma/
libs-y += drivers/gpio/
libs-y += drivers/i2c/
libs-y += drivers/input/
libs-y += drivers/mmc/
libs-y += drivers/mtd/
libs-$(CONFIG_CMD_NAND) += drivers/mtd/nand/
libs-y += drivers/mtd/onenand/
libs-$(CONFIG_CMD_UBI) += drivers/mtd/ubi/
libs-y += drivers/mtd/spi/
libs-y += drivers/net/
libs-y += drivers/net/phy/
libs-y += drivers/pci/
libs-y += drivers/power/ \
	drivers/power/fuel_gauge/ \
	drivers/power/mfd/ \
	drivers/power/pmic/ \
	drivers/power/battery/
libs-y += drivers/spi/
libs-$(CONFIG_FMAN_ENET) += drivers/net/fm/
libs-$(CONFIG_SYS_FSL_DDR) += drivers/ddr/fsl/
libs-y += drivers/serial/
libs-y += drivers/usb/eth/
libs-y += drivers/usb/gadget/
libs-y += drivers/usb/host/
libs-y += drivers/usb/musb/
libs-y += drivers/usb/musb-new/
libs-y += drivers/usb/phy/
libs-y += drivers/usb/ulpi/
libs-y += common/
libs-y += lib/libfdt/
libs-$(CONFIG_API) += api/
libs-$(CONFIG_HAS_POST) += post/
libs-y += test/

ifneq (,$(filter $(SOC), mx25 mx27 mx5 mx6 mx31 mx35 mxs vf610))
libs-y += arch/$(ARCH)/imx-common/
endif

libs-$(CONFIG_ARM) += arch/arm/cpu/
libs-$(CONFIG_PPC) += arch/powerpc/cpu/

libs-y += board/$(BOARDDIR)/

libs-y := $(sort $(libs-y))

u-boot-dirs	:= $(patsubst %/,%,$(filter %/, $(libs-y))) tools examples

u-boot-alldirs	:= $(sort $(u-boot-dirs) $(patsubst %/,%,$(filter %/, $(libs-))))

libs-y		:= $(patsubst %/, %/built-in.o, $(libs-y))

u-boot-init := $(head-y)
u-boot-main := $(libs-y)


# Add GCC lib
ifdef USE_PRIVATE_LIBGCC
ifeq ("$(USE_PRIVATE_LIBGCC)", "yes")
PLATFORM_LIBGCC = $(OBJTREE)/arch/$(ARCH)/lib/lib.a
else
PLATFORM_LIBGCC = -L $(USE_PRIVATE_LIBGCC) -lgcc
endif
else
PLATFORM_LIBGCC := -L $(shell dirname `$(CC) $(c_flags) -print-libgcc-file-name`) -lgcc
endif
PLATFORM_LIBS += $(PLATFORM_LIBGCC)
export PLATFORM_LIBS

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(TOPDIR)/include/u-boot/u-boot.lds.h \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

#########################################################################
#########################################################################

ifneq ($(CONFIG_BOARD_SIZE_LIMIT),)
BOARD_SIZE_CHECK = \
	@actual=`wc -c $@ | awk '{print $$1}'`; \
	limit=`printf "%d" $(CONFIG_BOARD_SIZE_LIMIT)`; \
	if test $$actual -gt $$limit; then \
		echo "$@ exceeds file size limit:" >&2 ; \
		echo "  limit:  $$limit bytes" >&2 ; \
		echo "  actual: $$actual bytes" >&2 ; \
		echo "  excess: $$((actual - limit)) bytes" >&2; \
		exit 1; \
	fi
else
BOARD_SIZE_CHECK =
endif

# Statically apply RELA-style relocations (currently arm64 only)
ifneq ($(CONFIG_STATIC_RELA),)
# $(1) is u-boot ELF, $(2) is u-boot bin, $(3) is text base
DO_STATIC_RELA = \
	start=$$($(NM) $(1) | grep __rel_dyn_start | cut -f 1 -d ' '); \
	end=$$($(NM) $(1) | grep __rel_dyn_end | cut -f 1 -d ' '); \
	tools/relocate-rela $(2) $(3) $$start $$end
else
DO_STATIC_RELA =
endif

# Always append ALL so that arch config.mk's can add custom ones
ALL-y += u-boot.srec u-boot.bin System.map

ALL-$(CONFIG_NAND_U_BOOT) += u-boot-nand.bin
ALL-$(CONFIG_ONENAND_U_BOOT) += u-boot-onenand.bin
ALL-$(CONFIG_RAMBOOT_PBL) += u-boot.pbl
ALL-$(CONFIG_SPL) += spl/u-boot-spl.bin
ALL-$(CONFIG_SPL_FRAMEWORK) += u-boot.img
ALL-$(CONFIG_TPL) += tpl/u-boot-tpl.bin
ALL-$(CONFIG_OF_SEPARATE) += u-boot-dtb.bin
ifneq ($(CONFIG_SPL_TARGET),)
ALL-$(CONFIG_SPL) += $(CONFIG_SPL_TARGET:"%"=%)
endif
ALL-$(CONFIG_REMAKE_ELF) += u-boot.elf

# enable combined SPL/u-boot/dtb rules for tegra
ifneq ($(CONFIG_TEGRA),)
ifeq ($(CONFIG_SPL),y)
ifeq ($(CONFIG_OF_SEPARATE),y)
ALL-y += u-boot-dtb-tegra.bin
else
ALL-y += u-boot-nodtb-tegra.bin
endif
endif
endif

LDFLAGS_u-boot += -T u-boot.lds $(LDFLAGS_FINAL)
ifneq ($(CONFIG_SYS_TEXT_BASE),)
LDFLAGS_u-boot += -Ttext $(CONFIG_SYS_TEXT_BASE)
endif

all:		$(ALL-y)

PHONY += dtbs
dtbs dts/dt.dtb: checkdtc u-boot
	$(Q)$(MAKE) $(build)=dts dtbs

u-boot-dtb.bin: u-boot.bin dts/dt.dtb
		cat $^ >$@

u-boot.hex:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O ihex $< $@

u-boot.srec:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

u-boot.bin:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@
		$(call DO_STATIC_RELA,$<,$@,$(CONFIG_SYS_TEXT_BASE))
		$(BOARD_SIZE_CHECK)

u-boot.ldr:	u-boot
		$(CREATE_LDR_ENV)
		$(LDR) -T $(CONFIG_BFIN_CPU) -c $@ $< $(LDR_FLAGS)
		$(BOARD_SIZE_CHECK)

u-boot.ldr.hex:	u-boot.ldr
		$(OBJCOPY) ${OBJCFLAGS} -O ihex $< $@ -I binary

u-boot.ldr.srec:	u-boot.ldr
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@ -I binary

#
# U-Boot entry point, needed for booting of full-blown U-Boot
# from the SPL U-Boot version.
#
ifndef CONFIG_SYS_UBOOT_START
CONFIG_SYS_UBOOT_START := 0
endif

u-boot.img:	u-boot.bin
		tools/mkimage -A $(ARCH) -T firmware -C none \
		-O u-boot -a $(CONFIG_SYS_TEXT_BASE) \
		-e $(CONFIG_SYS_UBOOT_START) \
		-n $(shell sed -n -e 's/.*U_BOOT_VERSION//p' $(VERSION_FILE) | \
			sed -e 's/"[	 ]*$$/ for $(BOARD) board"/') \
		-d $< $@

u-boot.imx: u-boot.bin
		$(MAKE) $(build)=arch/arm/imx-common $(objtree)/u-boot.imx

u-boot.kwb:       u-boot.bin
		tools/mkimage -n $(CONFIG_SYS_KWD_CONFIG) -T kwbimage \
		-a $(CONFIG_SYS_TEXT_BASE) -e $(CONFIG_SYS_TEXT_BASE) -d $< $@

u-boot.pbl:	u-boot.bin
		tools/mkimage -n $(CONFIG_SYS_FSL_PBL_RCW) \
		-R $(CONFIG_SYS_FSL_PBL_PBI) -T pblimage \
		-d $< $@

u-boot.sha1:	u-boot.bin
		tools/ubsha1 u-boot.bin

u-boot.dis:	u-boot
		$(OBJDUMP) -d $< > $@

# $@ is output, $(1) and $(2) are inputs, $(3) is padded intermediate,
# $(4) is pad-to
SPL_PAD_APPEND = \
		$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(4) -I binary -O binary \
		$(1) $(3); \
		cat $(3) $(2) > $@; \
		rm $(3)

ifdef CONFIG_TPL
SPL_PAYLOAD := tpl/u-boot-with-tpl.bin
else
SPL_PAYLOAD := u-boot.bin
endif

u-boot-with-spl.bin: spl/u-boot-spl.bin $(SPL_PAYLOAD)
		$(call SPL_PAD_APPEND,$<,$(SPL_PAYLOAD),spl/u-boot-spl-pad.bin,$(CONFIG_SPL_PAD_TO))

tpl/u-boot-with-tpl.bin: tpl/u-boot-tpl.bin u-boot.bin
		$(call SPL_PAD_APPEND,$<,u-boot.bin,tpl/u-boot-tpl-pad.bin,$(CONFIG_TPL_PAD_TO))

u-boot-with-spl.imx: spl/u-boot-spl.bin u-boot.bin
		$(MAKE) $(build)=arch/arm/imx-common \
			$(OBJTREE)/u-boot-with-spl.imx

u-boot-with-nand-spl.imx: spl/u-boot-spl.bin u-boot.bin
		$(MAKE) $(build)=arch/arm/imx-common \
			$(OBJTREE)/u-boot-with-nand-spl.imx

u-boot.ubl:       u-boot-with-spl.bin
		tools/mkimage -n $(UBL_CONFIG) -T ublimage \
		-e $(CONFIG_SYS_TEXT_BASE) -d $< u-boot.ubl

u-boot.ais:       spl/u-boot-spl.bin u-boot.img
		tools/mkimage -s -n $(if $(CONFIG_AIS_CONFIG_FILE),$(srctree)/$(CONFIG_AIS_CONFIG_FILE:"%"=%),"/dev/null") \
			-T aisimage \
			-e $(CONFIG_SPL_TEXT_BASE) \
			-d spl/u-boot-spl.bin \
			spl/u-boot-spl.ais
		$(OBJCOPY) ${OBJCFLAGS} -I binary \
			--pad-to=$(CONFIG_SPL_MAX_SIZE) -O binary \
			spl/u-boot-spl.ais spl/u-boot-spl-pad.ais
		cat spl/u-boot-spl-pad.ais u-boot.img > u-boot.ais


u-boot.sb:       u-boot.bin spl/u-boot-spl.bin
		$(MAKE) $(build)=$(CPUDIR)/$(SOC)/ $(OBJTREE)/u-boot.sb

# On x600 (SPEAr600) U-Boot is appended to U-Boot SPL.
# Both images are created using mkimage (crc etc), so that the ROM
# bootloader can check its integrity. Padding needs to be done to the
# SPL image (with mkimage header) and not the binary. Otherwise the resulting image
# which is loaded/copied by the ROM bootloader to SRAM doesn't fit.
# The resulting image containing both U-Boot images is called u-boot.spr
u-boot.spr:	u-boot.img spl/u-boot-spl.bin
		tools/mkimage -A $(ARCH) -T firmware -C none \
		-a $(CONFIG_SPL_TEXT_BASE) -e $(CONFIG_SPL_TEXT_BASE) -n XLOADER \
		-d spl/u-boot-spl.bin $@
		$(OBJCOPY) -I binary -O binary \
			--pad-to=$(CONFIG_SPL_PAD_TO) --gap-fill=0xff $@
		cat u-boot.img >> $@

ifneq ($(CONFIG_TEGRA),)
u-boot-nodtb-tegra.bin: spl/u-boot-spl.bin u-boot.bin
		$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(CONFIG_SYS_TEXT_BASE) -O binary spl/u-boot-spl spl/u-boot-spl-pad.bin
		cat spl/u-boot-spl-pad.bin u-boot.bin > $@
		rm spl/u-boot-spl-pad.bin

ifeq ($(CONFIG_OF_SEPARATE),y)
u-boot-dtb-tegra.bin: u-boot-nodtb-tegra.bin dts/dt.dtb
		cat $^ > $@
endif
endif

u-boot-img.bin: spl/u-boot-spl.bin u-boot.img
		cat spl/u-boot-spl.bin u-boot.img > $@

# PPC4xx needs the SPL at the end of the image, since the reset vector
# is located at 0xfffffffc. So we can't use the "u-boot-img.bin" target
# and need to introduce a new build target with the full blown U-Boot
# at the start padded up to the start of the SPL image. And then concat
# the SPL image to the end.
u-boot-img-spl-at-end.bin: spl/u-boot-spl.bin u-boot.img
		$(OBJCOPY) -I binary -O binary --pad-to=$(CONFIG_UBOOT_PAD_TO) \
			 --gap-fill=0xff u-boot.img $@
		cat spl/u-boot-spl.bin >> $@

# Create a new ELF from a raw binary file.  This is useful for arm64
# where static relocation needs to be performed on the raw binary,
# but certain simulators only accept an ELF file (but don't do the
# relocation).
# FIXME refactor dts/Makefile to share target/arch detection
u-boot.elf: u-boot.bin
	@$(OBJCOPY)  -B aarch64 -I binary -O elf64-littleaarch64 \
		$< u-boot-elf.o
	@$(LD) u-boot-elf.o -o $@ \
		--defsym=_start=$(CONFIG_SYS_TEXT_BASE) \
		-Ttext=$(CONFIG_SYS_TEXT_BASE)

ifeq ($(CONFIG_SANDBOX),y)
GEN_UBOOT = \
		$(CC) $(SYMS) -T u-boot.lds \
			-Wl,--start-group $(u-boot-main) -Wl,--end-group \
			$(PLATFORM_LIBS) -Wl,-Map -Wl,u-boot.map -o u-boot
else
GEN_UBOOT = \
		$(LD) $(LDFLAGS) $(LDFLAGS_$(@F)) \
			$(u-boot-init) \
			--start-group $(u-boot-main) --end-group $(PLATFORM_LIBS) \
			-Map u-boot.map -o u-boot
endif

u-boot:	$(u-boot-init) $(u-boot-main) u-boot.lds
		$(GEN_UBOOT)
ifeq ($(CONFIG_KALLSYMS),y)
		smap=`$(call SYSTEM_MAP,u-boot) | \
			awk '$$2 ~ /[tTwW]/ {printf $$1 $$3 "\\\\000"}'` ; \
		$(CC) $(c_flags) -DSYSTEM_MAP="\"$${smap}\"" \
			-c $(srctree)/common/system_map.c -o common/system_map.o
		$(GEN_UBOOT) common/system_map.o
endif

# The actual objects are generated when descending, 
# make sure no implicit rule kicks in
$(sort $(u-boot-init) $(u-boot-main)): $(u-boot-dirs) ;

# Handle descending into subdirectories listed in $(vmlinux-dirs)
# Preset locale variables to speed up the build process. Limit locale
# tweaks to this spot to avoid wrong language settings when running
# make menuconfig etc.
# Error messages still appears in the original language

PHONY += $(u-boot-dirs)
$(u-boot-dirs): depend prepare scripts
	$(Q)$(MAKE) $(build)=$@

tools: $(TIMESTAMP_FILE) $(VERSION_FILE)
# The "tools" are needed early
$(filter-out tools, $(u-boot-dirs)): tools
# The "examples" conditionally depend on U-Boot (say, when USE_PRIVATE_LIBGCC
# is "yes"), so compile examples after U-Boot is compiled.
examples: $(filter-out examples, $(u-boot-dirs))

# Things we need to do before we recursively start building the kernel
# or the modules are listed in "prepare".
# A multi level approach is used. prepareN is processed before prepareN-1.
# archprepare is used in arch Makefiles and when processed asm symlink,
# version.h and scripts_basic is processed / created.

# Listed in dependency order
PHONY += prepare archprepare prepare0 prepare1 prepare2 prepare3

# prepare3 is used to check if we are building in a separate output directory,
# and if so do:
# 1) Check that make has not been executed in the kernel src $(srctree)
prepare3:
ifneq ($(KBUILD_SRC),)
	@$(kecho) '  Using $(srctree) as source for u-boot'
	$(Q)if [ -f $(srctree)/include/config.mk ]; then \
		echo >&2 "  $(srctree) is not clean, please run 'make mrproper'"; \
		echo >&2 "  in the '$(srctree)' directory.";\
		/bin/false; \
	fi;
endif

# prepare2 creates a makefile if using a separate output directory
prepare2: prepare3 outputmakefile

prepare1: prepare2
	@:

archprepare: prepare1 scripts_basic

prepare0: archprepare FORCE
	@:

# All the preparing..
prepare: prepare0

#
# Auto-generate the autoconf.mk file (which is included by all makefiles)
#
# This target actually generates 2 files; autoconf.mk and autoconf.mk.dep.
# the dep file is only include in this top level makefile to determine when
# to regenerate the autoconf.mk file.

quiet_cmd_autoconf_dep = GEN     $@
      cmd_autoconf_dep = $(CC) -x c -DDO_DEPS_ONLY -M $(c_flags) \
	-MQ include/autoconf.mk $(srctree)/include/common.h > $@ || rm $@

include/autoconf.mk.dep: include/config.h include/common.h
	$(call cmd,autoconf_dep)

quiet_cmd_autoconf = GEN     $@
      cmd_autoconf = \
	$(CPP) $(c_flags) -DDO_DEPS_ONLY -dM $(srctree)/include/common.h > $@.tmp && \
	sed -n -f $(srctree)/tools/scripts/define2mk.sed $@.tmp > $@; \
	rm $@.tmp

include/autoconf.mk: include/config.h
	$(call cmd,autoconf)

u-boot.lds: $(LDSCRIPT) depend
		$(CPP) $(cpp_flags) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$< >$@

nand_spl:	$(TIMESTAMP_FILE) $(VERSION_FILE) depend prepare
		$(MAKE) $(build)=nand_spl/board/$(BOARDDIR) all

u-boot-nand.bin:	nand_spl u-boot.bin
		cat nand_spl/u-boot-spl-16k.bin u-boot.bin > u-boot-nand.bin

spl/u-boot-spl.bin: tools depend prepare 
		$(MAKE) obj=spl -f $(srctree)/spl/Makefile all

tpl/u-boot-tpl.bin: tools depend prepare
		$(MAKE) obj=tpl -f $(srctree)/spl/Makefile all CONFIG_TPL_BUILD=y

# Explicitly make _depend in subdirs containing multiple targets to prevent
# parallel sub-makes creating .depend files simultaneously.
depend dep:	$(TIMESTAMP_FILE) $(VERSION_FILE) \
		include/generated/generic-asm-offsets.h \
		include/generated/asm-offsets.h

TAG_SUBDIRS := $(u-boot-dirs) include

FIND := find
FINDFLAGS := -L

PHONY += checkstack

checkstack:
	$(OBJDUMP) -d u-boot $$(find . -name u-boot-spl) | \
	$(PERL) $(src)/scripts/checkstack.pl $(ARCH)

tags ctags:
		ctags -w -o ctags `$(FIND) $(FINDFLAGS) $(TAG_SUBDIRS) \
						-name '*.[chS]' -print`

etags:
		etags -a -o $(obj)etags `$(FIND) $(FINDFLAGS) $(TAG_SUBDIRS) \
						-name '*.[chS]' -print`
cscope:
		$(FIND) $(FINDFLAGS) $(TAG_SUBDIRS) -name '*.[chS]' -print > \
						cscope.files
		cscope -b -q -k

SYSTEM_MAP = \
		$(NM) $1 | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		LC_ALL=C sort
System.map:	u-boot
		@$(call SYSTEM_MAP,$<) > $@

checkthumb:
	@if test $(call cc-version) -lt 0404; then \
		echo -n '*** Your GCC does not produce working '; \
		echo 'binaries in THUMB mode.'; \
		echo '*** Your board is configured for THUMB mode.'; \
		false; \
	fi

# GCC 3.x is reported to have problems generating the type of relocation
# that U-Boot wants.
# See http://lists.denx.de/pipermail/u-boot/2012-September/135156.html
checkgcc4:
	@if test $(call cc-version) -lt 0400; then \
		echo -n '*** Your GCC is too old, please upgrade to GCC 4.x or newer'; \
		false; \
	fi

checkdtc:
	@if test $(call dtc-version) -lt 0104; then \
		echo '*** Your dtc is too old, please upgrade to dtc 1.4 or newer'; \
		false; \
	fi

quiet_cmd_offsets = GEN     $@
      cmd_offsets = $(srctree)/tools/scripts/make-asm-offsets $< $@

include/generated/generic-asm-offsets.h: lib/asm-offsets.s
	$(call cmd,offsets)

quiet_cmd_asm-offsets.s = CC      $@
      cmd_asm-offsets.s = mkdir -p lib; \
		$(CC) -DDO_DEPS_ONLY \
		$(c_flags) $(CFLAGS_$(BCURDIR)/$(@F)) $(CFLAGS_$(BCURDIR)) \
		-o $@ $< -c -S

lib/asm-offsets.s: $(srctree)/lib/asm-offsets.c include/config.h
	$(call cmd,asm-offsets.s)

include/generated/asm-offsets.h: $(CPUDIR)/$(SOC)/asm-offsets.s
	$(call cmd,offsets)

quiet_cmd_soc_asm-offsets.s = CC      $@
      cmd_soc_asm-offsets.s = mkdir -p $(CPUDIR)/$(SOC); \
	if [ -f $(srctree)/$(CPUDIR)/$(SOC)/asm-offsets.c ];then \
		$(CC) -DDO_DEPS_ONLY \
		$(c_flags) $(CFLAGS_$(BCURDIR)/$(@F)) $(CFLAGS_$(BCURDIR)) \
			-o $@ $(srctree)/$(CPUDIR)/$(SOC)/asm-offsets.c -c -S; \
	else \
		touch $@; \
	fi

$(CPUDIR)/$(SOC)/asm-offsets.s:	include/config.h
	$(call cmd,soc_asm-offsets.s)

#########################################################################

# ARM relocations should all be R_ARM_RELATIVE (32-bit) or
# R_AARCH64_RELATIVE (64-bit).
checkarmreloc: u-boot
	@RELOC="`$(CROSS_COMPILE)readelf -r -W $< | cut -d ' ' -f 4 | \
		grep R_A | sort -u`"; \
	if test "$$RELOC" != "R_ARM_RELATIVE" -a \
		 "$$RELOC" != "R_AARCH64_RELATIVE"; then \
		echo "$< contains unexpected relocations: $$RELOC"; \
		false; \
	fi

$(VERSION_FILE):
		@mkdir -p $(dir $(VERSION_FILE))
		@( localvers='$(shell $(TOPDIR)/scripts/setlocalversion $(TOPDIR))' ; \
		   printf '#define PLAIN_VERSION "%s%s"\n' \
			"$(U_BOOT_VERSION)" "$${localvers}" ; \
		   printf '#define U_BOOT_VERSION "U-Boot %s%s"\n' \
			"$(U_BOOT_VERSION)" "$${localvers}" ; \
		) > $@.tmp
		@( printf '#define CC_VERSION_STRING "%s"\n' \
		 '$(shell $(CC) --version | head -n 1)' )>>  $@.tmp
		@( printf '#define LD_VERSION_STRING "%s"\n' \
		 '$(shell $(LD) -v | head -n 1)' )>>  $@.tmp
		@cmp -s $@ $@.tmp && rm -f $@.tmp || mv -f $@.tmp $@

$(TIMESTAMP_FILE):
		@mkdir -p $(dir $(TIMESTAMP_FILE))
		@LC_ALL=C date +'#define U_BOOT_DATE "%b %d %C%y"' > $@.tmp
		@LC_ALL=C date +'#define U_BOOT_TIME "%T"' >> $@.tmp
		@cmp -s $@ $@.tmp && rm -f $@.tmp || mv -f $@.tmp $@

env: depend scripts_basic
	$(Q)$(MAKE) $(build)=tools/$@

tools-all: HOST_TOOLS_ALL=y
tools-all: env tools ;

.PHONY : CHANGELOG
CHANGELOG:
	git log --no-merges U-Boot-1_1_5.. | \
	unexpand -a | sed -e 's/\s\s*$$//' > $@

include/license.h: tools/bin2header COPYING
	cat COPYING | gzip -9 -c | ./tools/bin2header license_gzip > include/license.h
#########################################################################

###
# Cleaning is done on three levels.
# make clean     Delete most generated files
#                Leave enough to build external modules
# make mrproper  Delete the current configuration, and all generated files
# make distclean Remove editor backup files, patch leftover files and the like

# Directories & files removed with 'make clean'
CLEAN_DIRS  += $(MODVERDIR)
CLEAN_FILES += u-boot.lds include/bmp_logo.h include/bmp_logo_data.h \
	       board/*/config.tmp board/*/*/config.tmp \
	       include/autoconf.mk* include/spl-autoconf.mk \
	       include/tpl-autoconf.mk

# Directories & files removed with 'make clobber'
CLOBBER_DIRS  += $(patsubst %,spl/%, $(filter-out Makefile, \
		 $(shell ls -1 spl 2>/dev/null))) \
		 tpl
CLOBBER_FILES += u-boot* MLO MLO* SPL System.map nand_spl/u-boot*

# Directories & files removed with 'make mrproper'
MRPROPER_DIRS  += include/config include/generated
MRPROPER_FILES += .config .config.old \
		  tags TAGS cscope* GPATH GTAGS GRTAGS GSYMS \
		  include/config.h include/config.mk

# clean - Delete most, but leave enough to build external modules
#
clean: rm-dirs  := $(CLEAN_DIRS)
clean: rm-files := $(CLEAN_FILES)

clean-dirs	:= $(foreach f,$(u-boot-alldirs),$(if $(wildcard $f/Makefile),$f))

clean-dirs      := $(addprefix _clean_, $(clean-dirs) doc/DocBook)

PHONY += $(clean-dirs) clean archclean
$(clean-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _clean_%,%,$@)

# TODO: Do not use *.cfgtmp
clean: $(clean-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@find $(if $(KBUILD_EXTMOD), $(KBUILD_EXTMOD), .) $(RCS_FIND_IGNORE) \
		\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '*.ko.*' -o -name '*.su' -o -name '*.cfgtmp' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.symtypes' -o -name 'modules.order' \
		-o -name modules.builtin -o -name '.tmp_*.o.*' \
		-o -name '*.gcno' \) -type f -print | xargs rm -f
	@find $(if $(KBUILD_EXTMOD), $(KBUILD_EXTMOD), .) $(RCS_FIND_IGNORE) \
		-path './nand_spl/*' -type l -print | xargs rm -f

# clobber
#
clobber: rm-dirs  := $(CLOBBER_DIRS)
clobber: rm-files := $(CLOBBER_FILES)

PHONY += clobber

clobber: clean
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)

# mrproper - Delete all generated files, including .config
#
mrproper: rm-dirs  := $(wildcard $(MRPROPER_DIRS))
mrproper: rm-files := $(wildcard $(MRPROPER_FILES))
mrproper-dirs      := $(addprefix _mrproper_,scripts)

PHONY += $(mrproper-dirs) mrproper archmrproper
$(mrproper-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _mrproper_%,%,$@)

mrproper: clobber $(mrproper-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@rm -f arch/*/include/asm/arch arch/*/include/asm/proc

# distclean
#
PHONY += distclean

distclean: mrproper
	@find $(srctree) $(RCS_FIND_IGNORE) \
		\( -name '*.orig' -o -name '*.rej' -o -name '*~' \
		-o -name '*.bak' -o -name '#*#' -o -name '.*.orig' \
		-o -name '.*.rej' \
		-o -name '*%' -o -name '.*.cmd' -o -name 'core' \) \
		-type f -print | xargs rm -f

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `LC_ALL=C date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F


# Documentation targets
# ---------------------------------------------------------------------------
%docs: scripts_basic FORCE
	$(Q)$(MAKE) $(build)=scripts build_docproc
	$(Q)$(MAKE) $(build)=doc/DocBook $@

# Dummies...
PHONY += prepare scripts
prepare: ;
scripts: ;

endif #ifeq ($(config-targets),1)
endif #ifeq ($(mixed-targets),1)

quiet_cmd_rmdirs = $(if $(wildcard $(rm-dirs)),CLEAN   $(wildcard $(rm-dirs)))
      cmd_rmdirs = rm -rf $(rm-dirs)

quiet_cmd_rmfiles = $(if $(wildcard $(rm-files)),CLEAN   $(wildcard $(rm-files)))
      cmd_rmfiles = rm -f $(rm-files)

# Shorthand for $(Q)$(MAKE) -f scripts/Makefile.clean obj=dir
# Usage:
# $(Q)$(MAKE) $(clean)=dir
clean := -f $(if $(KBUILD_SRC),$(srctree)/)scripts/Makefile.clean obj

endif	# skip-makefile

PHONY += FORCE
FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
