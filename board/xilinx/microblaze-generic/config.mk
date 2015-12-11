#
# (C) Copyright 2007 Michal Simek
#
# Michal  SIMEK <monstr@monstr.eu>
#
# SPDX-License-Identifier:	GPL-2.0+
#
# CAUTION: This file is a faked configuration !!!
#          There is no real target for the microblaze-generic
#          configuration. You have to replace this file with
#          the generated file from your Xilinx design flow.
#

PLATFORM_CPPFLAGS += -mno-xl-soft-mul
PLATFORM_CPPFLAGS += -mno-xl-soft-div
PLATFORM_CPPFLAGS += -mxl-barrel-shift
