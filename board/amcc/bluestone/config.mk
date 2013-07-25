#
# Copyright (c) 2010, Applied Micro Circuits Corporation
# Author: Tirumala R Marri <tmarri@apm.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#
# Applied Micro APM821XX Evaluation board.
#

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
