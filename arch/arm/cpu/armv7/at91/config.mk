#
# Copyright (C) 2014, Andreas Bießmann <andreas.devel@googlemail.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#
ifdef CONFIG_SPL_BUILD
ALL-y	+= boot.bin
else
ALL-y	+= u-boot.img
endif
