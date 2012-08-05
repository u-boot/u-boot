#
# (C) Copyright 2012 Stephen Warren
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# Don't attempt to override the target CPU/ABI options;
# the Raspberry Pi toolchain does the right thing by default.
PLATFORM_RELFLAGS := $(filter-out -msoft-float,$(PLATFORM_RELFLAGS))
PLATFORM_CPPFLAGS := $(filter-out -march=armv5t,$(PLATFORM_CPPFLAGS))
