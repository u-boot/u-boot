#
# Copyright (C) 2013 - 2015 Xilinx, Inc. All rights reserved.
#
# SPDX-License-Identifier:      GPL-2.0
#
# Allow NEON instructions (needed for lowlevel_init.S with GNU toolchain)
PLATFORM_RELFLAGS += -mfpu=neon
