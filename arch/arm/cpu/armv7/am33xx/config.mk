#
# Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed "as is" WITHOUT ANY WARRANTY of any
# kind, whether express or implied; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
ifdef CONFIG_SPL_BUILD
ALL-y	+= $(OBJTREE)/MLO
else
ALL-y	+= $(obj)u-boot.img
endif
