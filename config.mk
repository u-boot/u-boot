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

CPPFLAGS = $(KBUILD_CPPFLAGS) $(RELFLAGS)
CPPFLAGS += $(UBOOTINCLUDE)
CPPFLAGS += $(NOSTDINC_FLAGS) -pipe $(PLATFORM_CPPFLAGS)

CFLAGS := $(KBUILD_CFLAGS) $(CPPFLAGS)

BCURDIR = $(subst $(SRCTREE)/,,$(CURDIR:$(obj)%=%))

AFLAGS := $(KBUILD_AFLAGS) $(CPPFLAGS)

LDFLAGS += $(PLATFORM_LDFLAGS)
LDFLAGS_FINAL += -Bstatic

#########################################################################

export PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS
