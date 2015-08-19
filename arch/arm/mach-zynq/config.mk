#
# Copyright (C) 2013 - 2015 Xilinx, Inc. All rights reserved.
#
# SPDX-License-Identifier:      GPL-2.0
#
PLATFORM_RELFLAGS += -fno-strict-aliasing
# Xilinx, added to prevent unaligned accesses which started happening # with GCC 4.5.2 tools
PLATFORM_RELFLAGS += -mno-unaligned-access
