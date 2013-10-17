#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2003
# Pavel Bartusek, Sysgo Real-Time Solutions AG, pba@sysgo.de
#
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_CMD_UBIFS) := ubifs.o io.o super.o sb.o master.o lpt.o
obj-$(CONFIG_CMD_UBIFS) += lpt_commit.o scan.o lprops.o
obj-$(CONFIG_CMD_UBIFS) += tnc.o tnc_misc.o debug.o crc16.o budget.o
obj-$(CONFIG_CMD_UBIFS) += log.o orphan.o recovery.o replay.o

# SEE README.arm-unaligned-accesses
$(obj)super.o: CFLAGS += $(PLATFORM_NO_UNALIGNED)
