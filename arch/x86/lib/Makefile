#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-y	+= cmd_boot.o
obj-y	+= gcc.o
obj-y	+= init_helpers.o
obj-y	+= interrupts.o
obj-$(CONFIG_SYS_PCAT_INTERRUPTS) += pcat_interrupts.o
obj-$(CONFIG_SYS_PCAT_TIMER) += pcat_timer.o
obj-$(CONFIG_PCI) += pci_type1.o
obj-y	+= relocate.o
obj-y += physmem.o
obj-y	+= string.o
obj-$(CONFIG_SYS_X86_TSC_TIMER)	+= tsc_timer.o
obj-$(CONFIG_VIDEO_VGA)	+= video.o
obj-$(CONFIG_CMD_ZBOOT)	+= zimage.o

LIBGCC := $(notdir $(NORMAL_LIBGCC))
extra-y := $(LIBGCC)

OBJCOPYFLAGS := --prefix-symbols=__normal_
$(obj)/$(LIBGCC): $(NORMAL_LIBGCC) FORCE
	$(call if_changed,objcopy)
