# Copyright (c) 2011 The Chromium OS Authors.
# SPDX-License-Identifier:	GPL-2.0+

PLATFORM_CPPFLAGS += -DCONFIG_SANDBOX -D__SANDBOX__ -U_FORTIFY_SOURCE
PLATFORM_CPPFLAGS += -DCONFIG_ARCH_MAP_SYSMEM -DCONFIG_SYS_GENERIC_BOARD
PLATFORM_LIBS += -lrt

# Support generic board on sandbox
__HAVE_ARCH_GENERIC_BOARD := y

cmd_u-boot__ = $(CC) -o $@ -T u-boot.lds \
	-Wl,--start-group $(u-boot-main) -Wl,--end-group \
	$(PLATFORM_LIBS) -Wl,-Map -Wl,u-boot.map

CONFIG_ARCH_DEVICE_TREE := sandbox
