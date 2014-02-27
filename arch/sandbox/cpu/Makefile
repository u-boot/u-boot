#
# Copyright (c) 2011 The Chromium OS Authors.
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= cpu.o os.o start.o state.o
obj-$(CONFIG_SANDBOX_SDL)	+= sdl.o

# os.c is build in the system environment, so needs standard includes
# CFLAGS_REMOVE_os.o cannot be used to drop header include path
quiet_cmd_cc_os.o = CC $(quiet_modtag)  $@
cmd_cc_os.o = $(CC) $(filter-out -nostdinc, \
	$(patsubst -I%,-idirafter%,$(c_flags))) -c -o $@ $<

$(obj)/os.o: $(src)/os.c FORCE
	$(call if_changed_dep,cc_os.o)
$(obj)/sdl.o: $(src)/sdl.c FORCE
	$(call if_changed_dep,cc_os.o)
