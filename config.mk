#
# (C) Copyright 2000-2013
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
#########################################################################

ifeq ($(CURDIR),$(SRCTREE))
dir :=
else
dir := $(subst $(SRCTREE)/,,$(CURDIR))
endif

ifneq ($(OBJTREE),$(SRCTREE))
# Create object files for SPL in a separate directory
ifeq ($(CONFIG_SPL_BUILD),y)
ifeq ($(CONFIG_TPL_BUILD),y)
obj := $(if $(dir),$(TPLTREE)/$(dir)/,$(TPLTREE)/)
else
obj := $(if $(dir),$(SPLTREE)/$(dir)/,$(SPLTREE)/)
endif
else
obj := $(if $(dir),$(OBJTREE)/$(dir)/,$(OBJTREE)/)
endif
src := $(if $(dir),$(SRCTREE)/$(dir)/,$(SRCTREE)/)

$(shell mkdir -p $(obj))
else
# Create object files for SPL in a separate directory
ifeq ($(CONFIG_SPL_BUILD),y)
ifeq ($(CONFIG_TPL_BUILD),y)
obj := $(if $(dir),$(TPLTREE)/$(dir)/,$(TPLTREE)/)
else
obj := $(if $(dir),$(SPLTREE)/$(dir)/,$(SPLTREE)/)

endif
$(shell mkdir -p $(obj))
else
obj :=
endif
src :=
endif

# clean the slate ...
PLATFORM_RELFLAGS =
PLATFORM_CPPFLAGS =
PLATFORM_LDFLAGS =

#########################################################################

# Load generated board configuration
ifeq ($(CONFIG_TPL_BUILD),y)
# Include TPL autoconf
sinclude $(OBJTREE)/include/tpl-autoconf.mk
else
ifeq ($(CONFIG_SPL_BUILD),y)
# Include SPL autoconf
sinclude $(OBJTREE)/include/spl-autoconf.mk
else
# Include normal autoconf
sinclude $(OBJTREE)/include/autoconf.mk
endif
endif
sinclude $(OBJTREE)/include/config.mk

# Some architecture config.mk files need to know what CPUDIR is set to,
# so calculate CPUDIR before including ARCH/SOC/CPU config.mk files.
# Check if arch/$ARCH/cpu/$CPU exists, otherwise assume arch/$ARCH/cpu contains
# CPU-specific code.
CPUDIR=arch/$(ARCH)/cpu/$(CPU)
ifneq ($(SRCTREE)/$(CPUDIR),$(wildcard $(SRCTREE)/$(CPUDIR)))
CPUDIR=arch/$(ARCH)/cpu
endif

sinclude $(TOPDIR)/arch/$(ARCH)/config.mk	# include architecture dependend rules
sinclude $(TOPDIR)/$(CPUDIR)/config.mk		# include  CPU	specific rules

ifdef	SOC
sinclude $(TOPDIR)/$(CPUDIR)/$(SOC)/config.mk	# include  SoC	specific rules
endif
ifdef	VENDOR
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
ifdef	BOARD
sinclude $(TOPDIR)/board/$(BOARDDIR)/config.mk	# include board specific rules
endif

#########################################################################

RELFLAGS= $(PLATFORM_RELFLAGS)

OBJCFLAGS += --gap-fill=0xff

gccincdir := $(shell $(CC) -print-file-name=include)

CPPFLAGS = $(KBUILD_CPPFLAGS) $(RELFLAGS)

# Enable garbage collection of un-used sections for SPL
ifeq ($(CONFIG_SPL_BUILD),y)
CPPFLAGS += -ffunction-sections -fdata-sections
LDFLAGS_FINAL += --gc-sections
endif

ifneq ($(CONFIG_SYS_TEXT_BASE),)
CPPFLAGS += -DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE)
endif

ifeq ($(CONFIG_SPL_BUILD),y)
CPPFLAGS += -DCONFIG_SPL_BUILD
ifeq ($(CONFIG_TPL_BUILD),y)
CPPFLAGS += -DCONFIG_TPL_BUILD
endif
endif

# Does this architecture support generic board init?
ifeq ($(__HAVE_ARCH_GENERIC_BOARD),)
ifneq ($(CONFIG_SYS_GENERIC_BOARD),)
CHECK_GENERIC_BOARD = $(error Your architecture does not support generic board. \
Please undefined CONFIG_SYS_GENERIC_BOARD in your board config file)
endif
endif

ifneq ($(OBJTREE),$(SRCTREE))
CPPFLAGS += -I$(OBJTREE)/include
endif

CPPFLAGS += -I$(TOPDIR)/include -I$(SRCTREE)/arch/$(ARCH)/include
CPPFLAGS += -nostdinc	\
	-isystem $(gccincdir) -pipe $(PLATFORM_CPPFLAGS)

CFLAGS := $(KBUILD_CFLAGS) $(CPPFLAGS)

BCURDIR = $(subst $(SRCTREE)/,,$(CURDIR:$(obj)%=%))

ifeq ($(findstring examples/,$(BCURDIR)),)
ifeq ($(CONFIG_SPL_BUILD),)
ifdef FTRACE
CFLAGS += -finstrument-functions -DFTRACE
endif
endif
endif

AFLAGS := $(KBUILD_AFLAGS) $(CPPFLAGS)

LDFLAGS += $(PLATFORM_LDFLAGS)
LDFLAGS_FINAL += -Bstatic

LDFLAGS_u-boot += -T $(obj)u-boot.lds $(LDFLAGS_FINAL)
ifneq ($(CONFIG_SYS_TEXT_BASE),)
LDFLAGS_u-boot += -Ttext $(CONFIG_SYS_TEXT_BASE)
endif

LDFLAGS_$(SPL_BIN) += -T $(obj)u-boot-spl.lds $(LDFLAGS_FINAL)
ifneq ($(CONFIG_SPL_TEXT_BASE),)
LDFLAGS_$(SPL_BIN) += -Ttext $(CONFIG_SPL_TEXT_BASE)
endif

#########################################################################

export	CONFIG_SYS_TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS
