#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

HOSTSRCS := $(SRCTREE)/lib/crc32.c  fw_env.c  fw_env_main.c
HOSTSRCS += $(SRCTREE)/lib/ctype.c $(SRCTREE)/lib/linux_string.c
HOSTSRCS += $(SRCTREE)/common/env_attr.c $(SRCTREE)/common/env_flags.c
HEADERS	:= fw_env.h $(OBJTREE)/include/config.h

# Compile for a hosted environment on the target
HOSTCPPFLAGS  = -idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
		-idirafter $(SRCTREE)/tools/env \
		-DUSE_HOSTCC \
		-DTEXT_BASE=$(TEXT_BASE)

ifeq ($(MTD_VERSION),old)
HOSTCPPFLAGS += -DMTD_OLD
endif

all:	$(obj)fw_printenv

# Some files complain if compiled with -pedantic, use HOSTCFLAGS_NOPED
$(obj)fw_printenv:	$(HOSTSRCS) $(HEADERS)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $(HOSTSRCS)
	$(HOSTSTRIP) $@

clean:
	rm -f $(obj)fw_printenv

#########################################################################

include $(TOPDIR)/rules.mk

sinclude $(obj).depend

#########################################################################
