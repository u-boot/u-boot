# SPDX-License-Identifier: GPL-2.0+

VERSION = 2025
PATCHLEVEL = 10
SUBLEVEL =
EXTRAVERSION = -rc3
NAME =

# *DOCUMENTATION*
# To see a list of typical targets execute "make help"
# More info can be located in ./README
# Comments in this file are targeted only to the developer, do not
# expect to learn how to build the kernel reading this file.

# That's our default target when none is given on the command line
PHONY := _all
_all:

# Determine target architecture for the sandbox
include include/host_arch.h
ifeq ("", "$(CROSS_COMPILE)")
  MK_ARCH="${shell uname -m}"
else
  MK_ARCH="${shell echo $(CROSS_COMPILE) | sed -n 's/^\(.*ccache\)\{0,1\}[[:space:]]*\([^\/]*\/\)*\([^-]*\)-[^[:space:]]*/\3/p'}"
endif
unexport HOST_ARCH
ifeq ("x86_64", $(MK_ARCH))
  export HOST_ARCH=$(HOST_ARCH_X86_64)
else ifneq (,$(findstring $(MK_ARCH), "i386" "i486" "i586" "i686"))
  export HOST_ARCH=$(HOST_ARCH_X86)
else ifneq (,$(findstring $(MK_ARCH), "aarch64" "armv8l"))
  export HOST_ARCH=$(HOST_ARCH_AARCH64)
else ifneq (,$(findstring $(MK_ARCH), "arm" "armv5tel" "armv6l" "armv7" "armv7a" "armv7l"))
  export HOST_ARCH=$(HOST_ARCH_ARM)
else ifeq ("riscv32", $(MK_ARCH))
  export HOST_ARCH=$(HOST_ARCH_RISCV32)
else ifeq ("riscv64", $(MK_ARCH))
  export HOST_ARCH=$(HOST_ARCH_RISCV64)
endif
undefine MK_ARCH

# We are using a recursive build, so we need to do a little thinking
# to get the ordering right.
#
# Most importantly: sub-Makefiles should only ever modify files in
# their own directory. If in some directory we have a dependency on
# a file in another dir (which doesn't happen often, but it's often
# unavoidable when linking the built-in.a targets which finally
# turn into vmlinux), we will call a sub make in that other dir, and
# after that we are sure that everything which is in that other dir
# is now up to date.
#
# The only cases where we need to modify files which have global
# effects are thus separated out and done before the recursive
# descending is started. They are now explicitly listed as the
# prepare rule.

ifneq ($(sub_make_done),1)

# Do not use make's built-in rules and variables
# (this increases performance and avoids hard-to-debug behaviour)
MAKEFLAGS += -rR

# Avoid funny character set dependencies
unexport LC_ALL
LC_COLLATE=C
LC_NUMERIC=C
export LC_COLLATE LC_NUMERIC

# Avoid interference with shell env settings
unexport GREP_OPTIONS

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
#
# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands
ifneq ($(filter 4.%,$(MAKE_VERSION)),)	# make-4
ifneq ($(filter %s ,$(firstword x$(MAKEFLAGS))),)
  quiet=silent_
endif
else					# make-3.8x
ifneq ($(filter s% -s%,$(MAKEFLAGS)),)
  quiet=silent_
endif
endif

export quiet Q KBUILD_VERBOSE

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

# KBUILD_SRC is not intended to be used by the regular user (for now),
# it is set on invocation of make with KBUILD_OUTPUT or O= specified.

# OK, Make called in directory where kernel src resides
# Do we want to locate output files in a separate directory?
ifeq ("$(origin O)", "command line")
  KBUILD_OUTPUT := $(O)
endif

ifneq ($(words $(subst :, ,$(CURDIR))), 1)
  $(error main directory cannot contain spaces nor colons)
endif

ifneq ($(KBUILD_OUTPUT),)
# check that the output directory actually exists
saved-output := $(KBUILD_OUTPUT)
KBUILD_OUTPUT := $(shell mkdir -p $(KBUILD_OUTPUT) && cd $(KBUILD_OUTPUT) \
								&& pwd)
$(if $(KBUILD_OUTPUT),, \
     $(error failed to create output directory "$(saved-output)"))

# Look for make include files relative to root of kernel src
#
# This does not become effective immediately because MAKEFLAGS is re-parsed
# once after the Makefile is read.  It is OK since we are going to invoke
# 'sub-make' below.
MAKEFLAGS += --include-dir=$(CURDIR)

need-sub-make := 1
else

# Do not print "Entering directory ..." at all for in-tree build.
MAKEFLAGS += --no-print-directory

endif # ifneq ($(KBUILD_OUTPUT),)

ifneq ($(filter 3.%,$(MAKE_VERSION)),)
# 'MAKEFLAGS += -rR' does not immediately become effective for GNU Make 3.x
# We need to invoke sub-make to avoid implicit rules in the top Makefile.
need-sub-make := 1
# Cancel implicit rules for this Makefile.
$(lastword $(MAKEFILE_LIST)): ;
endif

export sub_make_done := 1

ifeq ($(need-sub-make),1)

PHONY += $(MAKECMDGOALS) sub-make

$(filter-out _all sub-make $(CURDIR)/Makefile, $(MAKECMDGOALS)) _all: sub-make
	@:

# Invoke a second make in the output directory, passing relevant variables
sub-make:
	$(Q)$(MAKE) \
	$(if $(KBUILD_OUTPUT),-C $(KBUILD_OUTPUT) KBUILD_SRC=$(CURDIR)) \
	-f $(CURDIR)/Makefile $(filter-out _all sub-make,$(MAKECMDGOALS))

endif # need-sub-make
endif # sub_make_done

# We process the rest of the Makefile if this is the final invocation of make
ifeq ($(need-sub-make),)

# Do not print "Entering directory ...",
# but we want to display it when entering to the output directory
# so that IDEs/editors are able to understand relative filenames.
MAKEFLAGS += --no-print-directory

# Call a source code checker (by default, "sparse") as part of the
# C compilation.
#
# Use 'make C=1' to enable checking of only re-compiled files.
# Use 'make C=2' to enable checking of *all* source files, regardless
# of whether they are re-compiled or not.
#
# See the file "Documentation/dev-tools/sparse.rst" for more details,
# including where to get the "sparse" utility.

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
  $(warning ================= WARNING ================)
  $(warning 'SUBDIRS' will be removed after Linux 5.3)
  $(warning Please use 'M=' or 'KBUILD_EXTMOD' instead)
  $(warning ==========================================)
  KBUILD_EXTMOD ?= $(SUBDIRS)
endif

ifeq ("$(origin M)", "command line")
  KBUILD_EXTMOD := $(M)
endif

ifeq ($(KBUILD_SRC),)
        # building in the source tree
        srctree := .
else
        ifeq ($(KBUILD_SRC)/,$(dir $(CURDIR)))
                # building in a subdirectory of the source tree
                srctree := ..
        else
                srctree := $(KBUILD_SRC)
        endif
endif

export KBUILD_CHECKSRC KBUILD_EXTMOD KBUILD_SRC

objtree		:= .
src		:= $(srctree)
obj		:= $(objtree)

VPATH		:= $(srctree)

export srctree objtree VPATH

# To make sure we do not include .config for any of the *config targets
# catch them early, and hand them over to scripts/kconfig/Makefile
# It is allowed to specify more targets when calling make, including
# mixing *config targets and build targets.
# For example 'make oldconfig all'.
# Detect when mixed targets is specified, and make a second invocation
# of make so .config is not included in this case either (for *config).

version_h := include/generated/version_autogenerated.h
timestamp_h := include/generated/timestamp_autogenerated.h
defaultenv_h := include/generated/defaultenv_autogenerated.h
dt_h := include/generated/dt.h
env_h := include/generated/environment.h

clean-targets := %clean mrproper cleandocs
no-dot-config-targets := $(clean-targets) \
			 clobber distclean \
			 help %docs check% coccicheck \
			 ubootversion backup tests check pcheck qcheck tcheck \
			 pylint pylint_err _pip pip pip_test pip_release
no-sync-config-targets := $(no-dot-config-targets) install %install \
			   kernelrelease

config-targets  := 0
mixed-targets   := 0
dot-config      := 1
may-sync-config := 1

ifneq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-dot-config-targets), $(MAKECMDGOALS)),)
		dot-config := 0
	endif
endif

ifneq ($(filter $(no-sync-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-sync-config-targets), $(MAKECMDGOALS)),)
		may-sync-config := 0
	endif
endif

ifneq ($(KBUILD_EXTMOD),)
	may-sync-config := 0
endif

ifeq ($(KBUILD_EXTMOD),)
        ifneq ($(filter config %config,$(MAKECMDGOALS)),)
                config-targets := 1
                ifneq ($(words $(MAKECMDGOALS)),1)
                        mixed-targets := 1
                endif
        endif
endif

# For "make -j clean all", "make -j mrproper defconfig all", etc.
ifneq ($(filter $(clean-targets),$(MAKECMDGOALS)),)
        ifneq ($(filter-out $(clean-targets),$(MAKECMDGOALS)),)
                mixed-targets := 1
        endif
endif

# install and modules_install need also be processed one by one
ifneq ($(filter install,$(MAKECMDGOALS)),)
        ifneq ($(filter modules_install,$(MAKECMDGOALS)),)
	        mixed-targets := 1
        endif
endif

ifeq ($(mixed-targets),1)
# ===========================================================================
# We're called with mixed targets (*config and build targets).
# Handle them one by one.

PHONY += $(MAKECMDGOALS) __build_one_by_one

$(filter-out __build_one_by_one, $(MAKECMDGOALS)): __build_one_by_one
	@:

__build_one_by_one:
	$(Q)set -e; \
	for i in $(MAKECMDGOALS); do \
		$(MAKE) -f $(srctree)/Makefile $$i; \
	done

else

include scripts/Kbuild.include

# Read UBOOTRELEASE from include/config/uboot.release (if it exists)
UBOOTRELEASE = $(shell cat include/config/uboot.release 2> /dev/null)
UBOOTVERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(EXTRAVERSION)
export VERSION PATCHLEVEL SUBLEVEL UBOOTRELEASE UBOOTVERSION

# Modified for U-Boot
-include scripts/subarch.include

# Cross compiling and selecting different set of gcc/bin-utils
# ---------------------------------------------------------------------------
#
# When performing cross compilation for other architectures ARCH shall be set
# to the target architecture. (See arch/* for the possibilities).
# ARCH can be set during invocation of make:
# make ARCH=ia64
# Another way is to have ARCH set in the environment.
# The default ARCH is the host where make is executed.

# CROSS_COMPILE specify the prefix used for all executables used
# during compilation. Only gcc and related bin-utils executables
# are prefixed with $(CROSS_COMPILE).
# CROSS_COMPILE can be set on the command line
# make CROSS_COMPILE=ia64-linux-
# Alternatively CROSS_COMPILE can be set in the environment.
# Default value for CROSS_COMPILE is not to prefix executables
# Note: Some architectures assign CROSS_COMPILE in their arch/*/Makefile
ARCH		?= $(SUBARCH)

# Architecture as present in compile.h
UTS_MACHINE 	:= $(ARCH)
SRCARCH 	:= $(ARCH)

# Additional ARCH settings for x86
ifeq ($(ARCH),i386)
        SRCARCH := x86
endif
ifeq ($(ARCH),x86_64)
        SRCARCH := x86
endif

# Additional ARCH settings for sparc
ifeq ($(ARCH),sparc32)
       SRCARCH := sparc
endif
ifeq ($(ARCH),sparc64)
       SRCARCH := sparc
endif

# Additional ARCH settings for sh
ifeq ($(ARCH),sh64)
       SRCARCH := sh
endif

KCONFIG_CONFIG	?= .config
export KCONFIG_CONFIG

# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

HOST_LFS_CFLAGS := $(shell getconf LFS_CFLAGS 2>/dev/null)
HOST_LFS_LDFLAGS := $(shell getconf LFS_LDFLAGS 2>/dev/null)
HOST_LFS_LIBS := $(shell getconf LFS_LIBS 2>/dev/null)

HOSTCC       = cc
HOSTCXX      = c++
KBUILD_HOSTCFLAGS   := -Wall -Wstrict-prototypes -O2 \
		-fomit-frame-pointer -std=gnu11 $(HOST_LFS_CFLAGS) \
		$(HOSTCFLAGS) #-Wmissing-prototypes Enable it and fix warnings
KBUILD_HOSTCXXFLAGS := -O2 $(HOST_LFS_CFLAGS) $(HOSTCXXFLAGS)
KBUILD_HOSTLDFLAGS  := $(HOST_LFS_LDFLAGS) $(HOSTLDFLAGS)
KBUILD_HOSTLDLIBS   := $(HOST_LFS_LIBS) $(HOSTLDLIBS)

# Check ths size of a binary:
# Args:
#   $1: File to check
#   #2: Size limit in bytes (decimal or 0xhex)
define size_check
	actual=$$( wc -c $1 | awk '{print $$1}'); \
	limit=$$( printf "%d" $2 ); \
	if test $$actual -gt $$limit; then \
		echo "$1 exceeds file size limit:" >&2; \
		echo "  limit:  $$(printf %#x $$limit) bytes" >&2; \
		echo "  actual: $$(printf %#x $$actual) bytes" >&2; \
		echo "  excess: $$(printf %#x $$((actual - limit))) bytes" >&2;\
		exit 1; \
	fi
endef
export size_check

export KBUILD_MODULES KBUILD_BUILTIN
export KBUILD_CHECKSRC KBUILD_SRC KBUILD_EXTMOD

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
LDR		= $(CROSS_COMPILE)ldr
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
READELF		= $(CROSS_COMPILE)readelf
LEX		= flex
YACC		= bison
AWK		= awk
INSTALLKERNEL  := installkernel
DEPMOD		= /sbin/depmod
PERL		= perl
PYTHON		= python
PYTHON2		= python2
PYTHON3		= python3

# The devicetree compiler and pylibfdt are automatically built unless DTC is
# provided. If DTC is provided, it is assumed the pylibfdt is available too.
DTC_INTREE	:= $(objtree)/scripts/dtc/dtc
DTC		?= $(DTC_INTREE)
DTC_MIN_VERSION	:= 010406

CHECK		= sparse

CHECKFLAGS     := -D__linux__ -Dlinux -D__STDC__ -Dunix -D__unix__ \
		  -Wbitwise -Wno-return-void -Wno-unknown-attribute -D__CHECK_ENDIAN__ $(CF)
NOSTDINC_FLAGS :=
CFLAGS_MODULE   =
AFLAGS_MODULE   =
LDFLAGS_MODULE  =
CFLAGS_KERNEL	=
AFLAGS_KERNEL	=
LDFLAGS_vmlinux =

# Use USERINCLUDE when you must reference the UAPI directories only.
USERINCLUDE    := \
		-I$(srctree)/arch/$(SRCARCH)/include/uapi \
		-I$(objtree)/arch/$(SRCARCH)/include/generated/uapi \
		-I$(srctree)/include/uapi \
		-I$(objtree)/include/generated/uapi \
                -include $(srctree)/include/linux/kconfig.h

