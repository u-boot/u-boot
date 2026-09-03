# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	:= vexpress64.o

ifeq ($(CONFIG_BLOBLIST_PASSAGE_MANDATORY),)
obj-$(CONFIG_OF_HAS_PRIOR_STAGE)	+= lowlevel_init.o
endif

obj-$(CONFIG_TARGET_VEXPRESS64_JUNO)	+= pcie.o
