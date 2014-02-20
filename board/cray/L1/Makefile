#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	= L1.o flash.o
obj-y	+= init.o
obj-y	+= bootscript.o

$(obj)/bootscript.c: $(obj)/bootscript.image
	od -t x1 -v -A x $^ | awk -f $(srctree)/$(src)/x2c.awk > $@

$(obj)/bootscript.image: $(src)/bootscript.hush
	-$(OBJTREE)/tools/mkimage -A ppc -O linux -T script -C none -a 0 -e 0 -n bootscript -d $< $@

clean-files := bootscript.c bootscript.image