# Use UBOOTINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
UBOOTINCLUDE    := \
	-Iinclude \
	$(if $(KBUILD_SRC), -I$(srctree)/include) \
	$(if $(CONFIG_$(XPL_)MBEDTLS_LIB), \
		"-DMBEDTLS_CONFIG_FILE=\"mbedtls_def_config.h\"" \
		-I$(srctree)/lib/mbedtls \
		-I$(srctree)/lib/mbedtls/port \
		-I$(srctree)/lib/mbedtls/external/mbedtls \
		-I$(srctree)/lib/mbedtls/external/mbedtls/include) \
	$(if $(CONFIG_$(PHASE_)SYS_THUMB_BUILD), \
		$(if $(CONFIG_HAS_THUMB2), \
			$(if $(CONFIG_CPU_V7M), \
				-I$(srctree)/arch/arm/thumb1/include), \
			-I$(srctree)/arch/arm/thumb1/include)) \
	-I$(srctree)/arch/$(ARCH)/include \
	-include $(srctree)/include/linux/kconfig.h \
	-I$(srctree)/dts/upstream/include \
	$(if $(CONFIG_NET_LWIP), -I$(srctree)/lib/lwip/lwip/src/include \
		-I$(srctree)/lib/lwip/u-boot)


KBUILD_AFLAGS   := -D__ASSEMBLY__ -fno-PIE
KBUILD_CFLAGS   := -Wall -Werror=strict-prototypes -Wno-trigraphs \
		   -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE \
		   -Werror=implicit-function-declaration -Werror=implicit-int \
		   -Wno-format-security -std=gnu11 #-Wundef Enable it and fix warnings
UBOOT_CFLAGS	:= -ffreestanding -fno-builtin
KBUILD_CFLAGS	+= $(UBOOT_CFLAGS)

KBUILD_CPPFLAGS := -D__KERNEL__ -D__UBOOT__
KBUILD_AFLAGS_KERNEL :=
KBUILD_CFLAGS_KERNEL :=
KBUILD_AFLAGS_MODULE  := -DMODULE
KBUILD_CFLAGS_MODULE  := -DMODULE
KBUILD_LDFLAGS_MODULE := -T $(srctree)/scripts/module-common.lds
KBUILD_LDFLAGS :=
GCC_PLUGINS_CFLAGS :=

export ARCH SRCARCH CONFIG_SHELL HOSTCC KBUILD_HOSTCFLAGS CROSS_COMPILE AS LD CC
export CPP AR NM STRIP OBJCOPY OBJDUMP KBUILD_HOSTLDFLAGS KBUILD_HOSTLDLIBS
export MAKE LEX YACC AWK INSTALLKERNEL PERL PYTHON PYTHON2 PYTHON3 UTS_MACHINE
export HOSTCXX KBUILD_HOSTCXXFLAGS LDFLAGS_MODULE CHECK CHECKFLAGS

export KBUILD_CPPFLAGS NOSTDINC_FLAGS LINUXINCLUDE OBJCOPYFLAGS KBUILD_LDFLAGS
export KBUILD_CFLAGS CFLAGS_KERNEL CFLAGS_MODULE
export CFLAGS_KASAN CFLAGS_KASAN_NOSANITIZE CFLAGS_UBSAN
export KBUILD_AFLAGS AFLAGS_KERNEL AFLAGS_MODULE
export KBUILD_AFLAGS_MODULE KBUILD_CFLAGS_MODULE KBUILD_LDFLAGS_MODULE
export KBUILD_AFLAGS_KERNEL KBUILD_CFLAGS_KERNEL
export KBUILD_ARFLAGS

export LDR
export CPU BOARD VENDOR SOC CPUDIR BOARDDIR
export UBOOTINCLUDE
export CC_VERSION_TEXT := $(shell $(CC) --version | head -n 1)

# When compiling out-of-tree modules, put MODVERDIR in the module
# tree rather than in the kernel tree. The kernel tree might
# even be read-only.
export MODVERDIR := $(if $(KBUILD_EXTMOD),$(firstword $(KBUILD_EXTMOD))/).tmp_versions

# Files to ignore in find ... statements

export RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o    \
			  -name CVS -o -name .pc -o -name .hg -o -name .git \) \
			  -prune -o
export RCS_TAR_IGNORE := --exclude SCCS --exclude BitKeeper --exclude .svn \
			 --exclude CVS --exclude .pc --exclude .hg --exclude .git

export PYTHON_ENABLE

# This is y if U-Boot should not build any Python tools or libraries. Typically
# you would need to set this if those tools/libraries (typically binman and
# pylibfdt) cannot be built by your environment and are provided separately.
ifeq ($(NO_PYTHON),)
PYTHON_ENABLE=y
endif

# ===========================================================================
# Rules shared between *config targets and build targets

# Basic helpers built in scripts/basic/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic
	$(Q)rm -f .tmp_quiet_recordmcount

PHONY += outputmakefile
# outputmakefile generates a Makefile in the output directory, if using a
# separate output directory. This allows convenient use of make in the
# output directory.
# At the same time when output Makefile generated, generate .gitignore to
# ignore whole output directory
outputmakefile:
ifneq ($(KBUILD_SRC),)
	$(Q)ln -fsn $(srctree) source
	$(Q)$(CONFIG_SHELL) $(srctree)/scripts/mkmakefile $(srctree)
	$(Q)test -e .gitignore || \
	{ echo "# this is build directory, ignore it"; echo "*"; } > .gitignore
endif

ifneq ($(shell $(CC) --version 2>&1 | head -n 1 | grep clang),)
ifneq ($(CROSS_COMPILE),)
CLANG_FLAGS	:= --target=$(notdir $(CROSS_COMPILE:%-=%))
GCC_TOOLCHAIN_DIR := $(dir $(shell which $(CROSS_COMPILE)elfedit))
CLANG_FLAGS	+= --prefix=$(GCC_TOOLCHAIN_DIR)
GCC_TOOLCHAIN	:= $(realpath $(GCC_TOOLCHAIN_DIR)/..)
endif
ifneq ($(GCC_TOOLCHAIN),)
CLANG_FLAGS	+= --gcc-toolchain=$(GCC_TOOLCHAIN)
endif
CLANG_FLAGS	+= -no-integrated-as
KBUILD_CFLAGS	+= $(CLANG_FLAGS)
KBUILD_AFLAGS	+= $(CLANG_FLAGS)
export CLANG_FLAGS
endif

RETPOLINE_CFLAGS_GCC := -mindirect-branch=thunk-extern -mindirect-branch-register
RETPOLINE_VDSO_CFLAGS_GCC := -mindirect-branch=thunk-inline -mindirect-branch-register
RETPOLINE_CFLAGS_CLANG := -mretpoline-external-thunk
RETPOLINE_VDSO_CFLAGS_CLANG := -mretpoline
RETPOLINE_CFLAGS := $(call cc-option,$(RETPOLINE_CFLAGS_GCC),$(call cc-option,$(RETPOLINE_CFLAGS_CLANG)))
RETPOLINE_VDSO_CFLAGS := $(call cc-option,$(RETPOLINE_VDSO_CFLAGS_GCC),$(call cc-option,$(RETPOLINE_VDSO_CFLAGS_CLANG)))
export RETPOLINE_CFLAGS
export RETPOLINE_VDSO_CFLAGS

# The expansion should be delayed until arch/$(SRCARCH)/Makefile is included.
# Some architectures define CROSS_COMPILE in arch/$(SRCARCH)/Makefile.
# CC_VERSION_TEXT is referenced from Kconfig (so it needs export),
# and from include/config/auto.conf.cmd to detect the compiler upgrade.
CC_VERSION_TEXT = $(shell $(CC) --version | head -n 1)

ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

# Read arch specific Makefile to set KBUILD_DEFCONFIG as needed.
# KBUILD_DEFCONFIG may point out an alternative default configuration
# used for 'make defconfig'
# Modified for U-Boot: we don't include arch/$(SRCARCH)/Makefile for config
# targets, which is useless since U-Boot has no architecture defining its own
# KBUILD_{DEF,K}CONFIG, or CROSS_COMPILE.
export KBUILD_DEFCONFIG KBUILD_KCONFIG CC_VERSION_TEXT

config: scripts_basic outputmakefile FORCE
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

%config: scripts_basic outputmakefile FORCE
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

else
# ===========================================================================
# Build targets only - this includes vmlinux, arch specific targets, clean
# targets and others. In general all targets except *config targets.

# If building an external module we do not care about the all: rule
# but instead _all depend on modules
PHONY += all
ifeq ($(KBUILD_EXTMOD),)
_all: all
else
_all: modules
endif

# Decide whether to build built-in, modular, or both.
# Normally, just do built-in.

KBUILD_MODULES :=
KBUILD_BUILTIN := 1

# If we have only "make modules", don't compile built-in objects.
# When we're building modules with modversions, we need to consider
# the built-in objects during the descend as well, in order to
# make sure the checksums are up to date before we record them.

ifeq ($(MAKECMDGOALS),modules)
  KBUILD_BUILTIN := $(if $(CONFIG_MODVERSIONS),1)
endif

# If we have "make <whatever> modules", compile modules
# in addition to whatever we do anyway.
# Just "make" or "make all" shall build modules as well

ifneq ($(filter all _all modules,$(MAKECMDGOALS)),)
  KBUILD_MODULES := 1
endif

ifeq ($(MAKECMDGOALS),)
  KBUILD_MODULES := 1
endif

export KBUILD_MODULES KBUILD_BUILTIN

ifeq ($(KBUILD_EXTMOD),)
# Objects we will link into vmlinux / subdirs we need to visit
init-y		:= init/
drivers-y	:= drivers/ sound/
net-y		:= net/
libs-y		:= lib/
core-y		:= usr/
virt-y		:= virt/
endif # KBUILD_EXTMOD

ifeq ($(dot-config),1)
# Modified for U-Boot
-include include/config/auto.conf
endif

# The all: target is the default when no target is given on the
# command line.
# This allow a user to issue only 'make' to build a kernel including modules
# Defaults to vmlinux, but the arch makefile usually adds further targets
all: u-boot

CFLAGS_GCOV	:= -fprofile-arcs -ftest-coverage \
	$(call cc-option,-fno-tree-loop-im) \
	$(call cc-disable-warning,maybe-uninitialized,)
export CFLAGS_GCOV

# The arch Makefiles can override CC_FLAGS_FTRACE. We may also append it later.
ifdef CONFIG_FUNCTION_TRACER
  CC_FLAGS_FTRACE := -pg
endif

# The arch Makefile can set ARCH_{CPP,A,C}FLAGS to override the default
# values of the respective KBUILD_* variables
ARCH_CPPFLAGS :=
ARCH_AFLAGS :=
ARCH_CFLAGS :=
# Modified for U-Boot: we put off the include of arch/$(SRCARCH)/Makefile until
# making sure include/config/auto.conf is up-to-date and include of config.mk,
# because the architecture-specific Makefile may make use of variables defined
# in config.mk. See also the comment about autoconf_is_old.

ifeq ($(dot-config),1)
ifeq ($(may-sync-config),1)
# Read in dependencies to all Kconfig* files, make sure to run syncconfig if
# changes are detected. This should be included after arch/$(SRCARCH)/Makefile
# because some architectures define CROSS_COMPILE there.
-include include/config/auto.conf.cmd

$(KCONFIG_CONFIG):
	@echo >&2 '***'
	@echo >&2 '*** Configuration file "$@" not found!'
	@echo >&2 '***'
	@echo >&2 '*** Please run some configurator (e.g. "make oldconfig" or'
	@echo >&2 '*** "make menuconfig" or "make xconfig").'
	@echo >&2 '***'
	@/bin/false

# If .config is newer than include/config/auto.conf, someone tinkered
# with it and forgot to run make oldconfig.
# if auto.conf.cmd is missing then we are probably in a cleaned tree so
# we execute the config step to be sure to catch updated Kconfig files
include/config/%.conf: $(KCONFIG_CONFIG) #include/config/auto.conf.cmd
	$(Q)$(MAKE) -f $(srctree)/Makefile syncconfig
	@# If the following part fails, include/config/auto.conf should be
	@# deleted so "make silentoldconfig" will be re-run on the next build.
	$(Q)$(MAKE) -f $(srctree)/scripts/Makefile.autoconf || \
		{ rm -f include/config/auto.conf; false; }
	@# include/config.h has been updated after "make silentoldconfig".
	@# We need to touch include/config/auto.conf so it gets newer
	@# than include/config.h.
	@# Otherwise, 'make silentoldconfig' would be invoked twice.
	$(Q)touch include/config/auto.conf

u-boot.cfg spl/u-boot.cfg tpl/u-boot.cfg vpl/u-boot.cfg:
	$(Q)$(MAKE) -f $(srctree)/scripts/Makefile.autoconf $(@)

-include include/autoconf.mk
-include include/autoconf.mk.dep

# We want to include arch/$(ARCH)/config.mk only when include/config/auto.conf
# is up-to-date. When we switch to a different board configuration, old CONFIG
# macros are still remaining in include/config/auto.conf. Without the following
# gimmick, wrong config.mk would be included leading nasty warnings/errors.
ifneq ($(wildcard $(KCONFIG_CONFIG)),)
ifneq ($(wildcard include/config/auto.conf),)
autoconf_is_old := $(shell find . -path ./$(KCONFIG_CONFIG) -newer \
						include/config/auto.conf)
ifeq ($(autoconf_is_old),)
include config.mk
include arch/$(ARCH)/Makefile
endif
endif
endif

# These are set by the arch-specific config.mk. Make sure they are exported
# so they can be used when building an EFI application.
export EFI_LDS		# Filename of EFI link script in arch/$(ARCH)/lib
export EFI_CRT0		# Filename of EFI CRT0 in arch/$(ARCH)/lib
export EFI_RELOC	# Filename of EFU relocation code in arch/$(ARCH)/lib
export CFLAGS_EFI	# Compiler flags to add when building EFI app
export CFLAGS_NON_EFI	# Compiler flags to remove when building EFI app
export EFI_TARGET	# binutils target if EFI is natively supported

export LTO_ENABLE

# This is y if LTO is enabled for this build. See NO_LTO=1 to disable LTO
ifeq ($(NO_LTO),)
LTO_ENABLE=$(if $(CONFIG_LTO),y)
endif

# If board code explicitly specified LDSCRIPT or CONFIG_SYS_LDSCRIPT, use
# that (or fail if absent).  Otherwise, search for a linker script in a
# standard location.

ifndef LDSCRIPT
	#LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds.debug
	ifdef CONFIG_SYS_LDSCRIPT
		# need to strip off double quotes
		LDSCRIPT := $(srctree)/$(CONFIG_SYS_LDSCRIPT:"%"=%)
	endif
endif

