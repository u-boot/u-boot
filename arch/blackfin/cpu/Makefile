#
# U-boot - Makefile
#
# Copyright (c) 2005-2008 Analog Device Inc.
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Licensed under the GPL-2 or later.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

EXTRA    := init.elf
CEXTRA   := initcode.o
SEXTRA   := start.o
SOBJS    := interrupt.o cache.o
COBJS-y  += cpu.o
COBJS-y  += gpio.o
COBJS-y  += interrupts.o
COBJS-$(CONFIG_JTAG_CONSOLE) += jtag-console.o
COBJS-y  += os_log.o
COBJS-y  += reset.o
COBJS-y  += serial.o
COBJS-y  += traps.o
COBJS-$(CONFIG_HW_WATCHDOG)  += watchdog.o

SRCS     := $(SEXTRA:.o=.S) $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS     := $(addprefix $(obj),$(COBJS-y) $(SOBJS))
EXTRA    := $(addprefix $(obj),$(EXTRA))
CEXTRA   := $(addprefix $(obj),$(CEXTRA))
SEXTRA   := $(addprefix $(obj),$(SEXTRA))

all:	$(obj).depend $(LIB) $(obj).depend $(EXTRA) $(CEXTRA) $(SEXTRA) check_initcode

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(OBJS): $(obj)bootrom-asm-offsets.h
$(obj)bootrom-asm-offsets.c: bootrom-asm-offsets.c.in bootrom-asm-offsets.awk
	echo '#include <asm/mach-common/bits/bootrom.h>' | $(CPP) $(CPPFLAGS) - | gawk -f ./bootrom-asm-offsets.awk > $@.tmp
	mv $@.tmp $@
$(obj)bootrom-asm-offsets.s: $(obj)bootrom-asm-offsets.c
	$(CC) $(CFLAGS) -S $^ -o $@.tmp
	mv $@.tmp $@
$(obj)bootrom-asm-offsets.h: $(obj)bootrom-asm-offsets.s
	sed -ne "/^->/{s:^->\([^ ]*\) [\$$#]*\([^ ]*\) \(.*\):#define \1 \2 /* \3 */:; s:->::; p;}" $^ > $@

# make sure our initcode (which goes into LDR) does not
# have relocs or external references
$(obj)initcode.o: CFLAGS += -fno-function-sections -fno-data-sections
READINIT = env LC_ALL=C $(CROSS_COMPILE)readelf -s $<
check_initcode: $(obj)initcode.o
ifneq ($(CONFIG_BFIN_BOOT_MODE),BFIN_BOOT_BYPASS)
	@if $(READINIT) | grep '\<GLOBAL\>.*\<UND\>' ; then \
		echo "$< contains external references!" 1>&2 ; \
		exit 1 ; \
	fi
endif

$(obj)init.lds: init.lds.S
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P $^ -o $@
$(obj)init.elf: $(obj)init.lds $(obj)init.o $(obj)initcode.o
	$(LD) $(LDFLAGS) -T $^ -o $@

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
