#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

# Make ARMv5 to allow more compilers to work, even though its v7a.
PLATFORM_CPPFLAGS += -march=armv5