# If there is no specified link script, we look in a number of places for it
ifndef LDSCRIPT
	ifeq ($(wildcard $(LDSCRIPT)),)
		LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		LDSCRIPT := $(srctree)/$(CPUDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot.lds
	endif
endif

else
# External modules and some install targets need include/generated/autoconf.h
# and include/config/auto.conf but do not care if they are up-to-date.
# Use auto.conf to trigger the test
PHONY += include/config/auto.conf

include/config/auto.conf:
	$(Q)test -e include/generated/autoconf.h -a -e $@ || (		\
	echo >&2;							\
	echo >&2 "  ERROR: Kernel configuration is invalid.";		\
	echo >&2 "         include/generated/autoconf.h or $@ are missing.";\
	echo >&2 "         Run 'make oldconfig && make prepare' on kernel src to fix it.";	\
	echo >&2 ;							\
	/bin/false)

endif # may-sync-config
endif # $(dot-config)

KBUILD_CFLAGS	+= $(call cc-option,-fno-delete-null-pointer-checks,)
KBUILD_CFLAGS	+= $(call cc-disable-warning,frame-address,)
KBUILD_CFLAGS	+= $(call cc-disable-warning, format-truncation)
KBUILD_CFLAGS	+= $(call cc-disable-warning, format-overflow)
KBUILD_CFLAGS	+= $(call cc-disable-warning, int-in-bool-context)
KBUILD_CFLAGS	+= $(call cc-disable-warning, address-of-packed-member)

ifdef CONFIG_CC_OPTIMIZE_FOR_DEBUG
KBUILD_HOSTCFLAGS   := -Wall -Wstrict-prototypes -Og -g -fomit-frame-pointer \
		$(HOST_LFS_CFLAGS) $(HOSTCFLAGS)
# Avoid false positives -Wmaybe-uninitialized
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78394
KBUILD_HOSTCFLAGS   += -Wno-maybe-uninitialized
KBUILD_HOSTCXXFLAGS := -Og -g $(HOST_LFS_CFLAGS) $(HOSTCXXFLAGS)
endif

#
# Xtensa linker script cannot be preprocessed with -ansi because of
# preprocessor operations on strings that don't make C identifiers.
#
ifeq ($(CONFIG_XTENSA),)
LDPPFLAGS	+= -ansi
endif

ifdef CONFIG_CC_OPTIMIZE_FOR_SIZE
KBUILD_CFLAGS	+= -Os
else
KBUILD_CFLAGS   += -O2
endif

ifdef CONFIG_CC_DISABLE_WARN_MAYBE_UNINITIALIZED
KBUILD_CFLAGS   += -Wno-maybe-uninitialized
endif

# Tell gcc to never replace conditional load with a non-conditional one
KBUILD_CFLAGS	+= $(call cc-option,--param=allow-store-data-races=0)

include scripts/Makefile.kcov
include scripts/Makefile.gcc-plugins
LTO_CFLAGS :=
LTO_FINAL_LDFLAGS :=
export LTO_CFLAGS LTO_FINAL_LDFLAGS
ifeq ($(LTO_ENABLE),y)
	ifeq ($(cc-name),clang)
		LTO_CFLAGS		+= -DLTO_ENABLE -flto
		LTO_FINAL_LDFLAGS	+= -flto

		AR			= $(shell $(CC) -print-prog-name=llvm-ar)
		NM			= $(shell $(CC) -print-prog-name=llvm-nm)
	else
		NPROC			:= $(shell nproc 2>/dev/null || echo 1)
		LTO_CFLAGS		+= -DLTO_ENABLE -flto=$(NPROC)
		LTO_FINAL_LDFLAGS	+= -fuse-linker-plugin -flto=$(NPROC)

		# use plugin aware tools
		AR			= $(CROSS_COMPILE)gcc-ar
		NM			= $(CROSS_COMPILE)gcc-nm
	endif

	CFLAGS_NON_EFI			+= $(LTO_CFLAGS)

	KBUILD_CFLAGS			+= $(LTO_CFLAGS)
endif

ifeq ($(CONFIG_STACKPROTECTOR),y)
KBUILD_CFLAGS += $(call cc-option,-fstack-protector-strong)
KBUILD_CFLAGS += $(call cc-option,-mstack-protector-guard=global)
CFLAGS_EFI += $(call cc-option,-fno-stack-protector)
else
KBUILD_CFLAGS += $(call cc-option,-fno-stack-protector)
endif
KBUILD_CFLAGS += $(call cc-option,-fno-delete-null-pointer-checks)

KBUILD_CFLAGS += $(call cc-disable-warning, zero-length-bounds)
KBUILD_CFLAGS += $(call cc-disable-warning, array-bounds)
KBUILD_CFLAGS += $(call cc-disable-warning, stringop-overflow)

# Enabled with W=2, disabled by default as noisy
KBUILD_CFLAGS += $(call cc-disable-warning, maybe-uninitialized)

# change __FILE__ to the relative path from the srctree
KBUILD_CFLAGS	+= $(call cc-option,-fmacro-prefix-map=$(srctree)/=)

KBUILD_CFLAGS	+= -gdwarf-4
# $(KBUILD_AFLAGS) sets -g, which causes gcc to pass a suitable -g<format>
# option to the assembler.
KBUILD_AFLAGS	+= -gdwarf-4

# Report stack usage if supported
# ARC tools based on GCC 7.1 has an issue with stack usage
# with naked functions, see commit message for more details
ifndef CONFIG_ARC
ifeq ($(shell $(CONFIG_SHELL) $(srctree)/scripts/gcc-stack-usage.sh $(CC)),y)
	KBUILD_CFLAGS += -fstack-usage
endif
endif

KBUILD_CFLAGS += $(call cc-option,-Wno-format-nonliteral)
KBUILD_CFLAGS += $(call cc-disable-warning, address-of-packed-member)

ifdef CONFIG_CC_IS_CLANG
KBUILD_CPPFLAGS += $(call cc-option,-Qunused-arguments,)
KBUILD_CFLAGS += $(call cc-disable-warning, format-invalid-specifier)
KBUILD_CFLAGS += $(call cc-disable-warning, gnu)
# Quiet clang warning: comparison of unsigned expression < 0 is always false
KBUILD_CFLAGS += $(call cc-disable-warning, tautological-compare)
# CLANG uses a _MergedGlobals as optimization, but this breaks modpost, as the
# source of a reference will be _MergedGlobals and not on of the whitelisted names.
# See modpost pattern 2
KBUILD_CFLAGS += $(call cc-option, -mno-global-merge,)
KBUILD_CFLAGS += $(call cc-option, -fcatch-undefined-behavior)
KBUILD_CFLAGS += $(call cc-disable-warning, deprecated-non-prototype)
else

# These warnings generated too much noise in a regular build.
# Use make W=1 to enable them (see scripts/Makefile.extrawarn)
KBUILD_CFLAGS += -Wno-unused-but-set-variable
endif

# These warnings generated too much noise in a regular build.
# Use make W=1 to enable them (see scripts/Makefile.extrawarn)
KBUILD_CFLAGS += $(call cc-disable-warning, unused-but-set-variable)

# Prohibit date/time macros, which would make the build non-deterministic
KBUILD_CFLAGS   += $(call cc-option,-Werror=date-time)

include scripts/Makefile.extrawarn

# Add user supplied CPPFLAGS, AFLAGS and CFLAGS as the last assignments
KBUILD_CPPFLAGS += $(KCPPFLAGS)
KBUILD_AFLAGS += $(KAFLAGS)
KBUILD_CFLAGS += $(KCFLAGS)

KBUILD_LDFLAGS  += -z noexecstack
KBUILD_LDFLAGS  += -z norelro
KBUILD_LDFLAGS  += $(call ld-option,--no-warn-rwx-segments)

KBUILD_HOSTCFLAGS += $(if $(CONFIG_TOOLS_DEBUG),-g)

# Use UBOOTINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
UBOOTINCLUDE    := \
	-Iinclude \
	$(if $(KBUILD_SRC), -I$(srctree)/include) \
	$(if $(CONFIG_$(XPL_)MBEDTLS_LIB), \
		"-DMBEDTLS_CONFIG_FILE=\"mbedtls_def_config.h\"" \
		-I$(srctree)/lib/mbedtls \
		-I$(srctree)/lib/mbedtls/port \
		-I$(srctree)/lib/mbedtls/external/mbedtls \
		-I$(srctree)/lib/mbedtls/external/mbedtls/include) \
	$(if $(CONFIG_$(PHASE_)SYS_THUMB_BUILD), \
		$(if $(CONFIG_HAS_THUMB2), \
			$(if $(CONFIG_CPU_V7M), \
				-I$(srctree)/arch/arm/thumb1/include), \
			-I$(srctree)/arch/arm/thumb1/include)) \
	-I$(srctree)/arch/$(ARCH)/include \
	-include $(srctree)/include/linux/kconfig.h \
	-I$(srctree)/dts/upstream/include \
	$(if $(CONFIG_NET_LWIP), -I$(srctree)/lib/lwip/lwip/src/include \
		-I$(srctree)/lib/lwip/u-boot)

NOSTDINC_FLAGS += -nostdinc -isystem $(shell $(CC) -print-file-name=include)

# FIX ME
cpp_flags := $(KBUILD_CPPFLAGS) $(PLATFORM_CPPFLAGS) $(UBOOTINCLUDE) \
							$(NOSTDINC_FLAGS)
c_flags := $(KBUILD_CFLAGS) $(cpp_flags)


#########################################################################
# U-Boot objects....order is important (i.e. start must be first)

HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)

libs-$(CONFIG_API) += api/
libs-$(HAVE_VENDOR_COMMON_LIB) += board/$(VENDOR)/common/
libs-y += boot/
libs-$(CONFIG_CMDLINE) += cmd/
libs-y += common/
libs-$(CONFIG_OF_EMBED) += dts/
libs-y += env/
libs-y += lib/
libs-y += fs/
libs-$(filter y,$(CONFIG_NET) $(CONFIG_NET_LWIP)) += net/
libs-y += disk/
libs-y += drivers/
libs-$(CONFIG_SYS_FSL_DDR) += drivers/ddr/fsl/
libs-$(CONFIG_SYS_FSL_MMDC) += drivers/ddr/fsl/
libs-$(CONFIG_$(PHASE_)ALTERA_SDRAM) += drivers/ddr/altera/
libs-y += drivers/usb/cdns3/
libs-y += drivers/usb/dwc3/
libs-y += drivers/usb/common/
libs-y += drivers/usb/emul/
libs-y += drivers/usb/eth/
libs-$(CONFIG_USB_GADGET) += drivers/usb/gadget/
libs-$(CONFIG_USB_GADGET) += drivers/usb/gadget/udc/
libs-y += drivers/usb/host/
libs-y += drivers/usb/mtu3/
libs-y += drivers/usb/musb/
libs-y += drivers/usb/musb-new/
libs-y += drivers/usb/isp1760/
libs-y += drivers/usb/phy/
libs-y += drivers/usb/tcpm/
libs-y += drivers/usb/ulpi/
ifdef CONFIG_POST
libs-y += post/
endif
libs-$(CONFIG_$(PHASE_)UNIT_TEST) += test/

libs-y += $(if $(wildcard $(srctree)/board/$(BOARDDIR)/Makefile),board/$(BOARDDIR)/)

libs-y := $(sort $(libs-y))

u-boot-dirs	:= $(patsubst %/,%,$(filter %/, $(libs-y))) tools examples

u-boot-alldirs	:= $(sort $(u-boot-dirs) $(patsubst %/,%,$(filter %/, $(libs-))))

libs-y		:= $(patsubst %/, %/built-in.a, $(libs-y))
# Not needed in U-Boot
#init-y		:= $(patsubst %/, %/built-in.a, $(init-y))
#core-y		:= $(patsubst %/, %/built-in.a, $(core-y))
drivers-y	:= $(patsubst %/, %/built-in.a, $(drivers-y))
#net-y		:= $(patsubst %/, %/built-in.a, $(net-y))
libs-y1	:= $(patsubst %/, %/lib.a, $(libs-y))
libs-y2	:= $(patsubst %/, %/built-in.a, $(filter-out %.a, $(libs-y)))
#virt-y		:= $(patsubst %/, %/built-in.a, $(virt-y))

u-boot-init := $(head-y)
u-boot-main := $(libs-y)


# Add GCC lib
ifeq ($(CONFIG_USE_PRIVATE_LIBGCC),y)
PLATFORM_LIBGCC = arch/$(ARCH)/lib/lib.a
else
ifndef CONFIG_CC_IS_CLANG
PLATFORM_LIBGCC := -L $(shell dirname `$(CC) $(c_flags) -print-libgcc-file-name`) -lgcc
endif
endif
PLATFORM_LIBS += $(PLATFORM_LIBGCC)

ifdef CONFIG_CC_COVERAGE
KBUILD_CFLAGS += --coverage
PLATFORM_LIBGCC += -lgcov
endif

export PLATFORM_LIBS
export PLATFORM_LIBGCC

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(srctree)/include/u-boot/u-boot.lds.h \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

#########################################################################
#########################################################################

ifneq ($(CONFIG_BOARD_SIZE_LIMIT),)
BOARD_SIZE_CHECK= @ $(call size_check,$@,$(CONFIG_BOARD_SIZE_LIMIT))
else
BOARD_SIZE_CHECK =
endif

ifneq ($(CONFIG_SPL_SIZE_LIMIT),0x0)
SPL_SIZE_CHECK = @$(call size_check,$@,$$(tools/spl_size_limit))
else
SPL_SIZE_CHECK =
endif

ifneq ($(CONFIG_TPL_SIZE_LIMIT),0x0)
TPL_SIZE_CHECK = @$(call size_check,$@,$(CONFIG_TPL_SIZE_LIMIT))
else
TPL_SIZE_CHECK =
endif

ifneq ($(CONFIG_VPL_SIZE_LIMIT),0x0)
VPL_SIZE_CHECK = @$(call size_check,$@,$(CONFIG_VPL_SIZE_LIMIT))
else
VPL_SIZE_CHECK =
endif

# Statically apply RELA-style relocations (currently arm64 only)
# This is useful for arm64 where static relocation needs to be performed on
# the raw binary, but certain simulators only accept an ELF file (but don't
# do the relocation).
ifneq ($(CONFIG_STATIC_RELA),)
# $(2) is u-boot ELF, $(3) is u-boot bin, $(4) is text base
quiet_cmd_static_rela = RELOC   $@
cmd_static_rela = \
	tools/relocate-rela $(3) $(2)
else
quiet_cmd_static_rela =
cmd_static_rela =
endif

# Always append INPUTS so that arch config.mk's can add custom ones
INPUTS-y += u-boot.srec u-boot.bin u-boot.sym System.map binary_size_check

ifeq ($(CONFIG_SPL_FSL_PBL),y)
INPUTS-$(CONFIG_RAMBOOT_PBL) += u-boot-with-spl-pbl.bin
else
ifneq ($(CONFIG_NXP_ESBC), y)
# For Secure Boot The Image needs to be signed and Header must also
# be included. So The image has to be built explicitly
INPUTS-$(CONFIG_RAMBOOT_PBL) += u-boot.pbl
endif
endif
INPUTS-$(CONFIG_SPL) += spl/u-boot-spl.bin
ifeq ($(CONFIG_MX6)$(CONFIG_IMX_HAB), yy)
INPUTS-$(CONFIG_SPL_FRAMEWORK) += u-boot-ivt.img
else
ifeq ($(CONFIG_MX7)$(CONFIG_IMX_HAB), yy)
INPUTS-$(CONFIG_SPL_FRAMEWORK) += u-boot-ivt.img
else
INPUTS-$(CONFIG_SPL_FRAMEWORK) += u-boot.img
endif
endif
INPUTS-$(CONFIG_TPL) += tpl/u-boot-tpl.bin
INPUTS-$(CONFIG_VPL) += vpl/u-boot-vpl.bin

