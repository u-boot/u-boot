# Copyright (c) 2011 The Chromium OS Authors.
# SPDX-License-Identifier:	GPL-2.0+

PLATFORM_CPPFLAGS += -D__SANDBOX__ -U_FORTIFY_SOURCE
PLATFORM_CPPFLAGS += -DCONFIG_ARCH_MAP_SYSMEM
PLATFORM_LIBS += -lrt

# Define this to avoid linking with SDL, which requires SDL libraries
# This can solve 'sdl-config: Command not found' errors
ifneq ($(NO_SDL),)
PLATFORM_CPPFLAGS += -DSANDBOX_NO_SDL
else
ifdef CONFIG_SANDBOX_SDL
PLATFORM_LIBS += $(shell sdl-config --libs)
PLATFORM_CPPFLAGS += $(shell sdl-config --cflags)
endif
endif

cmd_u-boot__ = $(CC) -o $@ -T u-boot.lds \
	-Wl,--start-group $(u-boot-main) -Wl,--end-group \
	$(PLATFORM_LIBS) -Wl,-Map -Wl,u-boot.map

CONFIG_ARCH_DEVICE_TREE := sandbox
