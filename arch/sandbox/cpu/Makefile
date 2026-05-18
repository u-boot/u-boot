# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	:= cache.o cpu.o state.o initjmp.o os.o
extra-y	:= start.o
extra-$(CONFIG_SANDBOX_SDL)    += sdl.o
obj-$(CONFIG_XPL_BUILD)	+= spl.o
obj-$(CONFIG_ETH_SANDBOX_RAW)	+= eth-raw-os.o

# These files need to be built with system headers, since they use system
# calls or system-level interfaces. Generate a custom compile rule for each
# one that drops -nostdinc and converts -I to -idirafter.
CFLAGS_USE_SYSHDRS := eth-raw-os.o initjmp.o os.o sdl.o

define syshdrs_rule
quiet_cmd_cc_$(1) = CC $$(quiet_modtag)  $$@
cmd_cc_$(1) = $$(CC) $$(filter-out -nostdinc, \
	$$(patsubst -I%,-idirafter%,$$(c_flags))) -c -o $$@ $$<

$$(obj)/$(1): $$(src)/$(1:.o=.c) FORCE
	$$(call if_changed_dep,cc_$(1))
endef

$(foreach f,$(CFLAGS_USE_SYSHDRS),$(eval $(call syshdrs_rule,$(f))))

# sdl.c also needs -fshort-wchar removed (musl) and -fno-lto, so override
# the generated rule
cmd_cc_sdl.o = $(CC) $(filter-out -nostdinc -fshort-wchar, \
	$(patsubst -I%,-idirafter%,$(c_flags))) -fno-lto -c -o $@ $<