# Allow omitting the .dtb output if it is not normally used
INPUTS-$(CONFIG_OF_SEPARATE) += $(if $(CONFIG_OF_OMIT_DTB),dts/dt.dtb,u-boot.dtb)
ifeq ($(CONFIG_SPL_FRAMEWORK),y)
INPUTS-$(CONFIG_OF_SEPARATE) += u-boot-dtb.img
endif
INPUTS-$(CONFIG_SANDBOX) += u-boot.dtb
ifneq ($(CONFIG_SPL_TARGET),)
INPUTS-$(CONFIG_SPL) += $(CONFIG_SPL_TARGET:"%"=%)
endif
INPUTS-$(CONFIG_REMAKE_ELF) += u-boot.elf
INPUTS-$(CONFIG_EFI_APP) += u-boot-app.efi
INPUTS-$(CONFIG_EFI_STUB) += u-boot-payload.efi

# Generate this input file for binman
ifeq ($(CONFIG_SPL),)
ifneq ($(patsubst "%",%,$(CONFIG_MTK_BROM_HEADER_INFO)),)
INPUTS-$(CONFIG_ARCH_MEDIATEK) += u-boot-mtk.bin
endif
endif

ifdef CONFIG_FUNCTION_TRACER
ifdef CONFIG_FTRACE_MCOUNT_RECORD
  # gcc 5 supports generating the mcount tables directly
  ifeq ($(call cc-option-yn,-mrecord-mcount),y)
    CC_FLAGS_FTRACE	+= -mrecord-mcount
    export CC_USING_RECORD_MCOUNT := 1
  endif
  ifdef CONFIG_HAVE_NOP_MCOUNT
    ifeq ($(call cc-option-yn, -mnop-mcount),y)
      CC_FLAGS_FTRACE	+= -mnop-mcount
      CC_FLAGS_USING	+= -DCC_USING_NOP_MCOUNT
    endif
  endif
endif
ifdef CONFIG_HAVE_FENTRY
  ifeq ($(call cc-option-yn, -mfentry),y)
    CC_FLAGS_FTRACE	+= -mfentry
    CC_FLAGS_USING	+= -DCC_USING_FENTRY
  endif
endif
export CC_FLAGS_FTRACE
KBUILD_CFLAGS	+= $(CC_FLAGS_FTRACE) $(CC_FLAGS_USING)
KBUILD_AFLAGS	+= $(CC_FLAGS_USING)
ifdef CONFIG_DYNAMIC_FTRACE
	ifdef CONFIG_HAVE_C_RECORDMCOUNT
		BUILD_C_RECORDMCOUNT := y
		export BUILD_C_RECORDMCOUNT
	endif
endif
endif

# Add optional build target if defined in board/cpu/soc headers
ifneq ($(CONFIG_BUILD_TARGET),)
INPUTS-y += $(CONFIG_BUILD_TARGET:"%"=%)
endif

ifeq ($(CONFIG_INIT_SP_RELATIVE)$(CONFIG_OF_SEPARATE),yy)
INPUTS-y += init_sp_bss_offset_check
endif

ifeq ($(CONFIG_ARCH_ROCKCHIP)_$(CONFIG_SPL_FRAMEWORK),y_)
INPUTS-y += u-boot.img
endif

INPUTS-$(CONFIG_X86) += u-boot-x86-start16.bin u-boot-x86-reset16.bin \
	$(if $(CONFIG_SPL_X86_16BIT_INIT),spl/u-boot-spl.bin) \
	$(if $(CONFIG_TPL_X86_16BIT_INIT),tpl/u-boot-tpl.bin)

LDFLAGS_u-boot += $(LDFLAGS_FINAL)

# Avoid 'Not enough room for program headers' error on binutils 2.28 onwards.
LDFLAGS_u-boot += $(call ld-option, --no-dynamic-linker)

# ld.lld support
LDFLAGS_u-boot += -z notext $(call ld-option,--apply-dynamic-relocs)

LDFLAGS_u-boot += --build-id=none

ifeq ($(CONFIG_ARC)$(CONFIG_NIOS2)$(CONFIG_X86)$(CONFIG_XTENSA),)
LDFLAGS_u-boot += -Ttext $(CONFIG_TEXT_BASE)
endif

# make the checker run with the right architecture
CHECKFLAGS += --arch=$(ARCH)

# insure the checker run with the right endianness
CHECKFLAGS += $(if $(CONFIG_SYS_BIG_ENDIAN),-mbig-endian,-mlittle-endian)

# the checker needs the correct machine size
CHECKFLAGS += $(if $(CONFIG_64BIT),-m64,-m32)

# Normally we fill empty space with 0xff
quiet_cmd_objcopy = OBJCOPY $@
cmd_objcopy = $(OBJCOPY) --gap-fill=0xff $(OBJCOPYFLAGS) \
	$(OBJCOPYFLAGS_$(@F)) $< $@

# disable pointer signed / unsigned warnings in gcc 4.0
KBUILD_CFLAGS += -Wno-pointer-sign

# disable stringop warnings in gcc 8+
KBUILD_CFLAGS += $(call cc-disable-warning, stringop-truncation)

# disable invalid "can't wrap" optimizations for signed / pointers
KBUILD_CFLAGS	+= $(call cc-option,-fno-strict-overflow)

# Provide a version which does not do this, for use by EFI and hex/srec
quiet_cmd_zobjcopy = OBJCOPY $@
cmd_zobjcopy = $(OBJCOPY) $(OBJCOPYFLAGS) $(OBJCOPYFLAGS_$(@F)) $< $@

quiet_cmd_efipayload = OBJCOPY $@
cmd_efipayload = $(OBJCOPY) -I binary -O $(EFIPAYLOAD_BFDTARGET) -B $(EFIPAYLOAD_BFDARCH) $< $@

MKIMAGEOUTPUT ?= /dev/null

quiet_cmd_mkimage = MKIMAGE $@
cmd_mkimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) -d $< $@ \
	>$(MKIMAGEOUTPUT) $(if $(KBUILD_VERBOSE:0=), && cat $(MKIMAGEOUTPUT))

quiet_cmd_mkfitimage = MKIMAGE $@
cmd_mkfitimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) \
	-f $(U_BOOT_ITS) -p $(CONFIG_FIT_EXTERNAL_OFFSET) $@ \
	>$(MKIMAGEOUTPUT) $(if $(KBUILD_VERBOSE:0=), && cat $(MKIMAGEOUTPUT))

quiet_cmd_cat = CAT     $@
cmd_cat = cat $(filter-out $(PHONY), $^) > $@

append = cat $(filter-out $< $(PHONY), $^) >> $@

quiet_cmd_pad_cat = CAT     $@
cmd_pad_cat = $(cmd_objcopy) && $(append) || { rm -f $@; false; }

quiet_cmd_lzma = LZMA    $@
cmd_lzma = lzma -c -z -k -9 $< > $@

cfg: u-boot.cfg

quiet_cmd_ofcheck = OFCHK   $2
cmd_ofcheck = $(srctree)/scripts/check-of.sh $2 \
		$(srctree)/scripts/of_allowlist.txt

# Concat the value of all the CONFIGs (result is 'y' or 'yy', etc. )
got = $(foreach cfg,$(1),$($(cfg)))

# expected value 'y for each one
expect = $(foreach cfg,$(1),y)

# Show a deprecation message
# Args:
# 1: List of options to migrate to (e.g. "CONFIG_DM_MMC CONFIG_BLK")
# 2: Name of component (e.g . "Ethernet drivers")
# 3: Release deadline (e.g. "v202.07")
# 4: Condition to require before checking (e.g. "$(CONFIG_NET)")
# Note: Script avoids bash construct, hence the strange double 'if'
# (patches welcome!)
define deprecated
	@if [ -n "$(strip $(4))" ]; then if [ "$(got)" != "$(expect)" ]; then \
		echo >&2 "===================== WARNING ======================"; \
		echo >&2 "This board does not use $(firstword $(1)) (Driver Model"; \
		echo >&2 "for $(2)). Please update the board to use"; \
		echo >&2 "$(firstword $(1)) before the $(3) release. Failure to"; \
		echo >&2 "update by the deadline may result in board removal."; \
		echo >&2 "See doc/develop/driver-model/migration.rst for more info."; \
		echo >&2 "===================================================="; \
	fi; fi

endef

# Timestamp file to make sure that binman always runs
.binman_stamp: $(INPUTS-y) FORCE
ifeq ($(CONFIG_BINMAN),y)
	$(call if_changed,binman)
endif
	@touch $@

all: .binman_stamp

ifeq ($(CONFIG_DEPRECATED),y)
	$(warning "You have deprecated configuration options enabled in your .config! Please check your configuration.")
endif
ifeq ($(CONFIG_OF_EMBED)$(CONFIG_EFI_APP),y)
	@echo >&2 "===================== WARNING ======================"
	@echo >&2 "CONFIG_OF_EMBED is enabled. This option should only"
	@echo >&2 "be used for debugging purposes. Please use"
	@echo >&2 "CONFIG_OF_SEPARATE for boards in mainline."
	@echo >&2 "See doc/develop/devicetree/control.rst for more info."
	@echo >&2 "===================================================="
endif
	$(call deprecated,CONFIG_WDT,DM watchdog,v2019.10,\
		$(CONFIG_WATCHDOG)$(CONFIG_HW_WATCHDOG))
	$(call deprecated,CONFIG_DM_I2C,I2C drivers,v2022.04,$(CONFIG_SYS_I2C_LEGACY))
	@# CFG_SYS_TIMER_RATE has brackets in it for some boards which
	@# confuses this rule. Use if() to send just a single character which
	@# is enable to tell 'deprecated' that one of these symbols exists
	$(call deprecated,CONFIG_TIMER,Timer drivers,v2023.01,$(if $(strip $(CFG_SYS_TIMER_RATE)$(CFG_SYS_TIMER_COUNTER)),x))
	$(call deprecated,CONFIG_DM_SERIAL,Serial drivers,v2023.04,$(CONFIG_SERIAL))
	@# Check that this build does not override OF_HAS_PRIOR_STAGE by
	@# disabling OF_BOARD.
	$(call cmd,ofcheck,$(KCONFIG_CONFIG))

PHONY += dtbs dtbs_check
dtbs: dts/dt.dtb
	@:
dts/dt.dtb: dtbs_prepare u-boot
	$(Q)$(MAKE) $(build)=dts dtbs

dtbs_prepare: prepare3

ifneq ($(filter dtbs_check, $(MAKECMDGOALS)),)
export CHECK_DTBS=y
endif

ifneq ($(CHECK_DTBS),)
dtbs_prepare: dt_binding_check
endif

dtbs_check: dt_binding_check dtbs

DT_BINDING_DIR := dts/upstream/Bindings
dt_binding_check: scripts_dtc
	$(Q)$(MAKE) $(build)=$(DT_BINDING_DIR) $(DT_BINDING_DIR)/processed-schema.json

quiet_cmd_copy = COPY    $@
      cmd_copy = cp $< $@

ifeq ($(CONFIG_OF_UPSTREAM),y)
ifeq ($(CONFIG_ARM64),y)
dt_dir := dts/upstream/src/arm64
else
dt_dir := dts/upstream/src/$(ARCH)
endif
else
dt_dir := arch/$(ARCH)/dts
endif

ifeq ($(CONFIG_MULTI_DTB_FIT),y)

ifeq ($(CONFIG_MULTI_DTB_FIT_LZO),y)
FINAL_DTB_CONTAINER = fit-dtb.blob.lzo
else ifeq ($(CONFIG_MULTI_DTB_FIT_GZIP),y)
FINAL_DTB_CONTAINER = fit-dtb.blob.gz
else
FINAL_DTB_CONTAINER = fit-dtb.blob
endif

fit-dtb.blob.gz: fit-dtb.blob
	@gzip -nkf9 $< > $@

fit-dtb.blob.lzo: fit-dtb.blob
	@lzop -f9 $< > $@

fit-dtb.blob: dts/dt.dtb FORCE
	$(call if_changed,mkimage)
ifneq ($(SOURCE_DATE_EPOCH),)
	touch -d @$(SOURCE_DATE_EPOCH) fit-dtb.blob
	chmod 0600 fit-dtb.blob
endif

