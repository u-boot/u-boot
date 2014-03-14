#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	= L1.o flash.o
obj-y	+= init.o
obj-y	+= bootscript.o

quiet_cmd_awk = AWK     $@
      cmd_awk = od -t x1 -v -A x $< | $(AWK) -f $(filter-out $<,$^) > $@

$(obj)/bootscript.c: $(obj)/bootscript.image $(src)/x2c.awk
	$(call cmd,awk)

quiet_cmd_mkimage = MKIMAGE $@
cmd_mkimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) -d $< $@ \
        $(if $(KBUILD_VERBOSE:1=), >/dev/null)

MKIMAGEFLAGS_bootscript.image := -A ppc -O linux -T script -C none \
						-a 0 -e 0 -n bootscript
$(obj)/bootscript.image: $(src)/bootscript.hush
	$(call cmd,mkimage)

clean-files := bootscript.c bootscript.image