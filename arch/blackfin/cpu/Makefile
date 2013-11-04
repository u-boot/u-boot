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

extra-y := init.elf
extra-y += initcode.o
extra-y += start.o
obj-y    := interrupt.o cache.o
obj-y  += cpu.o
obj-y += gpio.o
obj-y  += interrupts.o
obj-$(CONFIG_JTAG_CONSOLE) += jtag-console.o
obj-y  += os_log.o
obj-y  += reset.o
obj-y  += traps.o

extra-y += check_initcode

extra-y += bootrom-asm-offsets.h
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
$(obj)check_initcode: $(obj)initcode.o
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