MKIMAGEFLAGS_fit-dtb.blob = -f auto -A $(ARCH) -T firmware -C none -O u-boot \
	-a 0 -e 0 -E \
	$(patsubst %,-b $(dt_dir)/%.dtb,$(subst ",,$(CONFIG_OF_LIST))) -d /dev/null

MKIMAGEFLAGS_fit-dtb.blob += -B 0x8

ifneq ($(EXT_DTB),)
u-boot-fit-dtb.bin: u-boot-nodtb.bin $(EXT_DTB)
		$(call if_changed,cat)
else
u-boot-fit-dtb.bin: u-boot-nodtb.bin $(FINAL_DTB_CONTAINER)
	$(call if_changed,cat)
endif

u-boot.bin: u-boot-fit-dtb.bin FORCE
	$(call if_changed,copy)

ifneq ($(CONFIG_MPC85XX_HAVE_RESET_VECTOR)$(CONFIG_OF_SEPARATE),yy)
u-boot-dtb.bin: u-boot-nodtb.bin dts/dt.dtb FORCE
	$(call if_changed,cat)
endif

else ifeq ($(CONFIG_OF_SEPARATE).$(CONFIG_OF_OMIT_DTB),y.)

ifneq ($(CONFIG_MPC85XX_HAVE_RESET_VECTOR)$(CONFIG_OF_SEPARATE),yy)
u-boot-dtb.bin: u-boot-nodtb.bin dts/dt.dtb FORCE
	$(call if_changed,cat)
endif

u-boot.bin: u-boot-dtb.bin FORCE
	$(call if_changed,copy)

else
u-boot.bin: u-boot-nodtb.bin FORCE
	$(call if_changed,copy)
endif

# we call Makefile in arch/arm/mach-imx which
# has targets which are dependent on targets defined
# here. make could not resolve them and we must ensure
# that they are finished before calling imx targets
ifeq ($(CONFIG_MULTI_DTB_FIT),y)
IMX_DEPS = u-boot-fit-dtb.bin
endif

%.imx: $(IMX_DEPS) %.bin
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@
	$(BOARD_SIZE_CHECK)

%.vyb: %.imx
	$(Q)$(MAKE) $(build)=arch/arm/cpu/armv7/vf610 $@

quiet_cmd_copy = COPY    $@
      cmd_copy = cp $< $@

u-boot.dtb: dts/dt.dtb
	$(call cmd,copy)

OBJCOPYFLAGS_u-boot.hex := -O ihex

OBJCOPYFLAGS_u-boot.srec := -O srec

u-boot.hex u-boot.srec: u-boot FORCE
	$(call if_changed,zobjcopy)

OBJCOPYFLAGS_u-boot-elf.srec := $(OBJCOPYFLAGS_u-boot.srec)

ifeq ($(CONFIG_POSITION_INDEPENDENT)$(CONFIG_RCAR_GEN3),yy)
# The flash_writer tool and previous recovery tools
# require the SREC load address to be 0x5000_0000 .
# The PIE U-Boot build sets the address to 0x0, so
# override the address back to make u-boot-elf.srec
# compatible with the recovery tools.
OBJCOPYFLAGS_u-boot-elf.srec += --change-addresses=0x50000000
endif

u-boot-elf.srec: u-boot.elf FORCE
	$(call if_changed,zobjcopy)

OBJCOPYFLAGS_u-boot-spl.srec = $(OBJCOPYFLAGS_u-boot.srec)

spl/u-boot-spl.srec: spl/u-boot-spl FORCE
	$(call if_changed,zobjcopy)

%.scif: %.srec
	$(Q)$(MAKE) $(build)=arch/arm/mach-renesas $@

OBJCOPYFLAGS_u-boot-nodtb.bin := -O binary \
		$(if $(CONFIG_X86_16BIT_INIT),-R .start16 -R .resetvec) \
		$(if $(CONFIG_MPC85XX_HAVE_RESET_VECTOR),$(if $(CONFIG_OF_SEPARATE),-R .bootpg -R .resetvec))

binary_size_check: u-boot-nodtb.bin FORCE
	@file_size=$(shell wc -c u-boot-nodtb.bin | awk '{ print $$1 }') ; \
	map_size=$(shell cat u-boot.map | \
		awk ' \
			/_image_copy_start/ { start = $$1 } \
			/_image_binary_end/ { end = $$1 } \
			END { \
				if (start != "" && end != "") \
					print end " " start; \
			}' \
		| sh -c 'read end start && echo $$((end - start))'); \
	if [ -n "$$map_size" ]; then \
		if test $$map_size -ne $$file_size; then \
			echo "u-boot.map shows a binary size of $$map_size" >&2 ; \
			echo "  but u-boot-nodtb.bin shows $$file_size" >&2 ; \
			exit 1; \
		fi; \
	fi

ifeq ($(CONFIG_INIT_SP_RELATIVE)$(CONFIG_OF_SEPARATE),yy)
ifneq ($(CONFIG_SYS_MALLOC_F),)
subtract_sys_malloc_f_len = space=$$(($${space} - $(CONFIG_SYS_MALLOC_F_LEN)))
else
subtract_sys_malloc_f_len = true
endif
# The 1/4 margin below is somewhat arbitrary. The likely initial SP usage is
# so low that the DTB could probably use 90%+ of the available space, for
# current values of CONFIG_SYS_INIT_SP_BSS_OFFSET at least. However, let's be
# safe for now and tweak this later if space becomes tight.
# A rejected alternative would be to check that some absolute minimum stack
# space was available. However, since CONFIG_SYS_INIT_SP_BSS_OFFSET is
# deliberately build-specific, to take account of build-to-build stack usage
# differences due to different feature sets, there is no common absolute value
# to check against.
init_sp_bss_offset_check: u-boot.dtb FORCE
	@dtb_size=$(shell wc -c u-boot.dtb | awk '{print $$1}') ; \
	space=$(CONFIG_SYS_INIT_SP_BSS_OFFSET) ; \
	$(subtract_sys_malloc_f_len) ; \
	quarter_space=$$(($${space} / 4)) ; \
	if [ $${dtb_size} -gt $${quarter_space} ]; then \
		echo "u-boot.dtb is larger than 1 quarter of " >&2 ; \
		echo "(CONFIG_SYS_INIT_SP_BSS_OFFSET - CONFIG_SYS_MALLOC_F_LEN)" >&2 ; \
		exit 1 ; \
	fi
endif

shell_cmd = { $(call echo-cmd,$(1)) $(cmd_$(1)); }

quiet_cmd_objcopy_uboot = OBJCOPY $@
ifdef cmd_static_rela
cmd_objcopy_uboot = $(cmd_objcopy) && $(call shell_cmd,static_rela,$<,$@,$(CONFIG_TEXT_BASE)) || { rm -f $@; false; }
else
cmd_objcopy_uboot = $(cmd_objcopy)
endif

u-boot-nodtb.bin: u-boot FORCE
	$(call if_changed,objcopy_uboot)
	$(BOARD_SIZE_CHECK)

u-boot.ldr:	u-boot
		$(LDR) -T $(CONFIG_LDR_CPU) -c $@ $< $(LDR_FLAGS)
		$(BOARD_SIZE_CHECK)

# binman
# ---------------------------------------------------------------------------
# Use 'make BINMAN_DEBUG=1' to enable debugging
# Use 'make BINMAN_VERBOSE=3' to set vebosity level

ifneq ($(EXT_DTB),)
ext_dtb_list := $(basename $(notdir $(EXT_DTB)))
default_dt := $(firstword $(ext_dtb_list))
of_list := "$(ext_dtb_list)"
of_list_dirs := $(dir $(EXT_DTB)) $(dt_dir)
else
of_list := $(CONFIG_OF_LIST)
of_list_dirs := $(dt_dir)
default_dt := $(if $(DEVICE_TREE),$(DEVICE_TREE),$(CONFIG_DEFAULT_DEVICE_TREE))
endif

ifneq ($(CONFIG_OF_UPSTREAM_INCLUDE_LOCAL_FALLBACK_DTBOS),)
of_list_dirs += arch/$(ARCH)/dts
endif

binman_dtb := $(shell echo $(CONFIG_BINMAN_DTB))
ifeq ($(strip $(binman_dtb)),)
ifeq ($(CONFIG_OF_EMBED),y)
binman_dtb = ./dts/dt.dtb
else
binman_dtb = ./u-boot.dtb
endif
endif

quiet_cmd_binman = BINMAN  $@
cmd_binman = $(srctree)/tools/binman/binman $(if $(BINMAN_DEBUG),-D) \
		$(foreach f,$(BINMAN_TOOLPATHS),--toolpath $(f)) \
                --toolpath $(objtree)/tools \
		$(if $(BINMAN_VERBOSE),-v$(BINMAN_VERBOSE)) \
		build -u -d $(binman_dtb) -O . -m \
		--allow-missing --fake-ext-blobs \
		$(if $(BINMAN_ALLOW_MISSING),--ignore-missing) \
		-I . -I $(srctree) -I $(srctree)/board/$(BOARDDIR) \
		$(foreach f,$(of_list_dirs),-I $(f)) -a of-list=$(of_list) \
		$(foreach f,$(BINMAN_INDIRS),-I $(f)) \
		-a atf-bl1-path=${BL1} \
		-a atf-bl31-path=${BL31} \
		-a tee-os-path=${TEE} \
		-a ti-dm-path=${TI_DM} \
		-a opensbi-path=${OPENSBI} \
		-a default-dt=$(default_dt) \
		-a scp-path=$(SCP) \
		-a rockchip-tpl-path=$(ROCKCHIP_TPL) \
		-a spl-bss-pad=$(if $(CONFIG_SPL_SEPARATE_BSS),,1) \
		-a tpl-bss-pad=$(if $(CONFIG_TPL_SEPARATE_BSS),,1) \
		-a vpl-bss-pad=$(if $(CONFIG_VPL_SEPARATE_BSS),,1) \
		-a spl-dtb=$(CONFIG_SPL_OF_REAL) \
		-a tpl-dtb=$(CONFIG_TPL_OF_REAL) \
		-a vpl-dtb=$(CONFIG_VPL_OF_REAL) \
		-a pre-load-key-path=${PRE_LOAD_KEY_PATH} \
		-a of-spl-remove-props=$(CONFIG_OF_SPL_REMOVE_PROPS) \
		$(BINMAN_$(@F))

OBJCOPYFLAGS_u-boot.ldr.hex := -I binary -O ihex

OBJCOPYFLAGS_u-boot.ldr.srec := -I binary -O srec

u-boot.ldr.hex u-boot.ldr.srec: u-boot.ldr FORCE
	$(call if_changed,zobjcopy)

ifdef CONFIG_SPL_LOAD_FIT
MKIMAGEFLAGS_u-boot.img = -f auto -A $(ARCH) -T firmware -C none -O u-boot \
	-a $(CONFIG_TEXT_BASE) -e $(CONFIG_SYS_UBOOT_START) \
	-p $(CONFIG_FIT_EXTERNAL_OFFSET) \
	-n "U-Boot $(UBOOTRELEASE) for $(BOARD) board" -E \
	$(patsubst %,-b $(dt_dir)/%.dtb,$(subst ",,$(DEVICE_TREE))) \
	$(patsubst %,-b $(dt_dir)/%.dtb,$(subst ",,$(CONFIG_OF_LIST))) \
	$(patsubst %,-b $(dt_dir)/%.dtbo,$(subst ",,$(CONFIG_OF_OVERLAY_LIST)))
else
MKIMAGEFLAGS_u-boot.img = -A $(ARCH) -T firmware -C none -O u-boot \
	-a $(CONFIG_TEXT_BASE) -e $(CONFIG_SYS_UBOOT_START) \
	-n "U-Boot $(UBOOTRELEASE) for $(BOARD) board"
MKIMAGEFLAGS_u-boot-ivt.img = -A $(ARCH) -T firmware_ivt -C none -O u-boot \
	-a $(CONFIG_TEXT_BASE) -e $(CONFIG_SYS_UBOOT_START) \
	-n "U-Boot $(UBOOTRELEASE) for $(BOARD) board"
u-boot-ivt.img: MKIMAGEOUTPUT = u-boot-ivt.img.log
endif

MKIMAGEFLAGS_u-boot-dtb.img = $(MKIMAGEFLAGS_u-boot.img)

# Some boards have the kwbimage.cfg file written in advance, while some
# other boards generate it on the fly during the build in the build tree.
# Let's check if the file exists in the build tree first, otherwise we
# fall back to use the one in the source tree.
KWD_CONFIG_FILE = $(shell \
	if [ -f $(objtree)/$(CONFIG_SYS_KWD_CONFIG:"%"=%) ]; then \
		echo -n $(objtree)/$(CONFIG_SYS_KWD_CONFIG:"%"=%); \
	else \
		echo -n $(srctree)/$(CONFIG_SYS_KWD_CONFIG:"%"=%); \
	fi)

MKIMAGEFLAGS_u-boot.kwb = -n $(KWD_CONFIG_FILE) \
	-T kwbimage -a $(CONFIG_TEXT_BASE) -e $(CONFIG_TEXT_BASE)

MKIMAGEFLAGS_u-boot-with-spl.kwb = -n $(KWD_CONFIG_FILE) \
	-T kwbimage -a $(CONFIG_TEXT_BASE) -e $(CONFIG_TEXT_BASE) \
	$(if $(KEYDIR),-k $(KEYDIR))

MKIMAGEFLAGS_u-boot.pbl = -n $(srctree)/$(CONFIG_SYS_FSL_PBL_RCW:"%"=%) \
		-R $(srctree)/$(CONFIG_SYS_FSL_PBL_PBI:"%"=%) -A $(ARCH) -T pblimage

UBOOT_BIN := u-boot.bin

MKIMAGEFLAGS_u-boot-lzma.img = -A $(ARCH) -T standalone -C lzma -O u-boot \
	-a $(CONFIG_TEXT_BASE) -e $(CONFIG_SYS_UBOOT_START) \
	-n "U-Boot $(UBOOTRELEASE) for $(BOARD) board"

u-boot.bin.lzma: u-boot.bin FORCE
	$(call if_changed,lzma)

u-boot-lzma.img: u-boot.bin.lzma FORCE
	$(call if_changed,mkimage)

fit_image := $(if $(CONFIG_SANDBOX_VPL),u-boot,u-boot-nodtb.bin)

u-boot-dtb.img u-boot.img u-boot.kwb u-boot.pbl u-boot-ivt.img: \
		$(if $(CONFIG_SPL_LOAD_FIT),$(fit_image) \
			$(if $(CONFIG_OF_SEPARATE)$(CONFIG_OF_EMBED)$(CONFIG_SANDBOX),dts/dt.dtb) \
		,$(UBOOT_BIN)) FORCE
	$(call if_changed,mkimage)
	$(BOARD_SIZE_CHECK)

ifeq ($(CONFIG_SPL_LOAD_FIT_FULL),y)
MKIMAGEFLAGS_u-boot.itb =
else
MKIMAGEFLAGS_u-boot.itb = -E
endif
MKIMAGEFLAGS_u-boot.itb += -B 0x8

ifdef U_BOOT_ITS
u-boot.itb: u-boot-nodtb.bin \
		$(if $(CONFIG_OF_SEPARATE)$(CONFIG_OF_EMBED)$(CONFIG_SANDBOX),dts/dt.dtb) \
		$(if $(CONFIG_MULTI_DTB_FIT),$(FINAL_DTB_CONTAINER)) \
		$(U_BOOT_ITS) FORCE
	$(call if_changed,mkfitimage)
	$(BOARD_SIZE_CHECK)
endif

u-boot-with-spl.kwb: u-boot.bin spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)
	$(BOARD_SIZE_CHECK)

u-boot.dis:	u-boot
		$(OBJDUMP) -d $< > $@

ifneq ($(CONFIG_SPL_PAYLOAD),)
SPL_PAYLOAD := $(CONFIG_SPL_PAYLOAD:"%"=%)
else
SPL_PAYLOAD := u-boot.bin
endif

SPL_IMAGE := $(CONFIG_SPL_IMAGE:"%"=%)

OBJCOPYFLAGS_u-boot-with-spl.bin = -I binary -O binary \
				   --pad-to=$(CONFIG_SPL_PAD_TO)
u-boot-with-spl.bin: $(SPL_IMAGE) $(SPL_PAYLOAD) FORCE
	$(call if_changed,pad_cat)

ifeq ($(CONFIG_ARCH_LPC32XX)$(CONFIG_SPL),yy)
MKIMAGEFLAGS_lpc32xx-spl.img = -T lpc32xximage -a $(CONFIG_SPL_TEXT_BASE)

lpc32xx-spl.img: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

OBJCOPYFLAGS_lpc32xx-boot-0.bin = -I binary -O binary --pad-to=$(CONFIG_SPL_PAD_TO)

lpc32xx-boot-0.bin: lpc32xx-spl.img FORCE
	$(call if_changed,objcopy)

OBJCOPYFLAGS_lpc32xx-boot-1.bin = -I binary -O binary --pad-to=$(CONFIG_SPL_PAD_TO)

lpc32xx-boot-1.bin: lpc32xx-spl.img FORCE
	$(call if_changed,objcopy)

lpc32xx-full.bin: lpc32xx-boot-0.bin lpc32xx-boot-1.bin u-boot.img FORCE
	$(call if_changed,cat)

endif

OBJCOPYFLAGS_u-boot-with-tpl.bin = -I binary -O binary \
				   --pad-to=$(CONFIG_TPL_PAD_TO)
tpl/u-boot-with-tpl.bin: tpl/u-boot-tpl.bin u-boot.bin FORCE
	$(call if_changed,pad_cat)

SPL: spl/u-boot-spl.bin FORCE
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@

#ifeq ($(CONFIG_ARCH_IMX8M)$(CONFIG_ARCH_IMX8), y)
ifeq ($(CONFIG_SPL_LOAD_IMX_CONTAINER), y)
u-boot.cnt: u-boot.bin FORCE
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@

flash.bin: spl/u-boot-spl.bin u-boot.cnt FORCE
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@
else
ifeq ($(CONFIG_BINMAN),y)
flash.bin: spl/u-boot-spl.bin $(INPUTS-y) FORCE
	$(call if_changed,binman)
else
flash.bin: spl/u-boot-spl.bin u-boot.itb FORCE
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@
endif
endif
#endif

u-boot.uim: u-boot.bin FORCE
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@

u-boot-nand.imx: u-boot.imx FORCE
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@

u-boot-with-spl.imx u-boot-with-nand-spl.imx: SPL $(if $(CONFIG_OF_SEPARATE),u-boot.img,u-boot.uim) FORCE
	$(Q)$(MAKE) $(build)=arch/arm/mach-imx $@

MKIMAGEFLAGS_u-boot.ubl = -n $(UBL_CONFIG) -T ublimage -e $(CONFIG_TEXT_BASE)

u-boot.ubl: u-boot-with-spl.bin FORCE
	$(call if_changed,mkimage)

MKIMAGEFLAGS_u-boot-spl.ais = -s -n "/dev/null" \
	-T aisimage -e $(CONFIG_SPL_TEXT_BASE)
spl/u-boot-spl.ais: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

OBJCOPYFLAGS_u-boot.ais = -I binary -O binary --pad-to=$(CONFIG_SPL_PAD_TO)
u-boot.ais: spl/u-boot-spl.ais u-boot.img FORCE
	$(call if_changed,pad_cat)

u-boot-signed.sb: u-boot.bin spl/u-boot-spl.bin
	$(Q)$(MAKE) $(build)=arch/arm/cpu/arm926ejs/mxs u-boot-signed.sb
u-boot.sb: u-boot.bin spl/u-boot-spl.bin
	$(Q)$(MAKE) $(build)=arch/arm/cpu/arm926ejs/mxs u-boot.sb

MKIMAGEFLAGS_u-boot-spl.img = -A $(ARCH) -T firmware -C none \
	-a $(CONFIG_SPL_TEXT_BASE) -e $(CONFIG_SPL_TEXT_BASE) -n XLOADER
spl/u-boot-spl.img: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

ifneq ($(CONFIG_ARCH_SOCFPGA),)
quiet_cmd_gensplx4 = GENSPLX4 $@
cmd_gensplx4 = $(OBJCOPY) -I binary -O binary --gap-fill=0x0		\
			--pad-to=$(CONFIG_SPL_PAD_TO)			\
			spl/u-boot-spl.sfp spl/u-boot-spl.sfp &&        \
		cat	spl/u-boot-spl.sfp spl/u-boot-spl.sfp		\
			spl/u-boot-spl.sfp spl/u-boot-spl.sfp > $@ || { rm -f $@; false; }
spl/u-boot-splx4.sfp: spl/u-boot-spl.sfp FORCE
	$(call if_changed,gensplx4)

quiet_cmd_socboot = SOCBOOT $@
cmd_socboot = cat	spl/u-boot-splx4.sfp u-boot.img > $@ || { rm -f $@; false; }
u-boot-with-spl.sfp: spl/u-boot-splx4.sfp u-boot.img FORCE
	$(call if_changed,socboot)

quiet_cmd_gensplpadx4 = GENSPLPADX4 $@
cmd_gensplpadx4 =  dd if=/dev/zero of=spl/u-boot-spl.pad bs=64 count=1024 ; \
		   cat	spl/u-boot-spl.sfp spl/u-boot-spl.pad \
			spl/u-boot-spl.sfp spl/u-boot-spl.pad \
			spl/u-boot-spl.sfp spl/u-boot-spl.pad \
			spl/u-boot-spl.sfp spl/u-boot-spl.pad > $@ || \
			{ rm -f $@ spl/u-boot-spl.pad; false; }
u-boot-spl-padx4.sfp: spl/u-boot-spl.sfp FORCE
	$(call if_changed,gensplpadx4)

quiet_cmd_socnandboot = SOCNANDBOOT $@
cmd_socnandboot = cat	u-boot-spl-padx4.sfp u-boot.img > $@ || { rm -f $@; false; }
u-boot-with-nand-spl.sfp: u-boot-spl-padx4.sfp u-boot.img FORCE
	$(call if_changed,socnandboot)

endif

ifeq ($(CONFIG_MPC85XX_HAVE_RESET_VECTOR)$(CONFIG_OF_SEPARATE),yy)
u-boot-dtb.bin: u-boot-nodtb.bin u-boot.dtb u-boot-br.bin FORCE
	$(call if_changed,binman)

OBJCOPYFLAGS_u-boot-br.bin := -O binary -j .bootpg -j .resetvec
u-boot-br.bin: u-boot FORCE
	$(call if_changed,objcopy)
endif

quiet_cmd_ldr = LD      $@
cmd_ldr = $(LD) $(LDFLAGS_$(@F)) \
	       $(filter-out FORCE,$^) -o $@

ifdef CONFIG_X86
OBJCOPYFLAGS_u-boot-x86-start16.bin := -O binary -j .start16
u-boot-x86-start16.bin: u-boot FORCE
	$(call if_changed,objcopy)

OBJCOPYFLAGS_u-boot-x86-reset16.bin := -O binary -j .resetvec
u-boot-x86-reset16.bin: u-boot FORCE
	$(call if_changed,objcopy)

endif # CONFIG_X86

OBJCOPYFLAGS_u-boot-app.efi := $(OBJCOPYFLAGS_EFI)
u-boot-app.efi: u-boot FORCE
	$(call if_changed,zobjcopy)

u-boot.bin.o: u-boot.bin FORCE
	$(call if_changed,efipayload)

u-boot-payload.lds: $(LDSCRIPT_EFI) FORCE
	$(call if_changed_dep,cpp_lds)

# Rule to link the EFI payload which contains a stub and a U-Boot binary
quiet_cmd_u-boot_payload ?= LD      $@
      cmd_u-boot_payload ?= $(LD) $(LDFLAGS_EFI_PAYLOAD) -o $@ \
      -T u-boot-payload.lds arch/x86/cpu/call32.o \
      lib/efi_client/efi.o lib/efi_client/efi_stub.o u-boot.bin.o \
      $(addprefix arch/$(ARCH)/lib/,$(EFISTUB))

u-boot-payload: u-boot.bin.o u-boot-payload.lds FORCE
	$(call if_changed,u-boot_payload)

OBJCOPYFLAGS_u-boot-payload.efi := $(OBJCOPYFLAGS_EFI)
u-boot-payload.efi: u-boot-payload FORCE
	$(call if_changed,zobjcopy)

u-boot-img.bin: spl/u-boot-spl.bin u-boot.img FORCE
	$(call if_changed,cat)

#Add a target to create boot binary having SPL binary in PBI format
#concatenated with u-boot binary. It is need by PowerPC SoC having
#internal SRAM <= 512KB.
MKIMAGEFLAGS_u-boot-spl.pbl = -n $(srctree)/$(CONFIG_SYS_FSL_PBL_RCW:"%"=%) \
		-R $(srctree)/$(CONFIG_SYS_FSL_PBL_PBI:"%"=%) -T pblimage \
		-A $(ARCH) -a $(CONFIG_SPL_TEXT_BASE)

spl/u-boot-spl.pbl: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

ifeq ($(ARCH),arm)
UBOOT_BINLOAD := u-boot.img
else
UBOOT_BINLOAD := u-boot.bin
endif

OBJCOPYFLAGS_u-boot-with-spl-pbl.bin = -I binary -O binary --pad-to=$(CONFIG_SPL_PAD_TO) \
			  --gap-fill=0xff

u-boot-with-spl-pbl.bin: spl/u-boot-spl.pbl $(UBOOT_BINLOAD) FORCE
	$(call if_changed,pad_cat)

quiet_cmd_u-boot-elf ?= LD      $@
	cmd_u-boot-elf ?= $(LD) u-boot-elf.o -o $@ \
	$(if $(CONFIG_SYS_BIG_ENDIAN),-EB,-EL) \
	-T u-boot-elf.lds --defsym=$(CONFIG_PLATFORM_ELFENTRY)=$(CONFIG_TEXT_BASE) \
	-Ttext=$(CONFIG_TEXT_BASE)
u-boot.elf: u-boot.bin u-boot-elf.lds
	$(Q)$(OBJCOPY) -I binary $(PLATFORM_ELFFLAGS) $< u-boot-elf.o
	$(call if_changed,u-boot-elf)

u-boot-elf.lds: arch/u-boot-elf.lds prepare FORCE
	$(call if_changed_dep,cpp_lds)

PHONY += prepare0
# MediaTek's ARM-based u-boot needs a header to contains its load address
# which is parsed by the BootROM.
# If the SPL build is enabled, the header will be added to the spl binary,
# and the spl binary and the u-boot.img will be combined into one file.
# Otherwise the header will be added to the u-boot.bin directly.

ifeq ($(CONFIG_SPL),y)
spl/u-boot-spl-mtk.bin: spl/u-boot-spl

u-boot-mtk.bin: u-boot-with-spl.bin
	$(call if_changed,copy)
else
MKIMAGEFLAGS_u-boot-mtk.bin = -T mtk_image \
	-a $(CONFIG_TEXT_BASE) -e $(CONFIG_TEXT_BASE) \
	-n "$(patsubst "%",%,$(CONFIG_MTK_BROM_HEADER_INFO))"

u-boot-mtk.bin: u-boot.bin FORCE
	$(call if_changed,mkimage)
endif

quiet_cmd_endian_swap = SWAP    $@
      cmd_endian_swap = $(srctree)/tools/endian-swap.py $< $@

u-boot-swap.bin: u-boot.bin FORCE
	$(call if_changed,endian_swap)

ARCH_POSTLINK := $(wildcard $(srctree)/arch/$(ARCH)/Makefile.postlink)

# Generate linker list symbols references to force compiler to not optimize
# them away when compiling with LTO
ifeq ($(LTO_ENABLE),y)
u-boot-keep-syms-lto := keep-syms-lto.o
u-boot-keep-syms-lto_c := $(patsubst %.o,%.c,$(u-boot-keep-syms-lto))

quiet_cmd_keep_syms_lto = KSL     $@
      cmd_keep_syms_lto = \
	$(srctree)/scripts/gen_ll_addressable_symbols.sh $(NM) $^ > $@

quiet_cmd_keep_syms_lto_cc = KSLCC   $@
      cmd_keep_syms_lto_cc = \
	$(CC) $(filter-out $(LTO_CFLAGS),$(c_flags)) -c -o $@ $<

$(u-boot-keep-syms-lto_c): $(u-boot-main)
	$(call if_changed,keep_syms_lto)
$(u-boot-keep-syms-lto): $(u-boot-keep-syms-lto_c)
	$(call if_changed,keep_syms_lto_cc)
else
u-boot-keep-syms-lto :=
endif

# Rule to link u-boot
# May be overridden by arch/$(ARCH)/config.mk
ifeq ($(LTO_ENABLE),y)
quiet_cmd_u-boot__ ?= LTO     $@
      cmd_u-boot__ ?=								\
		touch $(u-boot-main) ;						\
		$(CC) -nostdlib -nostartfiles					\
		$(LTO_FINAL_LDFLAGS) $(c_flags)					\
		$(KBUILD_LDFLAGS:%=-Wl,%) $(LDFLAGS_u-boot:%=-Wl,%) -o $@	\
		-T u-boot.lds $(u-boot-init)					\
		-Wl,--whole-archive						\
			$(u-boot-main)						\
			$(u-boot-keep-syms-lto)					\
			$(PLATFORM_LIBS)					\
		-Wl,--no-whole-archive						\
		-Wl,-Map,u-boot.map;						\
		$(if $(ARCH_POSTLINK), $(MAKE) -f $(ARCH_POSTLINK) $@, true)
else
quiet_cmd_u-boot__ ?= LD      $@
      cmd_u-boot__ ?=								\
		touch $(u-boot-main) ;						\
		$(LD) $(KBUILD_LDFLAGS) $(LDFLAGS_u-boot) -o $@			\
		-T u-boot.lds $(u-boot-init)					\
		--whole-archive							\
			$(u-boot-main)						\
		--no-whole-archive						\
		$(PLATFORM_LIBS) -Map u-boot.map;				\
		$(if $(ARCH_POSTLINK), $(MAKE) -f $(ARCH_POSTLINK) $@, true)
endif

quiet_cmd_smap = GEN     common/system_map.o
cmd_smap = \
	smap=`$(call SYSTEM_MAP,u-boot) | \
		awk '$$2 ~ /[tTwW]/ {printf $$1 $$3 "\\\\000"}'` ; \
	$(CC) $(c_flags) -DSYSTEM_MAP="\"$${smap}\"" \
		-c $(srctree)/common/system_map.c -o common/system_map.o

u-boot:	$(u-boot-init) $(u-boot-main) $(u-boot-keep-syms-lto) u-boot.lds FORCE
	+$(call if_changed,u-boot__)
ifeq ($(CONFIG_KALLSYMS),y)
	$(call cmd,smap)
	$(call cmd,u-boot__) common/system_map.o
endif

ifeq ($(CONFIG_RISCV),y)
	@tools/prelink-riscv $@
endif

quiet_cmd_sym ?= SYM     $@
      cmd_sym ?= $(OBJDUMP) -t $< > $@
u-boot.sym: u-boot FORCE
	$(call if_changed,sym)

# Environment processing
# ---------------------------------------------------------------------------

# Directory where we expect the .env file, if it exists
ENV_DIR := $(srctree)/board/$(BOARDDIR)

# Basename of .env file, stripping quotes
ENV_SOURCE_FILE := $(CONFIG_ENV_SOURCE_FILE:"%"=%)

# Filename of .env file
ENV_FILE_CFG := $(ENV_DIR)/$(ENV_SOURCE_FILE).env

# Default filename, if CONFIG_ENV_SOURCE_FILE is empty
ENV_FILE_BOARD := $(ENV_DIR)/$(CONFIG_SYS_BOARD:"%"=%).env

# Select between the CONFIG_ENV_SOURCE_FILE and the default one
ENV_FILE := $(if $(ENV_SOURCE_FILE),$(ENV_FILE_CFG),$(wildcard $(ENV_FILE_BOARD)))

# Run the environment text file through the preprocessor, but only if it is
# non-empty, to save time and possible build errors if something is wonky with
# the board.
# If there is no ENV_FILE, produce an empty output file, to prevent a previous
# build's file being used in the case of in-tree builds.
quiet_cmd_gen_envp = ENVP    $@
      cmd_gen_envp = \
	if [ -s "$(ENV_FILE)" ]; then \
		$(CPP) -P $(cpp_flags) -x assembler-with-cpp -undef \
			-D__ASSEMBLY__ \
			-D__UBOOT_CONFIG__ \
			-DDEFAULT_DEVICE_TREE=$(subst ",,$(CONFIG_DEFAULT_DEVICE_TREE)) \
			-I . -I include -I $(srctree)/include \
			-include linux/kconfig.h -include include/config.h \
			-I$(srctree)/arch/$(ARCH)/include \
			$< -o $@; \
	else \
		rm -f $@; \
		touch $@ ; \
	fi
include/generated/env.in: include/generated/env.txt
	$(call cmd,gen_envp)

# Regenerate the environment if it changes
# We use 'wildcard' since the file is not required to exist (at present), in
# which case we don't want this dependency, but instead should create an empty
# file
# This rule is useful since it shows the source file for the environment
quiet_cmd_envc = ENVC    $@
      cmd_envc = \
	if [ -f "$<" ]; then \
		cat $< > $@; \
	elif [ -n "$(ENV_SOURCE_FILE)" ]; then \
		echo "Missing file $(ENV_FILE_CFG)"; \
	else \
		touch $@ ; \
	fi

include/generated/env.txt: $(wildcard $(ENV_FILE)) include/generated/autoconf.h
	$(call cmd,envc)

# Write out the resulting environment, converted to a C string
quiet_cmd_gen_envt = ENVT    $@
      cmd_gen_envt = \
	awk -f $(srctree)/scripts/env2string.awk $< >$@
$(env_h): include/generated/env.in
	$(call cmd,gen_envt)

# ---------------------------------------------------------------------------

# The actual objects are generated when descending,
# make sure no implicit rule kicks in
$(sort $(u-boot-init) $(u-boot-main)): $(u-boot-dirs) ;

# Handle descending into subdirectories listed in $(u-boot-dirs)
# Preset locale variables to speed up the build process. Limit locale
# tweaks to this spot to avoid wrong language settings when running
# make menuconfig etc.
# Error messages still appears in the original language

PHONY += $(u-boot-dirs)
$(u-boot-dirs): prepare
	$(Q)$(MAKE) $(build)=$@ need-builtin=1

tools: prepare
# The "tools" are needed early
$(filter-out tools, $(u-boot-dirs)): tools
# The "examples" conditionally depend on U-Boot (say, when USE_PRIVATE_LIBGCC
# is "yes"), so compile examples after U-Boot is compiled.
examples: $(filter-out examples, $(u-boot-dirs))

ifeq ($(CONFIG_USE_PRIVATE_LIBGCC),y)
# lib/efi_loader apps depend on arch/$(ARCH)/lib for lib.a
lib: $(filter arch/$(ARCH)/lib, $(u-boot-dirs))
endif

# The setlocalversion script comes from linux and expects a
# KERNELVERSION variable in the environment for figuring out which
# annotated tags are relevant. Pass UBOOTVERSION.
define filechk_uboot.release
	KERNELVERSION=$(UBOOTVERSION) $(CONFIG_SHELL) $(srctree)/scripts/setlocalversion $(srctree)
endef

# Store (new) UBOOTRELEASE string in include/config/uboot.release
include/config/uboot.release: include/config/auto.conf FORCE
	$(call filechk,uboot.release)
# Additional helpers built in scripts/
# Carefully list dependencies so we do not try to build scripts twice
# in parallel
PHONY += scripts
scripts: scripts_basic scripts_dtc
	$(Q)$(MAKE) $(build)=$(@)

# Things we need to do before we recursively start building the kernel
# or the modules are listed in "prepare".
# A multi level approach is used. prepareN is processed before prepareN-1.
# archprepare is used in arch Makefiles and when processed asm symlink,
# version.h and scripts_basic is processed / created.

PHONY += prepare archprepare prepare1 prepare3

# prepare3 is used to check if we are building in a separate output directory,
# and if so do:
# 1) Check that make has not been executed in the kernel src $(srctree)
prepare3: include/config/uboot.release
ifneq ($(KBUILD_SRC),)
	@$(kecho) '  Using $(srctree) as source for U-Boot'
	$(Q)if [ -f $(srctree)/.config -o -d $(srctree)/include/config ]; then \
		echo >&2 "  $(srctree) is not clean, please run 'make mrproper'"; \
		echo >&2 "  in the '$(srctree)' directory.";\
		/bin/false; \
	fi;
endif

prepare1: prepare3 outputmakefile cfg $(version_h) $(timestamp_h) $(dt_h) $(env_h) \
                   include/config/auto.conf
ifeq ($(wildcard $(LDSCRIPT)),)
	@echo >&2 "  Could not find linker script."
	@/bin/false
endif

ifeq ($(CONFIG_ENV_USE_DEFAULT_ENV_TEXT_FILE),y)
prepare1: $(defaultenv_h)

envtools: $(defaultenv_h)
endif
archprepare: prepare1 scripts

prepare0: archprepare
	$(Q)$(MAKE) $(build)=.

# All the preparing..
prepare: prepare0 prepare-objtool

# Support for using generic headers in asm-generic
asm-generic := -f $(srctree)/scripts/Makefile.asm-generic obj

PHONY += asm-generic uapi-asm-generic
asm-generic: uapi-asm-generic
	$(Q)$(MAKE) $(asm-generic)=arch/$(SRCARCH)/include/generated/asm \
	generic=include/asm-generic
uapi-asm-generic:
	$(Q)$(MAKE) $(asm-generic)=arch/$(SRCARCH)/include/generated/uapi/asm \
	generic=include/uapi/asm-generic

PHONY += prepare-objtool
prepare-objtool: $(objtool_target)
ifeq ($(SKIP_STACK_VALIDATION),1)
ifdef CONFIG_UNWINDER_ORC
	@echo "error: Cannot generate ORC metadata for CONFIG_UNWINDER_ORC=y, please install libelf-dev, libelf-devel or elfutils-libelf-devel" >&2
	@false
else
	@echo "warning: Cannot use CONFIG_STACK_VALIDATION=y, please install libelf-dev, libelf-devel or elfutils-libelf-devel" >&2
endif
endif

# Generate some files
# ---------------------------------------------------------------------------

# Use sed to remove leading zeros from PATCHLEVEL to avoid using octal numbers
define filechk_version.h
	(echo \#define PLAIN_VERSION \"$(UBOOTRELEASE)\"; \
	echo \#define U_BOOT_VERSION \"U-Boot \" PLAIN_VERSION; \
	echo \#define U_BOOT_VERSION_NUM $(VERSION); \
	echo \#define U_BOOT_VERSION_NUM_PATCH $$(echo $(PATCHLEVEL) | \
		sed -e "s/^0*//"); \
	echo \#define HOST_ARCH $(HOST_ARCH); \
	echo \#define CC_VERSION_STRING \"$$(LC_ALL=C $(CC) --version | head -n 1)\"; \
	echo \#define LD_VERSION_STRING \"$$(LC_ALL=C $(LD) --version | head -n 1)\"; )
endef

# The SOURCE_DATE_EPOCH mechanism requires a date that behaves like GNU date.
# The BSD date on the other hand behaves different and would produce errors
# with the misused '-d' switch.  Respect that and search a working date with
# well known pre- and suffixes for the GNU variant of date.
define filechk_timestamp.h
	(if test -n "$${SOURCE_DATE_EPOCH}"; then \
		SOURCE_DATE="@$${SOURCE_DATE_EPOCH}"; \
		DATE=""; \
		for date in gdate date.gnu date; do \
			$${date} -u -d "$${SOURCE_DATE}" >/dev/null 2>&1 && DATE="$${date}"; \
		done; \
		if test -n "$${DATE}"; then \
			LC_ALL=C $${DATE} -u -d "$${SOURCE_DATE}" +'#define U_BOOT_DATE "%b %d %C%y"'; \
			LC_ALL=C $${DATE} -u -d "$${SOURCE_DATE}" +'#define U_BOOT_TIME "%T"'; \
			LC_ALL=C $${DATE} -u -d "$${SOURCE_DATE}" +'#define U_BOOT_TZ "%z"'; \
			LC_ALL=C $${DATE} -u -d "$${SOURCE_DATE}" +'#define U_BOOT_EPOCH %s'; \
		else \
			return 42; \
		fi; \
	else \
		LC_ALL=C date +'#define U_BOOT_DATE "%b %d %C%y"'; \
		LC_ALL=C date +'#define U_BOOT_TIME "%T"'; \
		LC_ALL=C date +'#define U_BOOT_TZ "%z"'; \
		LC_ALL=C date +'#define U_BOOT_EPOCH %s'; \
	fi)
endef

define filechk_defaultenv.h
	( { grep -v '^#' | grep -v '^$$' || true ; echo '' ; } | \
	 tr '\n' '\0' | \
	 sed -e 's/\\\x0\s*//g' | \
	 xxd -i ; )
endef

define filechk_dt.h
	(if test -n "$${DEVICE_TREE}"; then \
		echo \#define DEVICE_TREE \"$(DEVICE_TREE)\"; \
	else \
		echo \#define DEVICE_TREE CONFIG_DEFAULT_DEVICE_TREE; \
	fi)
endef

$(version_h): include/config/uboot.release FORCE
	$(call filechk,version.h)

$(timestamp_h): $(srctree)/Makefile FORCE
	$(call filechk,timestamp.h)

$(dt_h): $(srctree)/Makefile FORCE
	$(call filechk,dt.h)

$(defaultenv_h): $(CONFIG_ENV_DEFAULT_ENV_TEXT_FILE:"%"=%) FORCE
	$(call filechk,defaultenv.h)

# ---------------------------------------------------------------------------
# Devicetree files

ifneq ($(wildcard $(srctree)/arch/$(SRCARCH)/boot/dts/),)
dtstree := arch/$(SRCARCH)/boot/dts
endif

ifneq ($(dtstree),)

%.dtb: prepare3 scripts_dtc
	$(Q)$(MAKE) $(build)=$(dtstree) $(dtstree)/$@

PHONY += dtbs dtbs_install dt_binding_check
dtbs dtbs_check: prepare3 scripts_dtc
	$(Q)$(MAKE) $(build)=$(dtstree)

dtbs_check: export CHECK_DTBS=1
dtbs_check: dt_binding_check

dtbs_install:
	$(Q)$(MAKE) $(dtbinst)=$(dtstree)

ifdef CONFIG_OF_EARLY_FLATTREE
all: dtbs
endif

endif

# Check dtc and pylibfdt, if DTC is provided, else build them
PHONY += scripts_dtc
scripts_dtc: scripts_basic
	$(Q)if test "$(DTC)" = "$(DTC_INTREE)"; then \
		$(MAKE) $(build)=scripts/dtc; \
	else \
		if ! $(DTC) -v >/dev/null; then \
			echo '*** Failed to check dtc version: $(DTC)'; \
			false; \
		else \
			if test "$(call dtc-version)" -lt $(DTC_MIN_VERSION); then \
				echo '*** Your dtc is too old, please upgrade to dtc $(DTC_MIN_VERSION) or newer'; \
				false; \
			else \
				if [ -n "$(CONFIG_PYLIBFDT)" ]; then \
					if ! echo "import libfdt" | $(PYTHON3) 2>/dev/null; then \
						echo '*** pylibfdt does not seem to be available with $(PYTHON3)'; \
						false; \
					fi; \
				fi; \
			fi; \
		fi; \
	fi

# ---------------------------------------------------------------------------
quiet_cmd_cpp_lds = LDS     $@
cmd_cpp_lds = $(CPP) -Wp,-MD,$(depfile) $(cpp_flags) $(LDPPFLAGS) \
		-D__ASSEMBLY__ -x assembler-with-cpp -std=c99 -P -o $@ $<

u-boot.lds: $(LDSCRIPT) prepare FORCE
	$(call if_changed_dep,cpp_lds)

spl/u-boot-spl.bin: spl/u-boot-spl
	@:
	$(SPL_SIZE_CHECK)

spl/u-boot-spl-dtb.bin: spl/u-boot-spl
	@:

spl/u-boot-spl-dtb.hex: spl/u-boot-spl
	@:

spl/u-boot-spl: tools prepare $(if $(CONFIG_SPL_OF_CONTROL),dts/dt.dtb)
	$(Q)$(MAKE) obj=spl -f $(srctree)/scripts/Makefile.xpl all

spl/sunxi-spl.bin: spl/u-boot-spl
	@:

spl/sunxi-spl-with-ecc.bin: spl/sunxi-spl.bin
	@:

spl/u-boot-spl.sfp: spl/u-boot-spl
	@:

spl/boot.bin: spl/u-boot-spl
	@:

tpl/u-boot-tpl.bin: tpl/u-boot-tpl
	@:
	$(TPL_SIZE_CHECK)

tpl/u-boot-tpl: tools prepare $(if $(CONFIG_TPL_OF_CONTROL),dts/dt.dtb)
	$(Q)$(MAKE) obj=tpl -f $(srctree)/scripts/Makefile.xpl all

vpl/u-boot-vpl.bin: vpl/u-boot-vpl
	@:
	$(VPL_SIZE_CHECK)

vpl/u-boot-vpl: tools prepare $(if $(CONFIG_TPL_OF_CONTROL),dts/dt.dtb)
	$(Q)$(MAKE) obj=vpl -f $(srctree)/scripts/Makefile.xpl all

TAG_SUBDIRS := $(patsubst %,$(srctree)/%,$(u-boot-dirs) include)

FIND := find
FINDFLAGS := -L

tags ctags:
		ctags -w -o ctags `$(FIND) $(FINDFLAGS) $(TAG_SUBDIRS) \
						-name '*.[chS]' -print`
		ln -s ctags tags

etags:
		etags -a -o etags `$(FIND) $(FINDFLAGS) $(TAG_SUBDIRS) \
						-name '*.[chS]' -print`
cscope:
		$(FIND) $(FINDFLAGS) $(TAG_SUBDIRS) -name '*.[chS]' -print > \
						cscope.files
		@find $(TAG_SUBDIRS) -name '*.[chS]' -type l -print | \
			grep -xvf - cscope.files > cscope.files.no-symlinks; \
		mv cscope.files.no-symlinks cscope.files
		cscope -b -q -k

SYSTEM_MAP = \
		$(NM) $1 | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		LC_ALL=C sort
System.map:	u-boot
		@$(call SYSTEM_MAP,$<) > $@

# ARM relocations should all be R_ARM_RELATIVE (32-bit) or
# R_AARCH64_RELATIVE (64-bit).
checkarmreloc: u-boot
	@RELOC="`$(READELF) -r -W $< | cut -d ' ' -f 4 | \
		grep R_A | sort -u`"; \
	if test "$$RELOC" != "R_ARM_RELATIVE" -a \
		 "$$RELOC" != "R_AARCH64_RELATIVE"; then \
		echo "$< contains unexpected relocations: $$RELOC"; \
		false; \
	fi

tools/version.h: include/version.h
	$(Q)mkdir -p $(dir $@)
	$(call if_changed,copy)

envtools: u-boot-initial-env scripts_basic $(version_h) $(timestamp_h) tools/version.h
	$(Q)$(MAKE) $(build)=tools/env

tools-only: export TOOLS_ONLY=y
tools-only: scripts_basic $(version_h) $(timestamp_h) tools/version.h
	$(Q)$(MAKE) $(build)=tools

tools-all: export HOST_TOOLS_ALL=y
tools-all: envtools tools ;

cross_tools: export CROSS_BUILD_TOOLS=y
cross_tools: tools ;

.PHONY : CHANGELOG
CHANGELOG:
	git log --no-merges U-Boot-1_1_5.. | \
	unexpand -a | sed -e 's/\s\s*$$//' > $@

#########################################################################

###
# Cleaning is done on three levels.
# make clean     Delete most generated files
#                Leave enough to build external modules
# make mrproper  Delete the current configuration, and all generated files
# make distclean Remove editor backup files, patch leftover files and the like

# Directories & files removed with 'make clean'
CLEAN_DIRS  += $(MODVERDIR) \
	       $(foreach d, spl tpl vpl, $(patsubst %,$d/%, \
			$(filter-out include, $(shell ls -1 $d 2>/dev/null))))

CLEAN_FILES += include/autoconf.mk* include/bmp_logo.h include/bmp_logo_data.h \
	       include/config.h include/generated/env.* drivers/video/u_boot_logo.S \
	       tools/version.h u-boot* MLO* SPL System.map fit-dtb.blob* \
	       u-boot-ivt.img.log u-boot-dtb.imx.log SPL.log u-boot.imx.log \
	       lpc32xx-* bl31.c bl31.elf bl31_*.bin image.map tispl.bin* \
	       idbloader.img flash.bin flash.log defconfig keep-syms-lto.c \
	       mkimage-out.spl.mkimage mkimage.spl.mkimage imx-boot.map \
	       itb.fit.fit itb.fit.itb itb.map spl.map mkimage-out.rom.mkimage \
	       mkimage.rom.mkimage mkimage-in-simple-bin* rom.map simple-bin* \
	       idbloader-spi.img lib/efi_loader/helloworld_efi.S *.itb \
	       Test* capsule*.*.efi-capsule capsule*.map mkimage.imx-boot.spl \
	       mkimage.imx-boot.u-boot mkimage-out.imx-boot.spl mkimage-out.imx-boot.u-boot

# Directories & files removed with 'make mrproper'
MRPROPER_DIRS  += include/config include/generated spl tpl vpl \
		  .tmp_objdiff doc/output include/asm

# Remove include/asm symlink created by U-Boot before v2014.01
MRPROPER_FILES += .config .config.old include/autoconf.mk* include/config.h \
		  ctags etags tags TAGS cscope* GPATH GTAGS GRTAGS GSYMS \
		  drivers/video/fonts/*.S include/asm *imx8mimage* *imx8mcst*

# clean - Delete most, but leave enough to build external modules
#
clean: rm-dirs  := $(CLEAN_DIRS)
clean: rm-files := $(CLEAN_FILES)
clean-dirs	:= $(foreach f,$(u-boot-alldirs),$(if $(wildcard $(srctree)/$f/Makefile),$f))
clean-dirs      := $(addprefix _clean_, $(clean-dirs))

PHONY += $(clean-dirs) clean archclean
$(clean-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _clean_%,%,$@)

clean: $(clean-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@find $(if $(KBUILD_EXTMOD), $(KBUILD_EXTMOD), .) $(RCS_FIND_IGNORE) \
		\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '*.ko.*' -o -name '*.su' -o -name '*.pyc' \
		-o -name '*.dtb' -o -name '*.dtbo' \
		-o -name '*.dtb.S' -o -name '*.dtbo.S' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.lex.c' -o -name '*.tab.[ch]' \
		-o -name '*.asn1.[ch]' \
		-o -name '*.symtypes' -o -name 'modules.order' \
		-o -name modules.builtin -o -name '.tmp_*.o.*' \
		-o -name 'dsdt_generated.aml' -o -name 'dsdt_generated.asl.tmp' \
		-o -name 'dsdt_generated.c' \
		-o -name 'generated_defconfig' \
		-o -name '*.efi' -o -name '*.gcno' -o -name '*.so' \) \
		-type f -print | xargs rm -f

# mrproper - Delete all generated files, including .config
#
mrproper: rm-dirs  := $(wildcard $(MRPROPER_DIRS))
mrproper: rm-files := $(wildcard $(MRPROPER_FILES))
mrproper-dirs      := $(addprefix _mrproper_,scripts)

PHONY += $(mrproper-dirs) mrproper
$(mrproper-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _mrproper_%,%,$@)

mrproper: clean $(mrproper-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@rm -f arch/*/include/asm/arch

# distclean
#
PHONY += distclean

distclean: mrproper
	@find $(srctree) $(RCS_FIND_IGNORE) \
		\( -name '*.orig' -o -name '*.rej' -o -name '*~' \
		-o -name '*.bak' -o -name '#*#' -o -name '.*.orig' \
		-o -name '.*.rej' -o -name '*%' -o -name 'core' \
		-o -name '*.pyc' \) \
		-type f -print | xargs rm -f
	@rm -f boards.cfg CHANGELOG .binman_stamp

# See doc/develop/python_cq.rst
PHONY += pylint pylint_err
PYLINT_BASE := scripts/pylint.base
PYLINT_CUR := pylint.cur
PYLINT_DIFF := pylint.diff
pylint:
	$(Q)echo "Running pylint on all files (summary in $(PYLINT_CUR); output in pylint.out/)"
	$(Q)mkdir -p pylint.out
	$(Q)rm -f pylint.out/out*
	$(Q)find tools test -name "*.py" \
		| xargs -n1 -P$(shell nproc 2>/dev/null || echo 1) \
			sh -c 'pylint --reports=y --exit-zero -f parseable --ignore-imports=yes $$@ > pylint.out/$$(echo $$@ | tr / _ | sed s/.py//)' _
	$(Q)rm -f $(PYLINT_CUR)
	$(Q)( cd pylint.out; for f in *; do \
		sed -ne "s/Your code has been rated at \([-0-9.]*\).*/$$f \1/p" $$f; \
	done ) | sort > $(PYLINT_CUR)
	$(Q)base=$$(mktemp) cur=$$(mktemp); cut -d' ' -f1 $(PYLINT_BASE) >$$base; \
		cut -d' ' -f1 $(PYLINT_CUR) >$$cur; \
		comm -3 $$base $$cur > $(PYLINT_DIFF); \
		if [ -s $(PYLINT_DIFF) ]; then \
			echo "Files have been added/removed. Try:\n\tcp $(PYLINT_CUR) $(PYLINT_BASE)"; \
			echo; \
			echo "Added files:"; \
			comm -13 $$base $$cur; \
			echo; \
			echo "Removed files:"; \
			comm -23 $$base $$cur; \
			false; \
		else \
			rm $$base $$cur $(PYLINT_DIFF); \
		fi
	$(Q)bad=false; while read base_file base_val <&3 && read cur_file cur_val <&4; do \
		if awk "BEGIN {exit !($$cur_val < $$base_val)}"; then \
			echo "$$base_file: Score was $$base_val, now $$cur_val"; \
			bad=true; fi; \
		done 3<$(PYLINT_BASE) 4<$(PYLINT_CUR); \
		if $$bad; then \
			echo "Some files have regressed, please fix"; \
			false; \
		else \
			echo "No pylint regressions"; \
		fi

# Check for errors only
pylint_err:
	$(Q)pylint -E  -j 0 --ignore-imports=yes \
		$(shell find tools test -name "*.py")

backup:
	F=`basename $(srctree)` ; cd .. ; \
	gtar --force-local -zcvf `LC_ALL=C date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

PHONY += _pip pip pip_release

pip_release: PIP_ARGS="--real"
pip_test: PIP_ARGS=""
pip: PIP_ARGS="-n"

pip pip_test pip_release: _pip

_pip:
	scripts/make_pip.sh u_boot_pylib ${PIP_ARGS}
	scripts/make_pip.sh patman ${PIP_ARGS}
	scripts/make_pip.sh buildman ${PIP_ARGS}
	scripts/make_pip.sh dtoc ${PIP_ARGS}
	scripts/make_pip.sh binman ${PIP_ARGS}

help:
	@echo  'Cleaning targets:'
	@echo  '  clean		  - Remove most generated files but keep the config'
	@echo  '  mrproper	  - Remove all generated files + config + various backup files'
	@echo  '  distclean	  - mrproper + remove editor backup and patch files'
	@echo  ''
	@echo  'Configuration targets:'
	@$(MAKE) -f $(srctree)/scripts/kconfig/Makefile help
	@echo  ''
	@echo  'Test targets:'
	@echo  ''
	@echo  '  check           - Run all automated tests that use sandbox'
	@echo  '  pcheck          - Run quick automated tests in parallel'
	@echo  '  qcheck          - Run quick automated tests that use sandbox'
	@echo  '  tcheck          - Run quick automated tests on tools'
	@echo  '  pylint          - Run pylint on all Python files'
	@echo  ''
	@echo  'Other generic targets:'
	@echo  '  all		  - Build all necessary images depending on configuration'
	@echo  '  tests		  - Build U-Boot for sandbox and run tests'
	@echo  '* u-boot	  - Build the bare u-boot'
	@echo  '  dir/            - Build all files in dir and below'
	@echo  '  dir/file.[oisS] - Build specified target only'
	@echo  '  dir/file.lst    - Build specified mixed source/assembly target only'
	@echo  '                    (requires a recent binutils and recent build (System.map))'
	@echo  '  tags/ctags	  - Generate ctags file for editors'
	@echo  '  etags		  - Generate etags file for editors'
	@echo  '  cscope	  - Generate cscope index'
	@echo  '  ubootrelease	  - Output the release version string (use with make -s)'
	@echo  '  ubootversion	  - Output the version stored in Makefile (use with make -s)'
	@echo  "  cfg		  - Don't build, just create the .cfg files"
	@echo  "  envtools	  - Build only the target-side environment tools"
	@echo  ''
	@echo  'PyPi / pip targets:'
	@echo  '  pip             - Check building of PyPi packages'
	@echo  '  pip_test        - Build PyPi pakages and upload to test server'
	@echo  '  pip_release     - Build PyPi pakages and upload to release server'
	@echo  ''
	@echo  'Static analysers'
	@echo  '  checkstack      - Generate a list of stack hogs'
	@echo  '  coccicheck      - Execute static code analysis with Coccinelle'
	@echo  ''
	@echo  'Documentation targets:'
	@$(MAKE) -f $(srctree)/doc/Makefile dochelp
	@echo  ''
	@echo  '  make V=0|1 [targets] 0 => quiet build (default), 1 => verbose build'
	@echo  '  make V=2   [targets] 2 => give reason for rebuild of target'
	@echo  '  make O=dir [targets] Locate all output files in "dir", including .config'
	@echo  '  make C=1   [targets] Check all c source with $$CHECK (sparse by default)'
	@echo  '  make C=2   [targets] Force check of all c source with $$CHECK'
	@echo  '  make RECORDMCOUNT_WARN=1 [targets] Warn about ignored mcount sections'
	@echo  '  make W=n   [targets] Enable extra gcc checks, n=1,2,3 where'
	@echo  '		1: warnings which may be relevant and do not occur too often'
	@echo  '		2: warnings which occur quite often but may still be relevant'
	@echo  '		3: more obscure warnings, can most likely be ignored'
	@echo  '		Multiple levels can be combined with W=12 or W=123'
	@echo  ''
	@echo  'Execute "make" or "make all" to build all targets marked with [*] '
	@echo  'For further info see the ./README file'

tests check:
	$(srctree)/test/run

pcheck:
	$(srctree)/test/run parallel

qcheck:
	$(srctree)/test/run quick

tcheck:
	$(srctree)/test/run tools

# Documentation targets
# ---------------------------------------------------------------------------
DOC_TARGETS := xmldocs latexdocs pdfdocs htmldocs epubdocs cleandocs \
	       linkcheckdocs dochelp refcheckdocs texinfodocs infodocs
PHONY += $(DOC_TARGETS)
$(DOC_TARGETS): scripts_basic FORCE
	$(Q)PYTHONPATH=$(srctree)/test/py/tests:$(srctree)/test/py \
	$(MAKE) $(build)=doc $@

PHONY += checkstack ubootrelease ubootversion

checkstack:
	$(OBJDUMP) -d u-boot $$(find . -name u-boot-spl) | \
	$(PERL) $(src)/scripts/checkstack.pl $(ARCH)

ubootversion:
	@echo $(UBOOTVERSION)

ubootrelease:
	@$(filechk_uboot.release)

# Clear a bunch of variables before executing the submake

ifeq ($(quiet),silent_)
tools_silent=s
endif

tools/: FORCE
	$(Q)mkdir -p $(objtree)/tools
	$(Q)$(MAKE) LDFLAGS= MAKEFLAGS="$(tools_silent) $(filter --j% -j,$(MAKEFLAGS))" O=$(abspath $(objtree)) subdir=tools -C $(src)/tools/

tools/%: FORCE
	$(Q)mkdir -p $(objtree)/tools
	$(Q)$(MAKE) LDFLAGS= MAKEFLAGS="$(tools_silent) $(filter --j% -j,$(MAKEFLAGS))" O=$(abspath $(objtree)) subdir=tools -C $(src)/tools/ $*

# Single targets
# ---------------------------------------------------------------------------
# Single targets are compatible with:
# - build with mixed source and output
# - build with separate output dir 'make O=...'
# - external modules
#
#  target-dir => where to store outputfile
#  build-dir  => directory in kernel source tree to use

build-target = $(if $(KBUILD_EXTMOD), $(KBUILD_EXTMOD)/)$@
build-dir = $(patsubst %/,%,$(dir $(build-target)))

%.i: prepare FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(build-target)
%.ll: prepare FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(build-target)
%.lst: prepare FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(build-target)
%.o: prepare FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(build-target)
%.s: prepare FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(build-target)
%.symtypes: prepare FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(build-target)
%.ko: %.o
	$(Q)$(MAKE) -f $(srctree)/scripts/Makefile.modpost

# Modules
PHONY += /
/: ./

# Make sure the latest headers are built for Documentation
Documentation/ samples/: headers_install
%/: prepare FORCE
	$(Q)$(MAKE) KBUILD_MODULES=1 $(build)=$(build-dir)

quiet_cmd_genenv = GENENV  $@
cmd_genenv = \
	$(objtree)/tools/printinitialenv | \
	sed -e '/^\s*$$/d' | \
	sort -t '=' -k 1,1 -s -o $@

u-boot-initial-env: scripts_basic $(version_h) $(env_h) include/config.h FORCE
	$(Q)$(MAKE) $(build)=tools $(objtree)/tools/printinitialenv
	$(call if_changed,genenv)

# Consistency checks
# ---------------------------------------------------------------------------

PHONY += coccicheck

coccicheck:
	$(Q)$(CONFIG_SHELL) $(srctree)/scripts/$@

# FIXME Should go into a make.lib or something
# ===========================================================================

quiet_cmd_rmdirs = $(if $(wildcard $(rm-dirs)),CLEAN   $(wildcard $(rm-dirs)))
      cmd_rmdirs = rm -rf $(rm-dirs)

quiet_cmd_rmfiles = $(if $(wildcard $(rm-files)),CLEAN   $(wildcard $(rm-files)))
      cmd_rmfiles = rm -f $(rm-files)

# Run depmod only if we have System.map and depmod is executable
quiet_cmd_depmod = DEPMOD  $(KERNELRELEASE)
      cmd_depmod = $(CONFIG_SHELL) $(srctree)/scripts/depmod.sh $(DEPMOD) \
                   $(KERNELRELEASE)

# Create temporary dir for module support files
# clean it up only when building all modules
cmd_crmodverdir = $(Q)mkdir -p $(MODVERDIR) \
                  $(if $(KBUILD_MODULES),; rm -f $(MODVERDIR)/*)

# read saved command lines for existing targets
existing-targets := $(wildcard $(sort $(targets)))

-include $(foreach f,$(existing-targets),$(dir $(f)).$(notdir $(f)).cmd)

endif   # ifeq ($(config-targets),1)
endif   # ifeq ($(mixed-targets),1)
endif   # need-sub-make

PHONY += FORCE
FORCE:

# Declare the contents of the PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
