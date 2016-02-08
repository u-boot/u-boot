#
# U-Boot - Makefile
#
# Copyright (c) 2005-2008 Analog Device Inc.
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
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
clean-files := init.lds

# make sure our initcode (which goes into LDR) does not
# have relocs or external references
CFLAGS_REMOVE_initcode.o := -ffunction-sections -fdata-sections
READINIT = env LC_ALL=C $(CROSS_COMPILE)readelf -s $<
$(obj)/check_initcode: $(obj)/initcode.o
ifneq ($(CONFIG_BFIN_BOOT_MODE),BFIN_BOOT_BYPASS)
	@if $(READINIT) | grep '\<GLOBAL\>.*\<UND\>' ; then \
		echo "$< contains external references!" 1>&2 ; \
		exit 1 ; \
	fi
endif

CPPFLAGS_init.lds := -ansi

quiet_cmd_link_init = LD      $@
      cmd_link_init = $(LD) $(LDFLAGS) -T $^ -o $@
$(obj)/init.elf: $(obj)/init.lds $(obj)/init.o $(obj)/initcode.o
	$(call if_changed,link_init)
targets += init.lds init.o
