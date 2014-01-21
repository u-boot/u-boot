#
# h2200 Support
#
# Copyright (C) 2012 Lukasz Dalek <luk0104@gmail.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= h2200.o

extra-y := h2200-header.bin

$(obj)h2200-header.bin: $(obj)h2200-header.o
	$(OBJCOPY) -O binary $< $@
